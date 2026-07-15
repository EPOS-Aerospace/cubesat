import serial
import time

porta = 'COM5'          # ajusta pra sua porta
baud = 115200
duracao_minutos = 1   

ser = serial.Serial(porta, baud, timeout=1)
tempo_fim = time.time() + duracao_minutos * 60

with open('dinam_test.csv', 'w', newline='') as f:
    cabecalho = ser.readline().decode('utf-8', errors='ignore').strip()
    f.write(cabecalho + '\n')
    print(f"Capturando por {duracao_minutos} minutos... mantenha o sensor PARADO.")

    linhas = 0
    while time.time() < tempo_fim:
        linha = ser.readline().decode('utf-8', errors='ignore').strip()
        if linha and ',' in linha:
            f.write(linha + '\n')
            linhas += 1
            if linhas % 500 == 0:
                print(f"{linhas} linhas capturadas...")

ser.close()
print(f"Captura concluída: dinam_test.csv ({linhas} linhas)")