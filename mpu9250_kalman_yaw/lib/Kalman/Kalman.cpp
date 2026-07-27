#include "Kalman.h"

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

  // 1. Predição
  kalman->rate = newRate - kalman->bias;
  kalman->angle += dt * kalman->rate;

  kalman->P[0][0] += dt * (dt * kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
  kalman->P[0][1] -= dt * kalman->P[1][1];
  kalman->P[1][0] -= dt * kalman->P[1][1];
  kalman->P[1][1] += kalman->Q_bias * dt;

  // 2. Atualização (assimilação da medição)
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
