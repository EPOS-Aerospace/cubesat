import pandas as pd
import matplotlib.pyplot as plt

# 1. Carregar os dados do novo ensaio dinâmico
try:
    df = pd.read_csv('dinam_test.csv')
except FileNotFoundError:
    print("Erro: Arquivo 'dados.csv' não encontrado na pasta atual.")
    exit()

# 2. Normalizar o tempo para começar em 0 segundos
tempo_inicial = df['Tempo(ms)'].iloc[0]
df['Tempo(s)'] = (df['Tempo(ms)'] - tempo_inicial) / 1000.0

# Estilo acadêmico
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams.update({'font.size': 11})

# ==========================================
# GRÁFICO 1: Resposta Dinâmica (Eixo Roll)
# ==========================================
plt.figure(figsize=(12, 5))
# Acelerômetro semi-transparente para mostrar a vibração/ruído sem esconder as outras linhas
plt.plot(df['Tempo(s)'], df['Roll_Acc'], label='Acelerômetro (Ruído/Vibração)', color='lightcoral', alpha=0.5, linewidth=1)
# Giroscópio tracejado mostrando a resposta rápida
plt.plot(df['Tempo(s)'], df['Roll_Gyro'], label='Giroscópio (Rápido, mas com leve deriva)', color='dodgerblue', linestyle='--', linewidth=1.5)
# Kalman em destaque
plt.plot(df['Tempo(s)'], df['Roll_Kalman'], label='Kalman (Resposta Limpa e Estável)', color='black', linewidth=2)

plt.title('Ensaio Dinâmico: Resposta ao Degrau (90°) e Rejeição a Vibrações - Eixo Roll', fontweight='bold')
plt.xlabel('Tempo (s)')
plt.ylabel('Ângulo (graus)')
plt.legend(loc='best')
plt.tight_layout()
plt.savefig('8_dinamico_roll11.png', dpi=300)

# ==========================================
# GRÁFICO 2: Resposta Dinâmica (Eixo Pitch)
# ==========================================
plt.figure(figsize=(12, 5))
plt.plot(df['Tempo(s)'], df['Pitch_Acc'], label='Acelerômetro (Ruído/Vibração)', color='mediumaquamarine', alpha=0.5, linewidth=1)
plt.plot(df['Tempo(s)'], df['Pitch_Gyro'], label='Giroscópio (Rápido, mas com leve deriva)', color='orange', linestyle='--', linewidth=1.5)
plt.plot(df['Tempo(s)'], df['Pitch_Kalman'], label='Kalman (Resposta Limpa e Estável)', color='darkgreen', linewidth=2)

plt.title('Ensaio Dinâmico: Resposta ao Degrau (90°) e Rejeição a Vibrações - Eixo Pitch', fontweight='bold')
plt.xlabel('Tempo (s)')
plt.ylabel('Ângulo (graus)')
plt.legend(loc='best')
plt.tight_layout()
plt.savefig('9_dinamico_pitch11.png', dpi=300)

"""
# ==========================================
# GRÁFICO 3: Aceleração Bruta (A Prova da Vibração)
# ==========================================
# Este gráfico é essencial para provar para a banca que você REALMENTE vibrou a placa
plt.figure(figsize=(12, 4))
plt.plot(df['Tempo(s)'], df['AccX'], label='Acc X', color='crimson', alpha=0.7, linewidth=1)
plt.plot(df['Tempo(s)'], df['AccY'], label='Acc Y', color='forestgreen', alpha=0.7, linewidth=1)
plt.plot(df['Tempo(s)'], df['AccZ'], label='Acc Z (Gravidade)', color='navy', alpha=0.7, linewidth=1)

plt.title('Perfil de Aceleração Linear (Evidência Física do Degrau e da Vibração Injetada)', fontweight='bold')
plt.xlabel('Tempo (s)')
plt.ylabel('Aceleração (g)')
plt.legend(loc='best')
plt.tight_layout()
plt.savefig('10_dinamico_aceleracao.png', dpi=300)

print("Processamento concluído! Gráficos do Ensaio Dinâmico gerados com sucesso.")
plt.show()
"""