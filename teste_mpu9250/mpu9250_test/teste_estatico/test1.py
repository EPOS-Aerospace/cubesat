import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# 1. Carregar os dados
try:
    df = pd.read_csv('soak_test.csv')
except FileNotFoundError:
    print("Erro: Arquivo 'soak_test.csv' não encontrado.")
    exit()

# 2. Normalizar o tempo para começar em 0 segundos
tempo_inicial = df['Tempo(ms)'].iloc[0]
df['Tempo(s)'] = (df['Tempo(ms)'] - tempo_inicial) / 1000.0

# 3. Converter a coluna 'dt' para milissegundos
df['dt_ms'] = df['dt'] * 1000.0

plt.rcParams.update({
    'font.family': 'serif',
    'font.serif': ['Times New Roman', 'DejaVu Serif', 'serif'],
    'font.size': 11,
    'axes.labelsize': 12,
    'legend.fontsize': 10,
    'xtick.labelsize': 10,
    'ytick.labelsize': 10,
    'axes.linewidth': 1.0,
    'grid.alpha': 0.5,
    'grid.linestyle': '--'
})
# ==========================================
# GRÁFICO 1: Desempenho do Filtro (Roll)
# ==========================================
plt.figure(figsize=(12, 5))

plt.plot(df['Tempo(s)'], df['Roll_Acc'],
         label='Ângulo (Acelerômetro)',
         color='lightcoral', alpha=0.5, linewidth=1)

plt.plot(df['Tempo(s)'], df['Roll_Gyro'],
         label='Ângulo (Giroscópio Integrado)',
         color='dodgerblue', linestyle='--', linewidth=1.5)

plt.plot(df['Tempo(s)'], df['Roll_Kalman'],
         label='Ângulo (Filtro de Kalman)',
         color='black', linewidth=2)

plt.title('Eixo Roll', fontsize=16, fontweight='bold')
plt.xlabel('Tempo (s)', fontsize=14)
plt.ylabel('Ângulo (graus)', fontsize=14)

plt.xticks(fontsize=12)
plt.yticks(fontsize=12)

plt.legend(fontsize=12)

plt.tight_layout()
plt.savefig('1_teste_roll.png', dpi=300)


# ==========================================
# GRÁFICO 2: Desempenho do Filtro (Pitch)
# ==========================================
plt.figure(figsize=(12, 5))

plt.plot(df['Tempo(s)'], df['Pitch_Acc'],
         label='Ângulo (Acelerômetro)',
         color='mediumaquamarine', alpha=0.5, linewidth=1)

plt.plot(df['Tempo(s)'], df['Pitch_Gyro'],
         label='Ângulo (Giroscópio Integrado)',
         color='orange', linestyle='--', linewidth=1.5)

plt.plot(df['Tempo(s)'], df['Pitch_Kalman'],
         label='Ângulo (Filtro de Kalman)',
         color='darkgreen', linewidth=2)

plt.title('Eixo Pitch', fontsize=16, fontweight='bold')
plt.xlabel('Tempo (s)', fontsize=14)
plt.ylabel('Ângulo (graus)', fontsize=14)

plt.xticks(fontsize=12)
plt.yticks(fontsize=12)

plt.legend(fontsize=12)

