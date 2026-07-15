# Estrutura do Projeto

O projeto foi desenvolvido utilizando o PlatformIO. As principais pastas são:

## `src/`

Contém os arquivos-fonte (`.cpp`) do projeto, incluindo o código principal do Kalman aplicado ao Roll e Pitch.

## `teste_estatico/`

Contém os resultados e arquivos referentes aos testes estáticos realizados com o sensor, utilizados para avaliar seu comportamento quando mantido em repouso. Há o arquivo CSV gerado pelo teste e o código python para gerar os gráficos.

## `teste_dinamico/`

Contém os resultados e arquivos referentes aos testes dinâmicos, realizados com o sensor em movimento para avaliar seu desempenho durante variações de orientação e movimento. Contém o arquivo CSV gerado pelo teste e o código para os gráficos.

## Demais arquivos

As demais pastas e arquivos pertencem à estrutura padrão do PlatformIO (`include`, `lib`, `test`, `.pio`, `.vscode`, etc.) ou correspondem a gráficos e dados gerados durante os experimentos.
