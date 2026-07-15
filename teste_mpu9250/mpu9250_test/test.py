import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# =========================================================
# Carrega o CSV gerado pelo script de captura
# =========================================================
df = pd.read_csv('soak_test.csv')
df['Tempo(s)'] = (df['Tempo(ms)'] - df['Tempo(ms)'].iloc[0]) / 1000.0

# Taxa de amostragem real (baseada no intervalo médio entre leituras)
dt_medio = df['Tempo(s)'].diff().mean()
fs = 1.0 / dt_medio
print(f"Taxa de amostragem detectada: {fs:.2f} Hz (dt médio = {dt_medio*1000:.2f} ms)")
print(f"Duração total do teste: {df['Tempo(s)'].iloc[-1]:.1f} s ({df['Tempo(s)'].iloc[-1]/60:.1f} min)")

# =========================================================
# 1. ALLAN VARIANCE (giroscópio X e Y)
# =========================================================
try:
    import allantools
except ImportError:
    print("\nInstale a lib primeiro: pip install allantools")
    raise

gyroX = df['GyroX_bruto'].values
gyroY = df['GyroY_bruto'].values

# oadev = overlapping Allan deviation, mais robusto estatisticamente que o não-overlapping
taus_x, adevs_x, errors_x, ns_x = allantools.oadev(gyroX, rate=fs, data_type="freq", taus="all")
taus_y, adevs_y, errors_y, ns_y = allantools.oadev(gyroY, rate=fs, data_type="freq", taus="all")

plt.figure(figsize=(10, 7))
plt.loglog(taus_x, adevs_x, label='Giro X', marker='o', markersize=3)
plt.loglog(taus_y, adevs_y, label='Giro Y', marker='o', markersize=3)
plt.xlabel('Tempo de integração τ (s)')
plt.ylabel('Desvio de Allan (°/s)')
plt.title('Allan Deviation - MPU9250 Giroscópio')
plt.legend()
plt.grid(True, which="both", alpha=0.3)
plt.tight_layout()
plt.savefig('allan_variance_giroscopio.png', dpi=150)
plt.show()

# Estimativa dos parâmetros-chave a partir da curva:
# - Angle Random Walk (ARW): valor do desvio de Allan em tau=1s (região de inclinação -1/2 no log-log)
# - Bias Instability: valor mínimo da curva (ponto de inflexão, região "flat")
idx_tau1_x = np.argmin(np.abs(taus_x - 1.0))
idx_tau1_y = np.argmin(np.abs(taus_y - 1.0))
bias_instab_x = np.min(adevs_x)
bias_instab_y = np.min(adevs_y)
tau_bias_instab_x = taus_x[np.argmin(adevs_x)]
tau_bias_instab_y = taus_y[np.argmin(adevs_y)]

print("\n" + "="*60)
print("PARÂMETROS ALLAN VARIANCE")
print("="*60)
print(f"Giro X - ARW (τ=1s): {adevs_x[idx_tau1_x]:.5f} °/s  |  Bias Instability: {bias_instab_x:.5f} °/s em τ={tau_bias_instab_x:.1f}s")
print(f"Giro Y - ARW (τ=1s): {adevs_y[idx_tau1_y]:.5f} °/s  |  Bias Instability: {bias_instab_y:.5f} °/s em τ={tau_bias_instab_y:.1f}s")

# =========================================================
# 2. DERIVA TÉRMICA (Temperatura vs Bias estimado pelo Kalman)
# =========================================================
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 9), sharex=True)

# Bias estimado pelo Kalman ao longo do tempo
ax1.plot(df['Tempo(s)'], df['BiasX_Kalman'], label='Bias X (Kalman)', color='tab:blue')
ax1.plot(df['Tempo(s)'], df['BiasY_Kalman'], label='Bias Y (Kalman)', color='tab:orange')
ax1.set_ylabel('Bias estimado (°/s)')
ax1.set_title('Deriva do Bias do Giroscópio ao longo do tempo')
ax1.legend()
ax1.grid(True, alpha=0.3)

# Temperatura ao longo do tempo
ax2.plot(df['Tempo(s)'], df['Temp'], color='tab:red')
ax2.set_xlabel('Tempo (s)')
ax2.set_ylabel('Temperatura (°C)')
ax2.set_title('Temperatura interna do MPU9250 ao longo do tempo')
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('deriva_termica.png', dpi=150)
plt.show()

# Correlação numérica entre temperatura e bias
corr_x = df['Temp'].corr(df['BiasX_Kalman'])
corr_y = df['Temp'].corr(df['BiasY_Kalman'])

print("\n" + "="*60)
print("CORRELAÇÃO TEMPERATURA x BIAS")
print("="*60)
print(f"Correlação Temp x Bias X: {corr_x:.3f}")
print(f"Correlação Temp x Bias Y: {corr_y:.3f}")
print("(valores próximos de +1 ou -1 indicam forte relação entre aquecimento e deriva do bias)")

# Variação total observada
delta_temp = df['Temp'].max() - df['Temp'].min()
delta_bias_x = df['BiasX_Kalman'].iloc[-1] - df['BiasX_Kalman'].iloc[0]
delta_bias_y = df['BiasY_Kalman'].iloc[-1] - df['BiasY_Kalman'].iloc[0]

print(f"\nVariação de temperatura durante o teste: {delta_temp:.2f}°C")
print(f"Variação total do Bias X: {delta_bias_x:.5f} °/s")
print(f"Variação total do Bias Y: {delta_bias_y:.5f} °/s")