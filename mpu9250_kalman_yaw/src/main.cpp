#include <Arduino.h>
#include <Wire.h>
#include <MPU9250.h> // Biblioteca do MPU9250 baixada pelo PlatformIO
#include <Kalman.h>  // lib/Kalman — filtro de Kalman 1D

MPU9250 mpu;

Kalman_t KalmanX; // Eixo X (Roll)
Kalman_t KalmanY; // Eixo Y (Pitch)
Kalman_t KalmanZ; // Eixo Z (Yaw)

unsigned long tempoAnterior = 0;

// Offsets de calibração manual do giroscópio e acelerômetro
float gyroX_offset = 0.0f;
float gyroY_offset = 0.0f;
float gyroZ_offset = 0.0f;
float roll_offset = 0.0f;
float pitch_offset = 0.0f;

// Constantes de calibração do magnetômetro (Hard-Iron)
// Estes valores devem ser obtidos com um script de calibração prévio na placa real
const float MAG_OFFSET_X = 0.0f;
const float MAG_OFFSET_Y = 0.0f;
const float MAG_OFFSET_Z = 0.0f;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Configuração explícita dos pinos I2C para o ESP32
  Wire.begin(21, 22);
  Wire.setClock(400000);

  // Inicializa o MPU9250
  if (!mpu.setup(0x68)) {
    Serial.println("Falha no MPU9250. Verifique a fiação e o endereço I2C.");
    while (1) { delay(1000); }
  }

  Serial.println("Aguardando estabilização do MEMS (500ms)...");
  delay(500);

  // Inicializa os 3 filtros de Kalman
  Kalman_Init(&KalmanX);
  Kalman_Init(&KalmanY);
  Kalman_Init(&KalmanZ);

  tempoAnterior = micros();

  Serial.println("Tempo(ms),Roll,Pitch,Yaw");
}

void loop() {
  if (mpu.update()) {
    unsigned long tempoAtual = micros();
    float dt = (tempoAtual - tempoAnterior) / 1000000.0f;
    tempoAnterior = tempoAtual;

    // 1. DADOS DO ACELERÔMETRO (Para Roll e Pitch)
    float accX = mpu.getAccX();
    float accY = mpu.getAccY();
    float accZ = mpu.getAccZ();

    float roll_acc = (atan2(accY, accZ) * 180.0 / PI) - roll_offset;
    float pitch_acc = (atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI) - pitch_offset;

    // 2. DADOS DO GIROSCÓPIO
    float gyroX = mpu.getGyroX() - gyroX_offset;
    float gyroY = mpu.getGyroY() - gyroY_offset;
    float gyroZ = mpu.getGyroZ() - gyroZ_offset;

    // 3. FILTRAGEM ROLL E PITCH
    float KalAngleX = Kalman_GetAngle(&KalmanX, roll_acc, gyroX, dt);
    float KalAngleY = Kalman_GetAngle(&KalmanY, pitch_acc, gyroY, dt);

    // =======================================================
    // 4. MÓDULO YAW (MAGNETÔMETRO)
    // =======================================================

    // Leitura magnética com correção Hard-Iron
    float magX = mpu.getMagX() - MAG_OFFSET_X;
    float magY = mpu.getMagY() - MAG_OFFSET_Y;
    float magZ = mpu.getMagZ() - MAG_OFFSET_Z;

    // Compensação de Inclinação (Tilt Compensation)
    float cosRoll = cos(KalAngleX * PI / 180.0);
    float sinRoll = sin(KalAngleX * PI / 180.0);
    float cosPitch = cos(KalAngleY * PI / 180.0);
    float sinPitch = sin(KalAngleY * PI / 180.0);

    float Xh = magX * cosPitch + magY * sinRoll * sinPitch + magZ * cosRoll * sinPitch;
    float Yh = magY * cosRoll - magZ * sinRoll;

    // Medição magnética absoluta projetada no plano horizontal
    float yaw_mag = atan2(-Yh, Xh) * 180.0 / PI;

    // Filtragem do Yaw
    // ATENÇÃO: o Kalman 1D não trata descontinuidade em ±180°. Ao cruzar essa
    // fronteira a inovação (yaw_mag - angle) salta ~360° e a estimativa demora
    // a reconvergir. Ver README (seção "Limitações conhecidas").
    float KalAngleZ = Kalman_GetAngle(&KalmanZ, yaw_mag, gyroZ, dt);

    // 5. SAÍDA DE DADOS (50Hz)
    static unsigned long ultimoPrint = 0;
    if (millis() - ultimoPrint > 20) {
      ultimoPrint = millis();

      Serial.print(millis()); Serial.print(",");
      Serial.print(KalAngleX); Serial.print(",");
      Serial.print(KalAngleY); Serial.print(",");
      Serial.println(KalAngleZ);
    }
  }
}
