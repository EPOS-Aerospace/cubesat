#include <Arduino.h>
#include <Wire.h>
#include <MPU9250.h>

MPU9250 mpu;

// Estrutura de estado e matrizes do Filtro de Kalman 1D
typedef struct {
  float Q_angle;
  float Q_bias;
  float R_measure;

  float angle;
  float bias;
  float rate;

  float P[2][2];
} Kalman_t;

Kalman_t KalmanX;
Kalman_t KalmanY;

unsigned long tempoAnterior = 0;

// Offsets de calibração manual
float gyroX_offset = 0.0f;
float gyroY_offset = 0.0f;
float roll_offset = 0.0f;
float pitch_offset = 0.0f;

// Variáveis para armazenar a integração pura do giroscópio
float gyroAngleX = 0.0f;
float gyroAngleY = 0.0f;

void Kalman_Init(Kalman_t *kalman) {
  kalman->Q_angle = 0.001f;
  kalman->Q_bias = 0.003f;
  kalman->R_measure = 0.03f;

  kalman->angle = 0.0f;
  kalman->bias = 0.0f;

  kalman->P[0][0] = 0.0f;
  kalman->P[0][1] = 0.0f;
  kalman->P[1][0] = 0.0f;
  kalman->P[1][1] = 0.0f;
}

float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt) {
  kalman->rate = newRate - kalman->bias;
  kalman->angle += dt * kalman->rate;

  kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
  kalman->P[0][1] -= dt * kalman->P[1][1];
  kalman->P[1][0] -= dt * kalman->P[1][1];
  kalman->P[1][1] += kalman->Q_bias * dt;

  float S = kalman->P[0][0] + kalman->R_measure;

  float K[2];
  K[0] = kalman->P[0][0] / S;
  K[1] = kalman->P[1][0] / S;

  float y = newAngle - kalman->angle;

  kalman->angle += K[0] * y;
  kalman->bias += K[1] * y;

  float P00 = kalman->P[0][0];
  float P01 = kalman->P[0][1];

  kalman->P[0][0] -= K[0] * P00;
  kalman->P[0][1] -= K[0] * P01;
  kalman->P[1][0] -= K[1] * P00;
  kalman->P[1][1] -= K[1] * P01;

  return kalman->angle;
}

void CalibrarGiroscopio(int amostras) {
  float somaX = 0.0f;
  float somaY = 0.0f;

  Serial.println("Calibrando giroscopio... mantenha o sensor PARADO.");

  int contador = 0;
  while (contador < amostras) {
    if (mpu.update()) {
      somaX += mpu.getGyroX();
      somaY += mpu.getGyroY();
      contador++;
      delay(2);
    }
  }

  gyroX_offset = somaX / amostras;
  gyroY_offset = somaY / amostras;
}

void CalibrarAcelerometro(int amostras) {
  float somaRoll = 0.0f;
  float somaPitch = 0.0f;

  Serial.println("Calibrando acelerometro... mantenha o sensor PARADO e NIVELADO.");

  int contador = 0;
  while (contador < amostras) {
    if (mpu.update()) {
      float accX = mpu.getAccX();
      float accY = mpu.getAccY();
      float accZ = mpu.getAccZ();

      somaRoll += atan2(accY, accZ) * 180.0 / PI;
      somaPitch += atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI;
      contador++;
      delay(2);
    }
  }

  roll_offset = somaRoll / amostras;
  pitch_offset = somaPitch / amostras;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  Wire.setClock(400000);

  if (!mpu.setup(0x68)) {
    Serial.println("Falha no MPU9250. Verifique fiacao.");
    while (1) { delay(1000); }
  }

  // --- CORREÇÃO 1: Delay de estabilização do transiente de power-on ---
  Serial.println("Aguardando estabilizacao do MEMS (500ms)...");
  delay(500); 

  CalibrarGiroscopio(500);
  CalibrarAcelerometro(500);

  Kalman_Init(&KalmanX);
  Kalman_Init(&KalmanY);

  tempoAnterior = micros();

  // --- CORREÇÃO 2: Cabeçalho com a 13ª Coluna (dt) ---
  Serial.println("Tempo(ms),Roll_Acc,Roll_Gyro,Roll_Kalman,AccX,AccY,AccZ,Pitch_Acc,Pitch_Gyro,Pitch_Kalman,GyroX_bruto,GyroY_bruto,Temp,BiasX,BiasY,dt");
}

void loop() {
  if (mpu.update()) {
    unsigned long tempoAtual = micros();
    float dt = (tempoAtual - tempoAnterior) / 1000000.0f; // dt calculado com precisão de microsegundos
    tempoAnterior = tempoAtual;

    float accX = mpu.getAccX();
    float accY = mpu.getAccY();
    float accZ = mpu.getAccZ();

    float gyroX_bruto = mpu.getGyroX();
    float gyroY_bruto = mpu.getGyroY();

    float gyroX = gyroX_bruto - gyroX_offset;
    float gyroY = gyroY_bruto - gyroY_offset;

    float roll_acc = (atan2(accY, accZ) * 180.0 / PI) - roll_offset;
    float pitch_acc = (atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0 / PI) - pitch_offset;

    gyroAngleX += gyroX * dt;
    gyroAngleY += gyroY * dt;

    float KalAngleX = Kalman_GetAngle(&KalmanX, roll_acc, gyroX, dt);
    float KalAngleY = Kalman_GetAngle(&KalmanY, pitch_acc, gyroY, dt);

    float temperatura = mpu.getTemperature();

    static unsigned long ultimoPrint = 0;
    if (millis() - ultimoPrint > 20) {
      ultimoPrint = millis();

      Serial.print(millis()); Serial.print(",");
      Serial.print(roll_acc); Serial.print(",");
      Serial.print(gyroAngleX); Serial.print(",");
      Serial.print(KalAngleX); Serial.print(",");

      Serial.print(accX); Serial.print(",");
      Serial.print(accY); Serial.print(",");
      Serial.print(accZ); Serial.print(",");
      
      Serial.print(pitch_acc); Serial.print(",");
      Serial.print(gyroAngleY); Serial.print(",");
      Serial.print(KalAngleY); Serial.print(",");
      
      Serial.print(gyroX_bruto, 4); Serial.print(",");
      Serial.print(gyroY_bruto, 4); Serial.print(",");
      Serial.print(temperatura, 2); Serial.print(",");
      Serial.print(KalmanX.bias, 5); Serial.print(",");
      Serial.print(KalmanY.bias, 5); Serial.print(",");
      
      
      Serial.println(dt, 6); 
    }
  }
}