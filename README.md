Controle de LEDs e Display OLED com Raspberry Pi Pico

Este projeto utiliza um Raspberry Pi Pico para controlar LEDs RGB e exibir informações em um display OLED SSD1306, utilizando um joystick analógico.

### Componentes Utilizados

- Raspberry Pi Pico

- Joystick analógico

- Display OLED SSD1306 (I2C)

- LEDs RGB (vermelho, azul e verde)

- Push Button

### Funcionalidades

- Controle de LEDs RGB via joystick

- Exibição de posição no display OLED

- Efeito de borda alternável no display

- Interrupções para controle do LED verde e ativação/desativação do PWM

### Pinos Utilizados

#### Joystick

| Componente | Pino no Pico 
| ---------- | ------------ 
| VRX | 27 (ADC1)  
| VRY | 26 (ADC0)
| SW (Botão)| 22 (GPIO22)

#### LEDs e Botão

| Componente | Pino no Pico 
| ---------- | ------------ 
| LED Verde | 11 (GPIO11)
| LED Azul | 12 (PWM - GPIO12)
| LED Vermelho | 13 (PWM - GPIO13)
| Botão A | 5 (GPIO5)

#### Display OLED

| Pino | Função
| ---- | ------
| 14 | SDA (I2C)
| 15 | SCL (I2C)

### Dependências

Antes de compilar o código, é necessário instalar o SDK do Raspberry Pi Pico e a biblioteca ssd1306.h para comunicação com o display.

### Como Compilar e Executar

- Configure o ambiente de desenvolvimento do Raspberry Pi Pico.

- Compile o código utilizando o SDK do Pico.

- Faça o upload do firmware para o Pico.

- Utilize o joystick para controlar os LEDs e visualizar a movimentação no display.

### Funcionamento

- Movendo o joystick, os LEDs vermelho e azul alteram seu brilho conforme a posição.

- O display exibe um quadrado que se move conforme o joystick.

- Pressionando o botão do joystick, a borda do display muda e alterna o estado do LED verde.

- Pressionando o botão A, o controle de LEDs via PWM é ativado/desativado.
