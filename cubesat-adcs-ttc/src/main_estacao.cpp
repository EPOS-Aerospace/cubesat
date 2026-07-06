// ============================================================
//  ESTAÇÃO TERRESTRE (fica ligada ao PC via USB)

//    GPIO16 (RX2) -> TX do AS32
//    GPIO17 (TX2) -> RX do AS32
//    GPIO4        -> M0 do AS32
//    GPIO5        -> M1 do AS32
//    GPIO18       -> AUX do AS32
//    3.3V         -> VCC do AS32
//    GND          -> GND do AS32
//
//  
//  Python de logging le essa porta COM e filtra as linhas "CSV:".
// ============================================================

#include <Arduino.h>
#include "LoRa_E32.h"
#include "Protocolo.h"

#define PIN_M0  4
#define PIN_M1  5
#define PIN_AUX 18

HardwareSerial loraSerial(2);
LoRa_E32 e32(&loraSerial, PIN_AUX, PIN_M0, PIN_M1);

uint32_t pktsRecebidos = 0;
uint32_t pktsPerdidos  = 0;
uint32_t pktsCrcInvalido = 0;
uint8_t  lastSeq = 255; // 255 = "ainda nao recebi nada"

// Tamanho fixo do pacote de telemetria (versão reduzida deste teste):
// 5 (cabecalho) + 8 (payload: pitch_10+roll_10+timestamp) + 1 (crc) = 14 bytes
const size_t TAMANHO_PACOTE_TELEMETRIA = 14;

void enviarAck(uint8_t seq);
void processarTelemetria(const uint8_t *payload, uint8_t seqRx);

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== ESTACAO TERRESTRE — TT&C ===");

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

    Serial.println("[INIT] Aguardando pacotes do satelite...\n");
}

void loop() {
    if (e32.available() > 1) {

        ResponseStructContainer rsc = e32.receiveMessage(TAMANHO_PACOTE_TELEMETRIA);

        if (rsc.status.code != 1) {
            Serial.printf("[ERRO RX] %s\n", rsc.status.getResponseDescription());
            rsc.close();
            return;
        }

        uint8_t payloadOut[PAYLOAD_MAX_LEN];
        uint8_t seqRx, typeRx, lenRx;

        bool ok = parsearPacote((uint8_t*)rsc.data, TAMANHO_PACOTE_TELEMETRIA,
                                 seqRx, typeRx, payloadOut, lenRx);

        rsc.close();

        if (!ok) {
            pktsCrcInvalido++;
            Serial.println("[ERRO] Pacote invalido (header/CRC) — descartado.");
            return;
        }

        // Deteccao de pacotes perdidos
        if (lastSeq != 255) {
            uint8_t esperado = lastSeq + 1;
            if (seqRx != esperado) {
                uint8_t perdidos = (uint8_t)(seqRx - esperado);
                pktsPerdidos += perdidos;
                Serial.printf("[WARN] %u pacote(s) perdido(s) entre SEQ=%03u e SEQ=%03u\n",
                              perdidos, lastSeq, seqRx);
            }
        }
        lastSeq = seqRx;
        pktsRecebidos++;

        if (typeRx == TYPE_TELEMETRIA) {
            processarTelemetria(payloadOut, seqRx);
        } else {
            Serial.printf("[RX] SEQ=%03u | TYPE=0x%02X (nao tratado neste teste)\n", seqRx, typeRx);
        }

        enviarAck(seqRx);
    }
}

void processarTelemetria(const uint8_t *payload, uint8_t seqRx) {
    Telemetria t;
    desserializarTelemetria(payload, t);

    // Linha legivel no monitor serial
    Serial.printf(
        "[RX] SEQ=%03u | pitch=%.1f | roll=%.1f | ts=%lu | OK=%lu Perdidos=%lu\n",
        seqRx, t.pitch_10 / 10.0f, t.roll_10 / 10.0f,
        (unsigned long)t.timestamp,
        (unsigned long)pktsRecebidos, (unsigned long)pktsPerdidos
    );

    // Linha em formato CSV — o logger Python filtra por "CSV:" e salva num arquivo
    Serial.printf(
        "CSV:%lu,%03u,%.1f,%.1f\n",
        (unsigned long)millis(), seqRx,
        t.pitch_10 / 10.0f, t.roll_10 / 10.0f
    );
}

void enviarAck(uint8_t seq) {
    uint32_t t0 = millis();
    while (digitalRead(PIN_AUX) == LOW) {
        if (millis() - t0 > 500) {
            Serial.println("[WARN] AUX timeout — ACK nao enviado.");
            return;
        }
        delay(5);
    }

    uint8_t payload[1] = {0x00};
    uint8_t pacote[16];
    size_t tamanhoPacote = montarPacote(seq, TYPE_ACK, payload, 1, pacote);

    ResponseStatus rs = e32.sendFixedMessage(0, 0, 0x17, pacote, tamanhoPacote);

    if (rs.code == 1) {
        Serial.printf("       -> ACK enviado para SEQ=%03u\n", seq);
    } else {
        Serial.printf("       -> ACK FALHOU: %s\n", rs.getResponseDescription());
    }
}