plt.tight_layout()
plt.savefig('2_teste_pitch.png', dpi=300)
"""
# ==========================================
# GRÁFICO 3: Evolução do Bias pelo Kalman
# ==========================================
plt.figure(figsize=(8, 4))
plt.plot(df['Tempo(s)'], df['BiasX'], label='Bias $X$', color='#9467bd', linewidth=1.5, linestyle='-')
plt.plot(df['Tempo(s)'], df['BiasY'], label='Bias $Y$', color='#8c564b', linewidth=1.5, linestyle='--')
plt.xlabel('Time (s)')
plt.ylabel('Drift Rate (deg/s)')
plt.grid(True)
plt.legend(frameon=False, loc='best')
plt.tight_layout()
plt.savefig('3_teste_bias.pdf', format='pdf', bbox_inches='tight')

# ==========================================
# GRÁFICO 4: Análise de Jitter (dt)
# ==========================================
plt.figure(figsize=(8, 3))
plt.plot(df['Tempo(s)'], df['dt_ms'], label='Sensor interval ($\Delta t$)', color='#17becf', linewidth=1)
media_dt = df['dt_ms'].mean()
plt.axhline(y=media_dt, color='red', linestyle='--', label=f'Mean: {media_dt:.2f} ms')
plt.xlabel('Time (s)')
plt.ylabel('$\Delta t$ (ms)')
plt.grid(True)
plt.legend(frameon=False, loc='upper right')
plt.tight_layout()
plt.savefig('4_teste_jitter.pdf', format='pdf', bbox_inches='tight')

# ==========================================
# GRÁFICO 5: Aceleração Bruta (Forças G)
# ==========================================
plt.figure(figsize=(8, 4))
plt.plot(df['Tempo(s)'], df['AccX'], label='Accel $X$', color='#d62728', alpha=0.7, linewidth=1)
plt.plot(df['Tempo(s)'], df['AccY'], label='Accel $Y$', color='#2ca02c', alpha=0.7, linewidth=1, linestyle='--')
plt.plot(df['Tempo(s)'], df['AccZ'], label='Accel $Z$', color='#1f77b4', alpha=0.7, linewidth=1, linestyle='-.')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (g)')
plt.grid(True)
plt.legend(frameon=False, loc='best')
plt.tight_layout()
plt.savefig('5_teste_aceleracao.pdf', format='pdf', bbox_inches='tight')

# ==========================================
# GRÁFICO 6: Deriva Térmica (Thermal Drift MPU9250)
# ==========================================
df['GyroX_suave'] = df['GyroX_bruto'].rolling(window=50, min_periods=1).mean()
df['GyroY_suave'] = df['GyroY_bruto'].rolling(window=50, min_periods=1).mean()

fig, ax1 = plt.subplots(figsize=(8, 4))
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('Raw Gyro Rate (deg/s)', color='black')
ax1.plot(df['Tempo(s)'], df['GyroX_suave'], label='Gyro $X$ (Filtered)', color='black', linestyle='-')
ax1.plot(df['Tempo(s)'], df['GyroY_suave'], label='Gyro $Y$ (Filtered)', color='dimgray', linestyle='--')
ax1.tick_params(axis='y', labelcolor='black')
ax1.grid(True)
ax1.legend(frameon=False, loc='upper left')

ax2 = ax1.twinx()  
ax2.set_ylabel('Internal Temp ($^\circ$C)', color='#d62728')  
ax2.plot(df['Tempo(s)'], df['Temp'], label='MPU9250 Temp', color='#d62728', linestyle=':', linewidth=2)
ax2.tick_params(axis='y', labelcolor='#d62728')
ax2.legend(frameon=False, loc='upper right')

plt.tight_layout()
plt.savefig('6_teste_deriva_termica.pdf', format='pdf', bbox_inches='tight')

# ==========================================
# GRÁFICO 7: Desvio de Allan (Allan Deviation)
# ==========================================
def compute_allan_deviation(data, dt):
    tau_out, ad_out = [], []
    N = len(data)
    max_m = int(N / 4) 
    if max_m < 2: return [], []
    
    m_list = np.unique(np.logspace(0, np.log10(max_m), 50).astype(int))
    for m in m_list:
        if m == 0: continue
        k = int(N / m)
        if k < 2: continue
        
        y_m = np.zeros(k)
        for i in range(k):
            y_m[i] = np.mean(data[i*m : (i+1)*m])
        
        diff = np.diff(y_m)
        adev = np.sqrt(0.5 * np.mean(diff**2))
        tau_out.append(m * dt)
        ad_out.append(adev)
    return tau_out, ad_out

dt_medio = df['dt'].mean()
tau_x, ad_x = compute_allan_deviation(df['GyroX_bruto'].values, dt_medio)
tau_y, ad_y = compute_allan_deviation(df['GyroY_bruto'].values, dt_medio)

plt.figure(figsize=(8, 5))
if tau_x and tau_y:
    plt.loglog(tau_x, ad_x, label='Gyro $X$', color='#1f77b4', linewidth=1.5, linestyle='-')
    plt.loglog(tau_y, ad_y, label='Gyro $Y$', color='#d62728', linewidth=1.5, linestyle='--')
    
    plt.xlabel(r'Integration Time, $\tau$ (s)')
    plt.ylabel(r'Allan Deviation, $\sigma(\tau)$ (deg/s)')
    plt.grid(True, which="both", ls=":", alpha=0.6)
    plt.legend(frameon=False, loc='lower left')
else:
    plt.text(0.5, 0.5, 'Insufficient data for Allan Deviation.', 
             horizontalalignment='center', verticalalignment='center', fontsize=12)

plt.tight_layout()
plt.savefig('7_teste_allan_dev.pdf', format='pdf', bbox_inches='tight')
"""