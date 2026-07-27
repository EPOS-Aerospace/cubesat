# MPU9250 + Kalman 3 eixos (Roll / Pitch / Yaw)

Projeto PlatformIO para ESP32 (`esp32dev`). Estima atitude com filtro de Kalman 1D
por eixo:

- **Roll / Pitch** — fusão acelerômetro (ângulo) + giroscópio (taxa)
- **Yaw** — fusão magnetômetro com compensação de inclinação (ângulo) + giroscópio (taxa)

## Estrutura

```
mpu9250_kalman_yaw/
├── platformio.ini          # env esp32dev, lib_deps: hideakitai/MPU9250
├── src/main.cpp            # setup/loop, leitura MPU, tilt compensation, saída CSV
├── lib/Kalman/             # filtro de Kalman 1D reutilizável (mesmo lib de cubesat-adcs-ttc)
│   ├── Kalman.h
│   └── Kalman.cpp
├── include/                # headers públicos do projeto
└── test/                   # testes unitários (PlatformIO Unit Testing)
```

## Ligação (I2C)

| MPU9250 | ESP32 |
|---------|-------|
| SDA     | GPIO 21 |
| SCL     | GPIO 22 |
| VCC     | 3V3 |
| GND     | GND |

Endereço I2C: `0x68` (AD0 em GND).

## Uso

```bash
pio run                 # compila
pio run --target upload # grava
pio device monitor       # serial a 115200
```

Saída serial em CSV: `Tempo(ms),Roll,Pitch,Yaw` a ~50 Hz. Os scripts de captura e
plotagem em `../teste_mpu9250/mpu9250_test/` (`captura.py`, `graph.py`) consomem
esse mesmo formato.

## Calibração

Antes de confiar no Yaw, preencher em `src/main.cpp`:

- `gyroX/Y/Z_offset` — média do giroscópio parado
- `roll_offset`, `pitch_offset` — leitura do acelerômetro na posição de referência
- `MAG_OFFSET_X/Y/Z` — hard-iron do magnetômetro (rotacionar a placa em todos os
  eixos e tirar o ponto médio de cada min/max)

## Limitações conhecidas

1. **Descontinuidade do Yaw em ±180°** — o Kalman 1D trata o ângulo como escalar
   linear. Ao cruzar ±180° a inovação salta ~360° e a estimativa leva tempo para
   reconvergir. Correção: normalizar a inovação para (−180, 180] antes do update,
   ou trocar por um filtro em quatérnios.
2. **Só hard-iron** — sem correção soft-iron (escala/elipsoide) o heading tem erro
   dependente da orientação.
3. **Frame do magnetômetro (AK8963) ≠ frame do accel/gyro** no MPU9250 (X/Y
   trocados e Z invertido). Se o Yaw sair espelhado ou defasado 90°, remapear os
   eixos magnéticos antes da tilt compensation.
