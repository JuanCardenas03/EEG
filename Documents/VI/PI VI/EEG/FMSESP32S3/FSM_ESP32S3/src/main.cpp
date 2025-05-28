// src/main.cpp

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/adc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <arduinoFFT.h> // Asegúrate de instalar kosme/arduinoFFT@^2.0.4 en lib_deps

// ————— Configuración ADC + FreeRTOS —————
#define ADC_CH ADC1_CHANNEL_3 // GPIO4
#define SR_HZ 1000            // Frecuencia de muestreo (Hz)
#define N_SAMPLES 1024        // Muestras por bloque

static uint16_t rawBuf[N_SAMPLES]; // Buffer de ADC
static QueueHandle_t qADC;         // Cola para bloques de ADC

// FFT buffers e instanciación
static double vReal[N_SAMPLES];
static double vImag[N_SAMPLES];
ArduinoFFT<double> FFT(vReal, vImag, N_SAMPLES, SR_HZ);

// ————— Configuración FSM y temporizadores —————
enum State
{
  S0_REPOSO,
  S1_CAPTURA,
  S2_PROCESAMIENTO,
  S3_RESULTADO
};
static State currentState = S0_REPOSO;
#define CAPTURE_DURATION_MS 5000 // 5 segundos de captura
static unsigned long s1StartMillis = 0;
static bool fftDone = false;

// ————— OLED y pines I2C —————
#define SCREEN_W 128
#define SCREEN_H 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_W, SCREEN_H, &Wire, OLED_RESET);
const int SDApin = 41; // SDA I2C (soldado)
const int SCLpin = 2;  // SCL I2C (soldado)

// ————— Pines de toque —————
const int TOUCH_PIN = 11;
const uint32_t TOUCH_THRESH = 24000;

// ————— Prototipos —————
void setupAdc();
void setQueue();
void taskADC(void *pv);
void performFFT();
void drawSpectrum();
void updateDisplay();

// ————— Setup —————
void setup()
{
  Serial.begin(115200);
  // Inicializar I2C en los pines soldados
  Wire.begin(SDApin, SCLpin);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR))
  {
    Serial.println("Error al iniciar OLED");
    while (true)
      delay(10);
  }

  setupAdc();
  setQueue();
  xTaskCreate(taskADC, "ADC Task", 2048, NULL, configMAX_PRIORITIES - 1, NULL);

  // Mostrar estado inicial
  updateDisplay();
}

// ————— Loop principal —————
void loop()
{
  uint32_t t = touchRead(TOUCH_PIN);
  // Depuración: muestra valor crudo
  Serial.printf("Touch=%u\n", t);
  // Transiciones con Touch1 y temporizador en S1
  if (t > TOUCH_THRESH)
  {
    if (currentState == S1_CAPTURA)
    {
      if (millis() - s1StartMillis >= CAPTURE_DURATION_MS)
      {
        currentState = S2_PROCESAMIENTO;
        fftDone = false;
        updateDisplay();
        delay(300);
      }
    }
    else
    {
      // Avanzar linealmente: S0->S1->S2->S3->S0
      if (currentState == S0_REPOSO)
      {
        currentState = S1_CAPTURA;
        s1StartMillis = millis();
      }
      else if (currentState == S2_PROCESAMIENTO && fftDone)
      {
        currentState = S3_RESULTADO;
      }
      else if (currentState == S3_RESULTADO)
      {
        currentState = S0_REPOSO;
      }
      updateDisplay();
      delay(300);
    }
  }

  // S2: FFT una sola vez
  if (currentState == S2_PROCESAMIENTO && !fftDone)
  {
    performFFT();
    fftDone = true;
    updateDisplay();
  }

  // S3: Mostrar espectro
  if (currentState == S3_RESULTADO)
  {
    drawSpectrum();
  }

  vTaskDelay(pdMS_TO_TICKS(50));
}

// ————— Configuración ADC y cola —————
void setupAdc()
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC_CH, ADC_ATTEN_DB_12);
}

void setQueue()
{
  qADC = xQueueCreate(2, N_SAMPLES * sizeof(uint16_t));
  configASSERT(qADC != NULL);
}

// ————— Tarea de muestreo —————
void taskADC(void *pv)
{
  uint32_t idx = 0;
  for (;;)
  {
    rawBuf[idx++] = analogRead(ADC_CH);
    if (idx >= N_SAMPLES)
    {
      idx = 0;
      xQueueSend(qADC, rawBuf, portMAX_DELAY);
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// ————— Cálculo de FFT —————
void performFFT()
{
  uint16_t buf[N_SAMPLES];
  xQueueReceive(qADC, buf, portMAX_DELAY);
  double mean = 0;
  for (int i = 0; i < N_SAMPLES; i++)
  {
    vReal[i] = buf[i];
    mean += vReal[i];
    vImag[i] = 0;
  }
  mean /= N_SAMPLES;
  for (int i = 0; i < N_SAMPLES; i++)
    vReal[i] -= mean;
  FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(FFT_FORWARD);
  FFT.complexToMagnitude();
}

// ————— Dibujar espectro en OLED —————
void drawSpectrum()
{
  display.clearDisplay();
  const float maxFreq = 60.0; // Mostrar solo hasta 60 Hz
  int maxBin = (int)((maxFreq / (float)SR_HZ) * N_SAMPLES);
  if (maxBin > N_SAMPLES / 2)
    maxBin = N_SAMPLES / 2;

  double maxVal = 1;
  for (int i = 1; i < maxBin; i++)
  {
    if (vReal[i] > maxVal)
      maxVal = vReal[i];
  }
  for (int i = 1; i < maxBin; i++)
  {
    int x = map(i, 1, maxBin - 1, 0, SCREEN_W - 1);
    int y = SCREEN_H - 1 - map((int)vReal[i], 0, (int)maxVal, 0, SCREEN_H - 1);
    display.drawLine(x, SCREEN_H - 1, x, y, SSD1306_WHITE);
  }
  display.display();
}

// ————— Actualiza texto en pantalla según estado —————
void updateDisplay()
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 16);
  switch (currentState)
  {
  case S0_REPOSO:
    display.print("S0 Reposo");
    break;
  case S1_CAPTURA:
    display.print("Tomando...");
    break;
  case S2_PROCESAMIENTO:
    display.print(fftDone ? "Listo" : "Procesando");
    break;
  case S3_RESULTADO:
    display.print("Resultado");
    break;
  }
  display.display();
}
