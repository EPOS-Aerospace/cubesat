import pandas as pd
import matplotlib.pyplot as plt

# Carrega o CSV
df = pd.read_csv('dados.csv')

# Normaliza o tempo: subtrai o primeiro valor e converte de ms para segundos
df['Tempo(s)'] = (df['Tempo(ms)'] - df['Tempo(ms)'].iloc[0]) / 1000.0

# --- Gráfico 1: Roll (Acc vs Gyro vs Kalman) ---
plt.figure(figsize=(12, 6))
plt.plot(df['Tempo(s)'], df['Roll_Acc'], label='Roll Acelerômetro (ruidoso)', alpha=0.5, linewidth=1)
plt.plot(df['Tempo(s)'], df['Roll_Gyro'], label='Roll Giroscópio (integração pura)', alpha=0.7, linewidth=1)
plt.plot(df['Tempo(s)'], df['Roll_Kalman'], label='Roll Kalman (filtrado)', linewidth=2, color='red')
plt.xlabel('Tempo (s)')
plt.ylabel('Ângulo (graus)')
plt.title('Roll: Acelerômetro vs Giroscópio vs Kalman')
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('grafico_roll.png', dpi=150)
plt.show()

# --- Gráfico 2: Pitch (Acc vs Gyro vs Kalman) ---
plt.figure(figsize=(12, 6))
plt.plot(df['Tempo(s)'], df['Pitch_Acc'], label='Pitch Acelerômetro (ruidoso)', alpha=0.5, linewidth=1)
plt.plot(df['Tempo(s)'], df['Pitch_Gyro'], label='Pitch Giroscópio (integração pura)', alpha=0.7, linewidth=1)
plt.plot(df['Tempo(s)'], df['Pitch_Kalman'], label='Pitch Kalman (filtrado)', linewidth=2, color='red')
plt.xlabel('Tempo (s)')
plt.ylabel('Ângulo (graus)')
plt.title('Pitch: Acelerômetro vs Giroscópio vs Kalman')
plt.legend()
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.savefig('grafico_pitch.png', dpi=150)
plt.show()

# --- Gráfico 3: Comparação lado a lado (subplots) ---
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 10), sharex=True)

ax1.plot(df['Tempo(s)'], df['Roll_Acc'], label='Acelerômetro', alpha=0.5, linewidth=1)
ax1.plot(df['Tempo(s)'], df['Roll_Gyro'], label='Giroscópio', alpha=0.7, linewidth=1)
ax1.plot(df['Tempo(s)'], df['Roll_Kalman'], label='Kalman', linewidth=2, color='red')
ax1.set_ylabel('Roll (graus)')
ax1.set_title('Roll')
ax1.legend()
ax1.grid(True, alpha=0.3)

ax2.plot(df['Tempo(s)'], df['Pitch_Acc'], label='Acelerômetro', alpha=0.5, linewidth=1)
ax2.plot(df['Tempo(s)'], df['Pitch_Gyro'], label='Giroscópio', alpha=0.7, linewidth=1)
ax2.plot(df['Tempo(s)'], df['Pitch_Kalman'], label='Kalman', linewidth=2, color='red')
ax2.set_xlabel('Tempo (s)')
ax2.set_ylabel('Pitch (graus)')
ax2.set_title('Pitch')
ax2.legend()
ax2.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('grafico_comparativo.png', dpi=150)
plt.show()