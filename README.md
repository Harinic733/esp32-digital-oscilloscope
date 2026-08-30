# ESP32 Digital Oscilloscope & Waveform Visualizer 🎛️

![Target Microcontroller](https://img.shields.io/badge/Target-ESP32%20Dual--Core-blue)
![Domain](https://img.shields.io/badge/Domain-Digital%20Signal%20Processing-purple)
![Language](https://img.shields.io/badge/Language-C%2B%2B%20%2F%20HTML5-orange)
![Simulation](https://img.shields.io/badge/Simulation-Wokwi-brightgreen)

An **advanced Digital Signal Processing (DSP) Edge application** built on the ESP32 dual-core microcontroller. 

This project synthesizes multi-mode analog waveforms (Sine, Square, Sawtooth) in real-time, samples High-Speed ADC telemetry, and streams live signal frames to a phosphor-green **HTML5 Canvas Oscilloscope Dashboard**.

---


## ✨ Key Technical Features & DSP Concepts

1. **Mathematical Waveform Synthesis:**
   - **Sine Wave:** Generated via Trigonometric Floating Point: $V(t) = \frac{V_{pp}}{2} \cdot \sin(2\pi \cdot f \cdot t) + \frac{V_{pp}}{2}$
   - **Square Wave:** Binary threshold phase switching at frequency $f$.
   - **Sawtooth Wave:** Linear ramp phase synthesis over normalized period $[0.0, 1.0)$.
2. **Dual-Core FreeRTOS Partitioning:**
   - **Core 0:** High-frequency synthesis and ADC sampling loop (`TaskSignalSynthesizer`).
   - **Core 1:** Async JSON telemetry frame serialization and WebSocket broadcasting (`TaskTelemetryStream`).
3. **Thread-Safe Data Protection (`SemaphoreHandle_t` Mutex):**
   - Synchronizes memory access between the Core 0 synthesis buffer generator and Core 1 output streamer.
4. **HTML5 Canvas Oscilloscope GUI:**
   - Custom phosphor-green scope screen rendering real-time grid lines, trigger offset, Peak-to-Peak Voltage ($V_{pp}$), and RMS voltage ($V_{rms}$) measurements.

---

## 🔌 Hardware Pin Mapping

| Component | ESP32 GPIO Pin | Function |
| :--- | :--- | :--- |
| **Potentiometer 1** | `GPIO 34` | Frequency Control Tuner (1 Hz - 30 Hz) |
| **Potentiometer 2** | `GPIO 35` | Amplitude Control Tuner (0.5 V - 3.3 V) |
| **Status LED** | `GPIO 2` | Synthesis Activity Indicator LED |

---

## 💻 How to Run

### 1. Wokwi Hardware Simulation
1. Go to **[Wokwi.com](https://wokwi.com/)** and select **ESP32**.
2. Copy [`main.ino`](./main.ino) into `sketch.ino`.
3. Copy [`diagram.json`](./diagram.json) into `diagram.json`.
4. Press **Play ▶️** to view live DSP synthesis logs in the Serial Monitor!

### 2. Browser Oscilloscope GUI
1. Open [`index.html`](./index.html) in any web browser to view the real-time phosphor oscilloscope dashboard and live waveform controls!

---

