// ============================================================

//  PINAGEM:
//  MPU9250:
//    GPIO21 -> SDA
//    GPIO22 -> SCL
//    3.3V   -> VCC
//    GND    -> GND
//    (AD0 -> GND, endereço I2C fica em 0x68)
//
//  LoRa AS32-TTL100:
//    GPIO16 (RX2) -> TX do AS32
//    GPIO17 (TX2) -> RX do AS32
//    GPIO4        -> M0 do AS32
//    GPIO5        -> M1 do AS32
//    GPIO18       -> AUX do AS32
//    3.3V         -> VCC do AS32
//    GND          -> GND do AS32
//
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <MPU9250.h>       // hideakitai/MPU9250
#include "LoRa_E32.h"
#include "Kalman.h"
#include "Protocolo.h"

// ------------------------------------------------------------
// Pinos
// ------------------------------------------------------------
#define PIN_M0        4
#define PIN_M1        5
#define PIN_AUX       18

// ------------------------------------------------------------
// Objetos globais
// ------------------------------------------------------------
MPU9250 mpu;
HardwareSerial loraSerial(2);
LoRa_E32 e32(&loraSerial, PIN_AUX, PIN_M0, PIN_M1);

Kalman_t KalmanRoll;
Kalman_t KalmanPitch;

uint8_t seq = 0;
uint32_t lastLeituraMs = 0;   // taxa de leitura do MPU (~100Hz)
uint32_t lastTxMs = 0;        // taxa de transmissão (conforme doc: 5s)
const uint32_t INTERVALO_LEITURA_MS = 10;
const uint32_t INTERVALO_TX_MS = 5000;

// Protótipos
void calibrarGyroBias();
void enviarTelemetria();
uint32_t agoraMs();

// ------------------------------------------------------------
// Calibração de bias do giroscópio (parado, 500 amostras)
// ------------------------------------------------------------
void calibrarGyroBias() {
    Serial.println("[INIT] Calibrando bias do giroscopio - NAO MOVER o sensor...");
    mpu.calibrateAccelGyro();  // biblioteca hideakitai já faz essa rotina internamente
    Serial.println("[INIT] Calibracao concluida.");
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== SATELITE — ADCS + TT&C ===");

    // ---------------- MPU9250 ----------------
    Wire.begin(21, 22);
    Wire.setClock(400000);

    if (!mpu.setup(0x68)) {
        Serial.println("[ERRO] MPU9250 nao encontrado. Verifique fiacao/endereco.");
        while (1) { delay(1000); }
    }

    calibrarGyroBias();

    Kalman_Init(&KalmanRoll);
    Kalman_Init(&KalmanPitch);

    // ---------------- LoRa ----------------
    loraSerial.begin(9600, SERIAL_8N1, 16, 17);
    e32.begin();

    ResponseStructContainer c = e32.getConfiguration();
    if (c.status.code == 1) {
        Configuration cfg = *(Configuration*) c.data;
        Serial.print("[LORA] Canal: "); Serial.println(cfg.CHAN);
        Serial.print("[LORA] Air rate: "); Serial.println(cfg.SPED.airDataRate);
    } else {
        Serial.println("[AVISO] Nao foi possivel ler config do LoRa. Verifique M0/M1/AUX.");
    }
    c.close();

    lastLeituraMs = millis();
    lastTxMs = millis();

    Serial.println("[INIT] Pronto. Iniciando loop de leitura + transmissao.\n");
}

void loop() {
    uint32_t agora = millis();

    // ---------------- Leitura do MPU + Kalman (~100Hz) ----------------
    if (agora - lastLeituraMs >= INTERVALO_LEITURA_MS) {

        float dt = (agora - lastLeituraMs) / 1000.0f;
        lastLeituraMs = agora;

        if (mpu.update()) {
            float ax = mpu.getAccX();
            float ay = mpu.getAccY();
            float az = mpu.getAccZ();

            float gx = mpu.getGyroX(); // graus/s (lib ja entrega em dps)
            float gy = mpu.getGyroY();

            float roll  = atan2(ay, az) * 180.0f / PI;
            float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;

            Kalman_GetAngle(&KalmanRoll, roll, gx, dt);
            Kalman_GetAngle(&KalmanPitch, pitch, gy, dt);
        }
    }

    // ---------------- Transmissao de telemetria (5s) ----------------
    if (agora - lastTxMs >= INTERVALO_TX_MS) {
        lastTxMs = agora;
        enviarTelemetria();
    }

    // ---------------- Recepcao de ACK ----------------
    // Pacote de ACK: header1+header2+seq+type+len+payload(1 byte)+crc8 = 7 bytes fixos
    const size_t TAMANHO_PACOTE_ACK = 7;
    if (e32.available() > 1) {
        ResponseStructContainer rsc = e32.receiveMessage(TAMANHO_PACOTE_ACK);
        if (rsc.status.code == 1) {
            uint8_t payloadOut[PAYLOAD_MAX_LEN];
            uint8_t seqRx, typeRx, lenRx;
            if (parsearPacote((uint8_t*)rsc.data, TAMANHO_PACOTE_ACK,
                               seqRx, typeRx, payloadOut, lenRx)) {
                if (typeRx == TYPE_ACK) {
                    Serial.printf("[RX] ACK recebido para SEQ=%03u\n", seqRx);
                }
            } else {
                Serial.println("[RX] Pacote de ACK invalido (header/CRC).");
            }
        }
        rsc.close();
    }
}

void enviarTelemetria() {

    if (digitalRead(PIN_AUX) == LOW) {
        Serial.println("[ERRO] AUX em LOW — modulo LoRa ocupado, pulando este ciclo.");
        return;
    }

    Telemetria t;
    t.pitch_10  = (int16_t)(KalmanPitch.angle * 10.0f);
    t.roll_10   = (int16_t)(KalmanRoll.angle * 10.0f);
    t.timestamp = agoraMs();

    uint8_t payload[TELEMETRIA_LEN];
    serializarTelemetria(t, payload);

    uint8_t pacote[32];
    size_t tamanhoPacote = montarPacote(seq, TYPE_TELEMETRIA, payload, TELEMETRIA_LEN, pacote);

    ResponseStatus rs = e32.sendFixedMessage(0, 0, 0x17, pacote, tamanhoPacote);

    if (rs.code == 1) {
        Serial.printf("[TX] SEQ=%03u | pitch=%.1f | roll=%.1f | OK\n",
                      seq, t.pitch_10 / 10.0f, t.roll_10 / 10.0f);
    } else {
        Serial.printf("[TX] SEQ=%03u | FALHA: %s\n", seq, rs.getResponseDescription());
    }

    seq++;
}

uint32_t agoraMs() {
    return millis();
}
