# Radar de Velocidade com Sensores Magnéticos

## Descrição do Projeto
Projeto de um radar de velocidade que utiliza dois sensores magnéticos espaçados para realizar a contagem de eixos e o cálculo de velocidade de veículos. O sistema é baseado em threads para coleta de dados, processamento e exibição, simulando também uma câmera que reconhece a placa do veículo.

## Configuração e Execução no QEMU
Para rodar o código no QEMU, abra o terminal do VSCode dentro da pasta do projeto com o ambiente virtual ativado e utilize os comandos:

- Compilar o projeto: "west build -p always -b mps2/an385 -- -DDTC_OVERLAY_FILE=boards/arm/mps2_an385.overlay"
- Rodar o projeto: "west build -t run"
- Abrir o menu de configuração (menuconfig): "west build -t menuconfig"

## Opções Kconfig
- `CONFIG_RADAR_SENSOR_DISTANCE_MM`: Distância entre os sensores em milímetros.
- `CONFIG_RADAR_SPEED_LIMIT_LIGHT_KMH`: Limite de velocidade para veículos leves em km/h.
- `CONFIG_RADAR_SPEED_LIMIT_HEAVY_KMH`: Limite de velocidade para veículos pesados em km/h.
- `CONFIG_RADAR_WARNING_THRESHOLD_PERCENT`: Percentual do limite que ativa o display no modo “amarelo” (exemplo: 90% do limite).
- `CONFIG_RADAR_CAMERA_FAILURE_RATE_PERCENT`: Porcentagem de chance da câmera simular uma falha (0-100%).

## Arquitetura do Sistema
O projeto é baseado em threads, onde estas tem as seguintes funcionalidades:

- **Thread Sensores**: Responsável pela coleta dos dados dos sensores magnéticos, incluindo a contagem dos eixos via máquina de estados e cálculo da velocidade com base no tempo entre o acionamento dos sensores e a distância entre eles, Os dados de velocidade e contagem de eixos são enviados para a thread principal (main).
- **Thread de camera**: recebe um trigger via zBus e retorna uma placa no formato mercosul pelo mesmo canal.
- **Thread principal**: espera mensagem do sensor de velocidade via mensage queue e aciona a thread da câmera via zbus como trigger para executar a leitura de uma placa no formato Mercosul, recebendo a placa de volta pelo zbus, valida a placa e envia para a thread do display os valores velocidade, eixos e placa para a thread display:
- **Thread de display**: processa os valores enviados pela main em sua fila de mensagens e exibe corretamente no terminal QEMU as informações de placa, velocidade e eixos, também exibe se o veiculo estava na velocidaded correta, na zona de aviso e acima da velocidade permitida via cores de texto.
<img width="481" height="361" alt="arquitetura emb" src="https://github.com/user-attachments/assets/69ab791c-635b-4aef-b605-c7ff207c9d74" />

<img width="511" height="303" alt="state machine emb" src="https://github.com/user-attachments/assets/b4a34088-905f-457a-96a8-57b0bfd16d79" />


