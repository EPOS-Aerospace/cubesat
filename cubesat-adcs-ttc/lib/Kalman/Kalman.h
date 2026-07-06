#pragma once

// ============================================================
//  FILTRO DE KALMAN 1D — fusão acelerômetro + giroscópio
//  Agnóstico ao sensor: recebe apenas ângulo (accel) e taxa (gyro)
// ============================================================

typedef struct {
  // Parâmetros de sintonia (Covariâncias de ruído)
  float Q_angle;   // Variância do ruído de processo (modelo de estado)
  float Q_bias;    // Variância do ruído de processo (deriva do giroscópio)
  float R_measure; // Variância do ruído de medição (sensor)

  // Variáveis de estado
  float angle;     // Ângulo estimado (a posteriori)
  float bias;      // Deriva estimada do giroscópio
  float rate;      // Taxa de rotação sem polarização (unbiased)

  // Matriz de covariância do erro de estimação (2x2)
  float P[2][2];
} Kalman_t;

void Kalman_Init(Kalman_t *kalman);
float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt);
