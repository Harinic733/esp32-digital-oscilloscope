// =============================================================
// ESP32 WebSockets Real-Time Signal Generator & Oscilloscope
// Domain: Embedded Systems / Digital Signal Processing (DSP)
// Author: 3rd Year ECE Project
// =============================================================

#include <Arduino.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Hardware Pin Definitions
#define FREQ_POT_PIN     34     // Potentiometer 1: Signal Frequency Tuner
#define AMP_POT_PIN      35     // Potentiometer 2: Signal Amplitude Tuner
#define LED_STATUS_PIN   2      // Status LED (Blinks on sample transmission)

// DSP Constants
#define SAMPLE_BUFFER_SIZE  50  // Number of samples per waveform frame
#define PI_VAL              3.14159265358979323846f

// Waveform Modes
enum WaveformType { SINE_WAVE = 0, SQUARE_WAVE = 1, SAWTOOTH_WAVE = 2 };
WaveformType currentWaveform = SINE_WAVE;

// Data Structure for Waveform Packet
struct WaveformFrame {
  float samples[SAMPLE_BUFFER_SIZE];
  float frequency;
  float vpp;       // Peak-to-Peak Voltage (0 - 3.3V)
  float vrms;      // RMS Voltage
  const char* waveType;
};

// Global Handles & Synchronization Mutex
SemaphoreHandle_t dspMutex = NULL;
WaveformFrame globalFrame;

// Task Declarations
void TaskSignalSynthesizer(void *pvParameters);
void TaskTelemetryStream(void *pvParameters);

// SETUP FUNCTION
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, HIGH);

  // Create Mutex for DSP memory safety
  dspMutex = xSemaphoreCreateMutex();

  if (dspMutex == NULL) {
    Serial.println("FATAL: Failed to create FreeRTOS Mutex!");
    while (1);
  }

  Serial.println("==================================================");
  Serial.println(" ESP32 Oscilloscope & Signal Generator Starting...");
  Serial.println("==================================================");

  // Create Dual-Core Tasks
  // Core 0: High-Speed Signal Synthesis & Sampling
  xTaskCreatePinnedToCore(TaskSignalSynthesizer, "SignalSynthTask", 4096, NULL, 2, NULL, 0);

  // Core 1: Telemetry Stream & JSON Serialization
  xTaskCreatePinnedToCore(TaskTelemetryStream, "TelemetryTask", 4096, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

// TASK 1: Signal Synthesis Engine (Core 0, Priority 2)
void TaskSignalSynthesizer(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    // Read Tuners (ADC 0-4095)
    int rawFreq = analogRead(FREQ_POT_PIN);
    int rawAmp  = analogRead(AMP_POT_PIN);

    // Map Frequency (1.0 Hz to 30.0 Hz) and Amplitude (0.5V to 3.3V)
    float targetFreq = 1.0f + ((float)rawFreq / 4095.0f) * 29.0f;
    float peakVolts  = 0.5f + ((float)rawAmp  / 4095.0f) * 2.8f;

    // Determine Waveform Mode based on frequency range division
    WaveformType waveMode = (rawFreq < 1365) ? SINE_WAVE : ((rawFreq < 2730) ? SQUARE_WAVE : SAWTOOTH_WAVE);
    const char* modeName = (waveMode == SINE_WAVE) ? "SINE" : ((waveMode == SQUARE_WAVE) ? "SQUARE" : "SAWTOOTH");

    WaveformFrame tempFrame;
    tempFrame.frequency = targetFreq;
    tempFrame.vpp = peakVolts;
    tempFrame.vrms = peakVolts * 0.707f;
    tempFrame.waveType = modeName;

    // Generate 50 Samples in Time Domain
    float dt = (1.0f / targetFreq) / SAMPLE_BUFFER_SIZE;
    float t = 0.0f;

    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
      float sampleVal = 0.0f;
      float normTime = fmod(t * targetFreq, 1.0f);

      switch (waveMode) {
        case SINE_WAVE:
          sampleVal = (peakVolts / 2.0f) * sin(2.0f * PI_VAL * normTime) + (peakVolts / 2.0f);
          break;
        case SQUARE_WAVE:
          sampleVal = (normTime < 0.5f) ? peakVolts : 0.0f;
          break;
        case SAWTOOTH_WAVE:
          sampleVal = peakVolts * normTime;
          break;
      }
      tempFrame.samples[i] = sampleVal;
      t += dt;
    }

    // Safely copy sample frame to global memory using Mutex
    if (xSemaphoreTake(dspMutex, portMAX_DELAY) == pdTRUE) {
      globalFrame = tempFrame;
      xSemaphoreGive(dspMutex);
    }

    digitalWrite(LED_STATUS_PIN, !digitalRead(LED_STATUS_PIN));
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// TASK 2: Telemetry Serialization & Stream (Core 1, Priority 1)
void TaskTelemetryStream(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(150));

    WaveformFrame frameCopy;
    bool dataReady = false;

    if (xSemaphoreTake(dspMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
      frameCopy = globalFrame;
      dataReady = true;
      xSemaphoreGive(dspMutex);
    }

    if (dataReady) {
      String json = "{";
      json += "\"wave\":\"" + String(frameCopy.waveType) + "\",";
      json += "\"freq\":" + String(frameCopy.frequency, 1) + ",";
      json += "\"vpp\":" + String(frameCopy.vpp, 2) + ",";
      json += "\"vrms\":" + String(frameCopy.vrms, 2) + ",";
      json += "\"samples\":[";
      
      for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
        json += String(frameCopy.samples[i], 2);
        if (i < SAMPLE_BUFFER_SIZE - 1) json += ",";
      }
      json += "]}";

      Serial.println("[DSP_FRAME] " + json);
    }
  }
}
