# 🧠⚡ EEG CEGS – Sistema de Captura y Procesamiento de Señales Cerebrales 🧠⚡

> Proyecto desarrollado por estudiantes de Ingeniería Electrónica - UCEVA  
> ¡Captura, procesa y analiza la actividad eléctrica del cerebro con ESP32-S3 y PCB personalizada!

---

## 📋 Descripción del Proyecto

Este proyecto tiene como objetivo el diseño e implementación de un sistema funcional de **electroencefalograma (EEG)**, capaz de capturar y procesar señales cerebrales en tiempo real.

Se ha diseñado una **máquina de estados finita (FSM) tipo Moore**, programada en la **ESP32-S3**, que controla las fases del sistema: desde la captura de señales hasta la presentación del resultado, usando indicadores visuales (LEDs) y entradas táctiles (`Touch 1` y `Touch 2`) para interactuar con el flujo.

---

## 🧩 Características principales

- ✅ Captura de señales EEG por medio de entrada analógica (ADC)
- ✅ Procesamiento interno de señales para detección de transiciones (alfa/beta)
- ✅ Máquina de estados con 4 fases: reposo, captura, procesamiento, resultado
- ✅ Control con sensores táctiles capacitivos (touch)
- ✅ Indicadores LED para mostrar estado del sistema en tiempo real
- ✅ FSM implementada en lenguaje C++ sobre el framework de Arduino
- ✅ PCB diseñada en **Autodesk EAGLE**, lista para fabricación 🛠️📦

---

## 🛠️ Tecnologías y herramientas usadas

- 🔹 ESP32-S3 (WiFi + Touch + ADC + potencia de procesamiento)
- 🔹 EAGLE para diseño de la PCB
- 🔹 Arduino Framework (ESP-IDF también compatible)
- 🔹 Procesamiento básico de señales EEG (FFT futura)
- 🔹 Pantalla OLED para retroalimentación visual
- 🔹 LEDs para retroalimentación visual

---

## 🚀 Estado del proyecto

> En desarrollo activo – se encuentra en fase de pruebas de la FSM y captura de señal.  
> Próximamente se integrará la lógica de procesamiento alfa/beta y se enviará la PCB a fabricación.

---

## 🤝 Aportaciones

¿Tienes experiencia en procesado de señales cerebrales, diseño de filtros, o análisis?  
¡Contribuye con este proyecto abierto! 😄

---

## 📜 Licencia

Este proyecto está bajo licencia **MIT**.  
Puedes usar, modificar y compartir el código libremente, siempre dando crédito a los autores.

---

**Proyecto académico desarrollado por estudiantes de Ingeniería Electrónica – UCEVA**
