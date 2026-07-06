#pragma once
#include <Arduino.h>

// ============================================================
//  PROTOCOLO BINÁRIO
//  Este header é a ÚNICA fonte de verdade do formato do pacote.
//  Satélite (TX) e Estação (RX) devem incluir este mesmo arquivo.
//
// 
//  [HEADER1][HEADER2][SEQ][TYPE][LEN][PAYLOAD...][CRC8]
//
//  NOTA: versão reduzida do payload de telemetria, usada
//  para o teste inicial de MPU9250 + Kalman + LoRa. Contém apenas
//  pitch_10 (2) + roll_10 (2) + timestamp (4) = 8 bytes.
//  Pacote total = 5 (cabeçalho) + 8 + 1 (crc) = 14 bytes.
//  Quando bat_mv, rpm_a/b e oil_flag tiverem fonte de dados real
//  (divisor de bateria, sensores Hall, payload YOLOv8), adicionar
//  os campos de volta na struct Telemetria e ajuste TELEMETRIA_LEN.
// ============================================================

#define PKT_HEADER1 0xAA
#define PKT_HEADER2 0xBB

#define PAYLOAD_MAX_LEN 32   // margem de segurança acima do payload de telemetria (15 bytes)

enum PacketType : uint8_t {
    TYPE_TELEMETRIA    = 0x01,
    TYPE_ALERTA_OLEO   = 0x02,
    TYPE_ACK           = 0x03,
    TYPE_CMD_CAPTURA   = 0x04,
    TYPE_PING          = 0xFF
};


struct Telemetria {
    int16_t  pitch_10;   // pitch * 10 (graus)
    int16_t  roll_10;    // roll * 10 (graus)
    uint32_t timestamp;  // millis() no momento da leitura
};

#define TELEMETRIA_LEN 8  // 2 + 2 + 4 bytes reais após serialização

// ------------------------------------------------------------
// CRC-8/SMBUS (polinômio 0x07)
// ------------------------------------------------------------
inline uint8_t crc8(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ 0x07) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

// ------------------------------------------------------------
// Serializa a struct Telemetria em um buffer de bytes (little-endian).
// Retorna o número de bytes escritos (TELEMETRIA_LEN).
// ------------------------------------------------------------
inline size_t serializarTelemetria(const Telemetria &t, uint8_t *buf) {
    size_t i = 0;
    memcpy(&buf[i], &t.pitch_10,  2); i += 2;
    memcpy(&buf[i], &t.roll_10,   2); i += 2;
    memcpy(&buf[i], &t.timestamp, 4); i += 4;
    return i; // deve ser 8
}

// ------------------------------------------------------------
// Desserializa um buffer de bytes de volta pra struct Telemetria.
// ------------------------------------------------------------
inline void desserializarTelemetria(const uint8_t *buf, Telemetria &t) {
    size_t i = 0;
    memcpy(&t.pitch_10,  &buf[i], 2); i += 2;
    memcpy(&t.roll_10,   &buf[i], 2); i += 2;
    memcpy(&t.timestamp, &buf[i], 4); i += 4;
}

// ------------------------------------------------------------
// Monta um pacote completo pronto pra transmissão.
// pktBuf deve ter espaço para 5 + payloadLen + 1 bytes.
// Retorna o tamanho total do pacote montado.
// ------------------------------------------------------------
inline size_t montarPacote(uint8_t seq, uint8_t type,
                            const uint8_t *payload, uint8_t payloadLen,
                            uint8_t *pktBuf) {
    size_t i = 0;
    pktBuf[i++] = PKT_HEADER1;
    pktBuf[i++] = PKT_HEADER2;
    pktBuf[i++] = seq;
    pktBuf[i++] = type;
    pktBuf[i++] = payloadLen;
    memcpy(&pktBuf[i], payload, payloadLen);
    i += payloadLen;

    // CRC8 sobre SEQ + TYPE + LEN + PAYLOAD (não inclui os headers de sync)
    uint8_t crc = crc8(&pktBuf[2], 3 + payloadLen);
    pktBuf[i++] = crc;

    return i;
}

// ------------------------------------------------------------
// Tenta parsear um buffer recebido do rádio como um pacote válido.
// Retorna true se header+CRC forem válidos.
// ------------------------------------------------------------
inline bool parsearPacote(const uint8_t *buf, size_t len,
                           uint8_t &seq, uint8_t &type,
                           uint8_t *payloadOut, uint8_t &payloadLen) {
    if (len < 6) return false; // menor pacote possível: 2 header + seq + type + len + crc

    if (buf[0] != PKT_HEADER1 || buf[1] != PKT_HEADER2) return false;

    seq = buf[2];
    type = buf[3];
    payloadLen = buf[4];

    if (payloadLen > PAYLOAD_MAX_LEN) return false;
    if (len < (size_t)(5 + payloadLen + 1)) return false;

    uint8_t crcRecebido = buf[5 + payloadLen];
    uint8_t crcCalculado = crc8(&buf[2], 3 + payloadLen);

    if (crcRecebido != crcCalculado) return false;

    memcpy(payloadOut, &buf[5], payloadLen);
    return true;
}
