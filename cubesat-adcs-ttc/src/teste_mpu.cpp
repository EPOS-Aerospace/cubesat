// ============================================================
//  TESTE ISOLADO — MPU9250 + Filtro de Kalman (SEM LoRa)

// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <MPU9250.h>       // hideakitai/MPU9250
#include "Kalman.h"

MPU9250 mpu;

Kalman_t KalmanRoll;
Kalman_t KalmanPitch;

uint32_t lastLeituraMs = 0;
uint32_t lastPrintMs = 0;

const uint32_t INTERVALO_LEITURA_MS = 10;  // ~100Hz, igual ao firmware final
const uint32_t INTERVALO_PRINT_MS = 100;   // print a cada 100ms pra nao poluir o serial

// Guarda os ultimos valores crus lidos, pra imprimir fora do bloco de leitura
float ultimoAx = 0, ultimoAy = 0, ultimoAz = 0;
float ultimoGx = 0, ultimoGy = 0, ultimoGz = 0;
float ultimoMx = 0, ultimoMy = 0, ultimoMz = 0;
float ultimoTemp = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== TESTE ISOLADO: MPU9250 + KALMAN ===");

    Wire.begin(21, 22);
    Wire.setClock(400000);

    if (!mpu.setup(0x68)) {
        Serial.println("[ERRO] MPU9250 nao encontrado. Verifique fiacao/endereco (SDA=21, SCL=22, AD0=GND).");
        while (1) { delay(1000); }
    }

    Serial.println("[INIT] Sensor encontrado. Calibrando bias do giroscopio...");
    Serial.println("[INIT] NAO MOVA o sensor agora — deixe parado e nivelado.");
    delay(1000);

    mpu.calibrateAccelGyro();

    Serial.println("[INIT] Calibracao concluida.");

    Kalman_Init(&KalmanRoll);
    Kalman_Init(&KalmanPitch);

    lastLeituraMs = millis();
    lastPrintMs = millis();

    Serial.println("[INIT] Pronto. Incline o sensor pra testar.\n");
    Serial.println("AccelXYZ(g) | GyroXYZ(dps) | MagXYZ(uT) | Temp(C) || Roll(Kalman) | Pitch(Kalman)");
}

void loop() {
    uint32_t agora = millis();

    if (agora - lastLeituraMs >= INTERVALO_LEITURA_MS) {

        float dt = (agora - lastLeituraMs) / 1000.0f;
        lastLeituraMs = agora;

        if (mpu.update()) {
            float ax = mpu.getAccX();
            float ay = mpu.getAccY();
            float az = mpu.getAccZ();

            float gx = mpu.getGyroX(); // graus/s
            float gy = mpu.getGyroY();
            float gz = mpu.getGyroZ(); // nao usado no Kalman (yaw sem atuador no projeto)

            float mx = mpu.getMagX();  // uT — nao usado no Kalman ainda
            float my = mpu.getMagY();
            float mz = mpu.getMagZ();

            float temp = mpu.getTemperature(); // graus C, sensor interno

            float roll  = atan2(ay, az) * 180.0f / PI;
            float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0f / PI;

            Kalman_GetAngle(&KalmanRoll, roll, gx, dt);
            Kalman_GetAngle(&KalmanPitch, pitch, gy, dt);

            // guarda os valores crus pra impressao (fora do Kalman)
            ultimoAx = ax; ultimoAy = ay; ultimoAz = az;
            ultimoGx = gx; ultimoGy = gy; ultimoGz = gz;
            ultimoMx = mx; ultimoMy = my; ultimoMz = mz;
            ultimoTemp = temp;
        }
    }

    // Print periodico (nao a cada leitura, pra nao inundar o serial)
    if (agora - lastPrintMs >= INTERVALO_PRINT_MS) {
        lastPrintMs = agora;
        Serial.printf(
            "A[%6.2f %6.2f %6.2f] G[%7.1f %7.1f %7.1f] M[%6.1f %6.1f %6.1f] T=%5.1f || Roll=%6.1f Pitch=%6.1f\n",
            ultimoAx, ultimoAy, ultimoAz,
            ultimoGx, ultimoGy, ultimoGz,
            ultimoMx, ultimoMy, ultimoMz,
            ultimoTemp,
            KalmanRoll.angle, KalmanPitch.angle
        );
    }
}