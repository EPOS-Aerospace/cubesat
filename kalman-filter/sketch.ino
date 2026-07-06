#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// Estrutura de estado e matrizes do Filtro de Kalman 1D
typedef struct {
  // Parâmetros de sintonia (Covariâncias de ruído)
  float Q_angle;   // Variância do ruído de processo (modelo de estado)
  float Q_bias;    // Variância do ruído de processo (deriva do giroscópio)
  float R_measure; // Variância do ruído de medição (sensor)

  // Variáveis de estado
  float angle;     // Ângulo estimado (a posteriori)
  float bias;      // Deriva estimada do giroscópio
  float rate;      // Taxa de rotação desprovida de polarização (unbiased)

  // Matriz de covariância do erro de estimação (2x2)
  float P[2][2];
} Kalman_t;

Kalman_t KalmanX;
Kalman_t KalmanY;

unsigned long tempoAnterior = 0;

// Inicialização dos parâmetros e matrizes de covariância
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

// Estimação recursiva de estado (Propagação e Assimilação)
float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt) {

  // 1. Etapa de Predição (Propagação temporal)
  // Atualiza a estimativa de estado a priori
  kalman->rate = newRate - kalman->bias;
  kalman->angle += dt * kalman->rate;

  // Atualiza a covariância do erro a priori
  kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
  kalman->P[0][1] -= dt * kalman->P[1][1];
  kalman->P[1][0] -= dt * kalman->P[1][1];
  kalman->P[1][1] += kalman->Q_bias * dt;

  // 2. Etapa de Atualização (Assimilação da medição)
  // Calcula a covariância da inovação (S)
  float S = kalman->P[0][0] + kalman->R_measure;

  // Calcula o ganho de Kalman (K) otimizado
  float K[2];
  K[0] = kalman->P[0][0] / S;
  K[1] = kalman->P[1][0] / S;

  // Calcula a inovação (resíduo entre medição e predição)
  float y = newAngle - kalman->angle;

  // Atualiza a estimativa de estado a posteriori
  kalman->angle += K[0] * y;
  kalman->bias += K[1] * y;

  // Atualiza a covariância do erro a posteriori
  float P00 = kalman->P[0][0];
  float P01 = kalman->P[0][1];

  kalman->P[0][0] -= K[0] * P00;
  kalman->P[0][1] -= K[0] * P01;
  kalman->P[1][0] -= K[1] * P00;
  kalman->P[1][1] -= K[1] * P01;

  return kalman->angle;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!mpu.begin()) {
    Serial.println("Falha na inicialização do MPU6050.");
    while (1);
  }

  Kalman_Init(&KalmanX);
  Kalman_Init(&KalmanY);

  tempoAnterior = micros();
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Integração temporal (dt)
  unsigned long tempoAtual = micros();
  float dt = (tempoAtual - tempoAnterior) / 1000000.0f;
  tempoAnterior = tempoAtual;

  // Cálculo dos ângulos de inclinação via acelerômetro
  float roll = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  float pitch = atan2(
                  -a.acceleration.x,
                  sqrt(a.acceleration.y * a.acceleration.y + a.acceleration.z * a.acceleration.z)
                ) * 180.0 / PI;

  // Leituras angulares do giroscópio (graus/s)
  float gyroX = g.gyro.x * 180.0 / PI;
  float gyroY = g.gyro.y * 180.0 / PI;

  // Filtragem de estado
  float KalAngleX = Kalman_GetAngle(&KalmanX, roll, gyroX, dt);
  float KalAngleY = Kalman_GetAngle(&KalmanY, pitch, gyroY, dt);

  // Saída de dados (apenas variáveis de interesse)
  Serial.print("Roll: ");
  Serial.print(KalAngleX);
  Serial.print(" | Pitch: ");
  Serial.println(KalAngleY);

  // Frequência de amostragem ajustada para ~100Hz.
  // Fundamental para evitar acumulação de erro na integração do giroscópio.
  delay(10); 
}