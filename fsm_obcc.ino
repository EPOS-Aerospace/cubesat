// ============================================================
//  OBC — Esqueleto da Máquina de Estados (FSM)
// ============================================================

#include <Arduino.h>

// ------------------------------------------------------------
//  Estados
// ------------------------------------------------------------
typedef enum {
  STATE_INIT,
  STATE_STANDBY,
  STATE_CAPTURE,
  STATE_TRANSMIT,
  STATE_EMERGENCY
} SystemState;

SystemState currentState = STATE_INIT;

// ------------------------------------------------------------
//  Setup
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("[BOOT] ESP32 iniciando...");
}

// ------------------------------------------------------------
//  Loop — dispatcher da FSM
// ------------------------------------------------------------
void loop() {
  switch (currentState) {
    case STATE_INIT:      state_init();      break;
    case STATE_STANDBY:   state_standby();   break;
    case STATE_CAPTURE:   state_capture();   break;
    case STATE_TRANSMIT:  state_transmit();  break;
    case STATE_EMERGENCY: state_emergency(); break;
  }
}

// ------------------------------------------------------------
//  INIT
// ------------------------------------------------------------
void state_init() {
  Serial.println("[FSM] INIT");
  Serial.println("[INIT] Verificando RBF... [STUB OK]");
  Serial.println("[INIT] Inicializando perifericos... [STUB OK]");
  Serial.println("[INIT] Tensao da bateria: 3700 mV [STUB]");
  Serial.println("[FSM] INIT concluido -> STANDBY");
  currentState = STATE_STANDBY;
}

// ------------------------------------------------------------
//  STANDBY
//  Envia 'C' pelo Serial Monitor para simular comando de captura
// ------------------------------------------------------------
void state_standby() {
  Serial.println("[FSM] STANDBY — aguardando comando ('C' para capturar)");

  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'C' || c == 'c') {
        Serial.println("[FSM] Comando de captura recebido -> CAPTURE");
        currentState = STATE_CAPTURE;
        return;
      }
      if (c == 'E' || c == 'e') {
        Serial.println("[FSM] Simulando bateria baixa -> EMERGENCY");
        currentState = STATE_EMERGENCY;
        return;
      }
    }
    delay(1000);
  }
}

// ------------------------------------------------------------
//  CAPTURE
// ------------------------------------------------------------
void state_capture() {
  Serial.println("[FSM] CAPTURE");
  Serial.println("[RPI] Ligando RPi... [STUB]");
  Serial.println("[RPI] Aguardando READY... [STUB]");
  Serial.println("[RPI] READY recebido [STUB]");
  Serial.println("[RPI] Enviando CMD:CAPTURE [STUB]");
  Serial.println("[RPI] Resultado: OIL:0:92 [STUB]");
  Serial.println("[RPI] Shutdown controlado... [STUB]");
  Serial.println("[FSM] Captura concluida -> TRANSMIT");
  currentState = STATE_TRANSMIT;
}

// ------------------------------------------------------------
//  TRANSMIT
// ------------------------------------------------------------
void state_transmit() {
  Serial.println("[FSM] TRANSMIT");
  Serial.println("[LORA] Enviando telemetria... [STUB]");
  Serial.println("[LORA] Telemetria transmitida -> STANDBY");
  currentState = STATE_STANDBY;
}

// ------------------------------------------------------------
//  EMERGENCY
//  Envia 'R' pelo Serial Monitor para simular recuperacao
// ------------------------------------------------------------
void state_emergency() {
  Serial.println("[FSM] EMERGENCY");
  Serial.println("[EMRG] RPi desligado [STUB]");
  Serial.println("[EMRG] Transmitindo beacon a cada 30s... [STUB]");
  Serial.println("[EMRG] ('R' para simular recuperacao de tensao)");

  while (true) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == 'R' || c == 'r') {
        Serial.println("[FSM] Tensao recuperada -> INIT");
        currentState = STATE_INIT;
        return;
      }
    }
    delay(1000);
  }
}
