# Epos Cubesat

Firmware for the **EPOS CubeSat**. This repository holds the embedded code
for attitude estimation (ADCS), telemetry/command link (TT&C), the on-board
computer (OBC) state machine, and a Wokwi-based Kalman filter simulation.

All targets are **ESP32** (Arduino framework). Comments and serial output are in
Brazilian Portuguese.

## Repository layout

```
epos-cubesat/
├── cubesat-adcs-ttc/      PlatformIO project — ADCS + TT&C firmware (satellite + ground station)
├── fsm_obcc.ino           OBC finite-state-machine skeleton (Arduino sketch, stubbed peripherals)
└── kalman-filter/         Wokwi simulation of the 1D Kalman filter (MPU6050 + ESP32)
```

## `cubesat-adcs-ttc/` — ADCS + TT&C firmware

A single PlatformIO project that builds **two firmwares** from one shared
codebase, selected by build environment in `platformio.ini`:

| Environment | Source           | Role                                                                 |
|-------------|------------------|---------------------------------------------------------------------|
| `satelite`  | `main_satelite.cpp` | Reads the IMU, runs Kalman, transmits telemetry over LoRa (TX)    |
| `estacao`   | `main_estacao.cpp`  | Ground station on the PC; receives, validates, ACKs, logs CSV (RX) |

`build_src_filter` compiles only the matching `main_*.cpp` per environment while
both share `lib/` (`Kalman`, `Protocolo`).

**Hardware**
- MCU: ESP32 devkit (`esp32dev`)
- IMU: **MPU9250** over I2C (`SDA=GPIO21`, `SCL=GPIO22`, `AD0→GND` → address `0x68`)
- Radio: **LoRa AS32-TTL100 (E32)** on `UART2` (`RX=GPIO16`, `TX=GPIO17`, `M0=GPIO4`, `M1=GPIO5`, `AUX=GPIO18`)

**Dependencies** (`lib_deps`)
- `hideakitai/MPU9250 @ ^0.4.8`
- `xreef/LoRa_E32_Series_Library` (from git)

**Shared libraries** (`lib/`)
- **`Kalman`** — 1D Kalman filter fusing accelerometer angle and gyroscope rate,
  estimating angle plus gyro bias. Sensor-agnostic (takes only angle, rate, dt).
- **`Protocolo`** — single source of truth for the binary link protocol:
  ```
  [HEADER1=0xAA][HEADER2=0xBB][SEQ][TYPE][LEN][PAYLOAD...][CRC8]
  ```
  CRC-8/SMBUS (poly `0x07`) over `SEQ+TYPE+LEN+PAYLOAD`. Packet types: telemetry,
  oil alert, ACK, capture command, ping. Provides pack/parse and telemetry
  serialize/deserialize helpers.

**Telemetry payload** (current reduced test format): `pitch_10` (int16, deg×10) +
`roll_10` (int16, deg×10) + `timestamp` (uint32, millis) = 8 bytes → 14-byte
packet total. Battery voltage, wheel RPM, and oil flag are reserved for when real
data sources exist (see notes in `Protocolo.h`).

**Runtime behavior**
- Satellite: IMU + Kalman at ~100 Hz (10 ms); telemetry TX every 5 s; listens for ACKs.
- Ground station: parses packets, detects lost packets via SEQ gaps, counts CRC
  errors, sends ACKs, prints a human line plus a `CSV:` line for a Python logger.

**`src/teste_mpu.cpp`** — standalone MPU9250 + Kalman bring-up test (no LoRa),
prints raw accel/gyro/mag/temp and Kalman roll/pitch. Not part of either build
environment; compile it by pointing `build_src_filter` at it.

### Build & flash

```bash
cd cubesat-adcs-ttc
pio run -e satelite -t upload        # flash the satellite
pio run -e estacao  -t upload        # flash the ground station
pio device monitor -b 115200         # open the serial monitor
```

## `fsm_obcc.ino` — OBC state machine

Arduino sketch skeleton for the on-board computer. Implements the state
dispatcher and transitions for `INIT → STANDBY → CAPTURE → TRANSMIT → EMERGENCY`.
Peripheral actions (RBF check, battery, RPi payload, LoRa) are stubbed with serial
prints. Drive it over the serial monitor: `C` triggers capture, `E` forces
emergency, `R` recovers from emergency.

## `kalman-filter/` — Wokwi simulation

Self-contained [Wokwi](https://wokwi.com) project simulating the 1D Kalman filter
on an ESP32 with an **MPU6050** (`sketch.ino`, `diagram.json`). Same filter math as
the flight `Kalman` library; used to validate the estimator before touching
hardware. Original project:
<https://wokwi.com/projects/468204040156867585>.

## Status

Early integration / bring-up. The ADCS + TT&C link (MPU9250 + Kalman + LoRa) works
end-to-end with the reduced telemetry payload; the OBC FSM and the extended
telemetry fields (battery, RPM, oil flag) are still skeletons/reserved.
