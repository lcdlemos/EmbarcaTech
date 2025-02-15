#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

const uint I2C_SDA = 14;                //DEFICAO DAS CONEXOES I2C UTILIZADAS PARA O DISPLAY
const uint I2C_SCL = 15;

#define LED_R_PIN 13                    //DEFINICAO DOS PINOS DOS LEDS
#define LED_G_PIN 11
#define LED_B_PIN 12

#define SW 22                           //DEFINICAO DO PINO DO BOTAO DO JOYSTICK

#define BUZZER_PIN 21                   //DEFINICAO DO PINO DO BOTAO DO BUZZER

const int VRX = 26;                     //DEFINICAO DO PINO DE LEITURA DO EIXO X (CONECTADO AO ADC)
const int VRY = 27;                     //DEFINICAO DO PINO DE LEITURA DO EIXO Y(CONECTADO AO ADC)
const int ADC_CHANNEL_0 = 0;            //DEFINICAO DO CANAL ADC PARA O EIXO X DO JOYSTICK
const int ADC_CHANNEL_1 = 1;            //DEFINICAO DO CANAL ADC PARA O EIXO Y DO JOYSTICK

const uint16_t LED_STEP = 200;          //DEFINICAO DO PASSO PARA O DUTY CYCLE DO LED
uint16_t led_level = 10;                //DEFINICAO DO NIVEL INICIAL DO PWM

const int LED_B = 12;                   //DEFINICAO DO PINO PARA CONTROLE DO LED AZUL VIA PWM
const int LED_R = 13;                   //DEFINICAO DO PINO PARA CONTROLE DO LED VERMELHO VIA PWM
const float DIVIDER_PWM = 16.0;         //DIVISOR FRACIONAL DO CLOCK PARA O PWM
const uint16_t PERIOD = 4096;           //PERIODO DO PWM (VALOR MAXIMO DO CONTADOR)
uint16_t led_b_level, led_r_level = 10; //INICIALIZACAO DOS NIVEIS DE PWM PARA OS LEDS
uint slice_led_b, slice_led_r;          //VARIAVEIS PARA AS FATIAS DE PWM CORRESPONDENTES AOS LEDS

const uint star_wars_notes[] = {        //DEFINICAO DAS NOTAS MUSICAIS
    330, 330, 330, 262, 392, 523, 330, 262,
    392, 523, 330, 659, 659, 659, 698, 523,
    415, 349, 330, 262, 392, 523, 330, 262,
    392, 523, 330, 659, 659, 659, 698, 523,
    415, 349, 330, 523, 494, 440, 392, 330,
    659, 784, 659, 523, 494, 440, 392, 330,
    659, 659, 330, 784, 880, 698, 784, 659,
    523, 494, 440, 392, 659, 784, 659, 523,
    494, 440, 392, 330, 659, 523, 659, 262,
    330, 294, 247, 262, 220, 262, 330, 262,
    330, 294, 247, 262, 330, 392, 523, 440,
    349, 330, 659, 784, 659, 523, 494, 440,
    392, 659, 784, 659, 523, 494, 440, 392
};

const uint note_duration[] = {          //DURACAO DAS NOTAS EM MILISSEGUNDOS
    500, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 500, 500, 500, 350, 150,
    300, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 650, 500, 150, 300, 500, 350,
    150, 300, 500, 150, 300, 500, 350, 150,
    300, 650, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 500, 500, 500, 350, 150,
    300, 500, 500, 350, 150, 300, 500, 350,
    150, 300, 500, 350, 150, 300, 500, 500,
    350, 150, 300, 500, 500, 350, 150, 300,
};

//------------------------------------------
// 0. FUNCAO QUE EXIBE A MENSAGEM NO DISPLAY
//------------------------------------------

void ExibirDisplay(int codigo){ 

    //PREPARACAO DA AREA DE RENDERIZACAO DO DISPLAY
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    //ZERA O DISPLAY E O PREPARA PARA ESCRITA
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    //ESCREVE NO DISPLAY A MENSAGEM DESEJADA
    //VERIFICA O VALOR DO CODIGO PARA SABER QUAL MENSAGEM EXIBIR
    if(codigo == 1){                    //MENSAGEM PARA O SINALIZAR O PROGRAMA 1 COMO ESCOLHIDO
        ssd1306_draw_string(ssd, 40, 0, "MENU");
        ssd1306_draw_string(ssd, 1, 20, "o Programa 1");
        ssd1306_draw_string(ssd, 1, 35, "  Programa 2");
        ssd1306_draw_string(ssd, 1, 50, "  Programa 3");
        render_on_display(ssd, &frame_area);
    }
    else if(codigo == 2) {              //MENSAGEM PARA O SINALIZAR O PROGRAMA 2 COMO ESCOLHIDO
        ssd1306_draw_string(ssd, 40, 0, "MENU");
        ssd1306_draw_string(ssd, 1, 20, "  Programa 1");
        ssd1306_draw_string(ssd, 1, 35, "o Programa 2");
        ssd1306_draw_string(ssd, 1, 50, "  Programa 3");
        render_on_display(ssd, &frame_area);
    }
    else if(codigo == 3){               //MENSAGEM PARA O SINALIZAR O PROGRAMA 3 COMO ESCOLHIDO
        ssd1306_draw_string(ssd, 40, 0, "MENU");
        ssd1306_draw_string(ssd, 1, 20, "  Programa 1");
        ssd1306_draw_string(ssd, 1, 35, "  Programa 2");
        ssd1306_draw_string(ssd, 1, 50, "o Programa 3");
        render_on_display(ssd, &frame_area);
    }
}

//ESCREVE NO DISPLAY A MENSAGEM DESEJADA
//VERIFICA O VALOR INFORMADO PARA SABER QUAL MENSAGEM EXIBIR DO PROGRAMA SELECIONADO NO MENU
void RodarPrograma(int prog){           //PREPARACAO DA AREA DE RENDERIZACAO DO DISPLAY
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    //ZERA O DISPLAY E O PREPARA PARA ESCRITA
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    if(prog == 1){                       //MENSAGEM QUE INDICAO O PROGRAMA 1 ESTA SENDO EXECUTADO
        ssd1306_draw_string(ssd, 25, 20, "Programa 1");
        ssd1306_draw_string(ssd, 15, 35, "LED JOYSTICK");
        render_on_display(ssd, &frame_area);
    }
    else if(prog == 2){                 //MENSAGEM QUE INDICAO O PROGRAMA 2 ESTA SENDO EXECUTADO
        ssd1306_draw_string(ssd, 25, 20, "Programa 2");
        ssd1306_draw_string(ssd, 25, 35, "BUZZER PWM");
        render_on_display(ssd, &frame_area);
    }
    else if(prog == 3){                 //MENSAGEM QUE INDICAO O PROGRAMA 3 ESTA SENDO EXECUTADO
        ssd1306_draw_string(ssd, 25, 20, "Programa 3");
        ssd1306_draw_string(ssd, 35, 35, "LED PWM");
        render_on_display(ssd, &frame_area);
    }
}

//---------------------------------------
// 1. FUNCAO PARA OS LEDS USANDO JOYSTICK
//---------------------------------------

//FUNCAO PARA CONFIGURACAO DO JOYSTICK (PINOS DE LEITURA E ADC)
void setup_joystick(){
  adc_init();                           //INICIALIZACAO DO MODULO ADC
  adc_gpio_init(VRX);                   //CONFIGURACAO DO PINO VRX (EIXO X) PARA ENTRADA ADC
  adc_gpio_init(VRY);                   //CONFIGURACAO DO PINO VRY (EIXO Y) PARA ENTRADA ADC

  gpio_init(SW);                        //INICIALIZACAO DO PINO DO BOTAO
  gpio_set_dir(SW, GPIO_IN);            //CONFIGURACAO DO PINO DO BOTAO COMO ENTRADA
  gpio_pull_up(SW);                     //ATIVACAO DO PULL-UP NO PINO DO BOTAO PARA EVITAR FLUTUACOES
}

//FUNCAO PARA CONFIGURACAO DO PWM DE UM LED (AZUL E VERMELHO)
void setup_pwm_led(uint led, uint *slice, uint16_t level){
  gpio_set_function(led, GPIO_FUNC_PWM);    //CONFIGURACAO DO PINO DO LED COMO SAIDA PWM
  *slice = pwm_gpio_to_slice_num(led);      //OBTENCAO DA FATIA DE PWM ASSOCIADO AO PINO DO LED
  pwm_set_clkdiv(*slice, DIVIDER_PWM);      //DEFINICAO DO DIVISOR DE CLOCK DO PWM
  pwm_set_wrap(*slice, PERIOD);             //CONFIGURACAO DO VALOR MAXIMO DO CONTADOR (PERIODO DO PWM)
  pwm_set_gpio_level(led, level);           //DEFINICAO DO NIVEL INICIAL DO PWM PARA O LED
  pwm_set_enabled(*slice, true);            //HABILITACAO DO PWM DA PORCAO CORRESPONDENTE AO LED
}

//FUNCAO DE CONFIGURACAO GERAL DO JOYSTICK PARA USO COM PWM
void setup(){
  stdio_init_all();                                 
  setup_joystick();                                 //CHAMADA DA CONFIGURACAO DO JOYSTICK
  setup_pwm_led(LED_B, &slice_led_b, led_b_level);  //CONFIGURACAO DO PWM PARA O LED AZUL
  setup_pwm_led(LED_R, &slice_led_r, led_r_level);  //CONFIGURACAO DO PWM PARA O LED VERMELHO
}

//FUNCAO PARA LER OS VALORES DOS EIXOS DO JOYSTICK (X E Y)
void joystick_read_axis(uint16_t *vrx_value, uint16_t *vry_value){
  
  //LEITURA DO VALOR DO EIXO X DO JOYSTICK
  adc_select_input(ADC_CHANNEL_0);                  //SELECAO DO CANAL ADC PARA O EIXO X
  sleep_us(2);                                      //PEQUENO DELAY PARA ESTABILIDADE
  *vrx_value = adc_read();                          //LEITURA DO VALOR DO EIXO X (0-4095)

  //LEITURA DO VALOR DO EIXO Y DO JOYSTICK
  adc_select_input(ADC_CHANNEL_1);                  //SELECAO DO CANAL ADC PARA O EIXO Y
  sleep_us(2);                                      //PEQUENO DELAY PARA ESTABILIDADE
  *vry_value = adc_read();                          //LEITURA DO VALOR DO EIXO Y (0-4095)
}

//FUNCAO PRINCIPAI PARA ACENDER OS LEDS USANDO PWM E OS NIVEIS DE LEITURA DO JOYSTICK
int joystick_pwm(){
  uint16_t vrx_value, vry_value, sw_value;          //VARIAVEIS PARA OS VALORES DO JOYSTICK E BOTAO
  setup();                                          //CHAMADA DA FUNCAO DE CONFIGURACAO
  
  while (true){
    sleep_ms(100);                                  //ESPERA DE 100 MILISSEGUNDOS ANTES DE REPETIR O CICLO

    joystick_read_axis(&vrx_value, &vry_value);     //LEITURA DOS VALORES DOS EIXOS DO JOYSTICK
    
    //AJUSTE DOS NIVEIS PWM DOS LEDS DE ACORDO COM OS VALORES DO JOYSTICK
    pwm_set_gpio_level(LED_B, vrx_value);           //AJUSTE DO BRILHO DO LED AZUL COM O VALOR DO EIXO X
    pwm_set_gpio_level(LED_R, vry_value);           //AJUSTE DO BRILHO DO LED AZUL COM O VALOR DO EIXO Y

    if(gpio_get(SW) == 0){                          //VERIFICACAO SE O BOTAO DE INTERRUPCAO FOI ACIONADO
        pwm_set_gpio_level(LED_B, 0);               //DESLIGA O LED AZUL ANTES DE SAIR DA ROTINA
        pwm_set_gpio_level(LED_R, 0);               //DESLIGA O LED VERMELHO ANTES DE SAIR DA ROTINA
        ExibirDisplay(1);                           //CHAMA A FUNCAO PARA EXIBIR O DISPLAY DO PROGRAMA 1
        break;                                      //ENCERRA A FUNCAO E VOLTA AO PROGRAMA PRINCIPAL
    }
  }
}

//-----------------------------------
// 2. FUNCAO PARA O BUZZER USANDO PWM
//-----------------------------------

void pwm_init_buzzer(uint pin) {                        //INICIALIZACAO DO PWM NO PINO DO BUZZER
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);               //AJUSTE DO DIVISOR DE CLOCK
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0);                         //DESLIGA O PWM INICIALMENTE
}

//FUNCAO PARA TOCAR NOTAS COM FREQUENCIA E DURACAO ESPECIFICAS
void play_tone(uint pin, uint frequency, uint duration_ms) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin, top / 2);                   //50% DE DUTY CYCLE

    sleep_ms(duration_ms);

    pwm_set_gpio_level(pin, 0);                         //DESLIGA O SOM APOS A DURACAO
    sleep_ms(50);                                       //PAUSA ENTRE AS NOTAS
}

//FUNCAO PRINCIPAL PARA TOCAE A MUSICA
void play_star_wars(uint pin) {
    for (int i = 0; i < sizeof(star_wars_notes) / sizeof(star_wars_notes[0]); i++) {
        if (star_wars_notes[i] == 0) {
            sleep_ms(note_duration[i]);
        } else {
            play_tone(pin, star_wars_notes[i], note_duration[i]);
        }

        if(gpio_get(SW) == 0){                      //VERIFICACAO SE O BOTAO DE INTERRUPCAO FOI ACIONADO
            sleep_ms(75);                           //ESPERA PARA ATUALIZACAO E EVITAR SOBRESCRITA
            pwm_set_gpio_level(pin, 0);             //DESLIGA O SOM PWM NO PINO
            ExibirDisplay(2);                       //CHAMA A FUNCAO DE EXIBICAO DO PROGRAMA 2
            break;                                  //ENCERRA O PROGRAMA E VOLTA A FUNCAO BUZZER
        }
    }
}

void buzzer() {
    stdio_init_all();
    pwm_init_buzzer(BUZZER_PIN);
    while(true){
      play_star_wars(BUZZER_PIN);                   //CHAMA O PROGRAMA PRINCIPAL DE TOCAR MUSICA
      break;                                        //ENCERRA O PROGRAMA E VOLTA AO PROGRAMA PRINCIPAL
    }
}

//--------------------------------------------------
// 3. FUNCAO PARA ACIONAMENTO DO LED AZUL USANDO PWM
//--------------------------------------------------

void setup_pwm(){
    uint slice;
    gpio_set_function(LED_B, GPIO_FUNC_PWM);        //CONFIGURACAO DO PINO DO LED PARA A FUNCAO PWM
    slice = pwm_gpio_to_slice_num(LED_B);           //OBTENCAO DA FATIA DO PWM ASSOCIADO AO PINO DO LED
    pwm_set_clkdiv(slice, DIVIDER_PWM);             //DEFINICAO DO DIVISOR DE CLOCK DO PWM
    pwm_set_wrap(slice, PERIOD);                    //DEFINICAO DO VALOR MAXIMO DO CONTADOR (PERIODO PWM)
    pwm_set_gpio_level(LED_B, led_level);           //DEFINICAO DO NIVEL INICIAL DE PWM NO PINO DO LED
    pwm_set_enabled(slice, true);                   //HABILITACAO DO PWM NA FATIA CORRESPONDENTE
}

int led_pwm(){
    uint up_down = 1;                           //VARIAVEL DE CONTROLE DO NIVEL DO LED (AUMENTA OU DIMINUI)
    stdio_init_all();                          
    setup_pwm();                                //CONFIGURACAO DO PWM

    while (true){
        pwm_set_gpio_level(LED_B, led_level);   //DEFINICAO DO NIVEL ATUAL DO PWM (DUTY CYCLE)
        sleep_ms(100);                          //ATRASO DE 100 MILISSEGUNDOS PARA LEITURA

        if (up_down){
            led_level += LED_STEP;              //INCREMENTA O NIVEL DO LED
            if (led_level >= PERIOD)
                up_down = 0;                    //MUDA PARA DIMINUIR QUANDO ATINGIR O PERIODO MAXIMO
        }
        else{
            led_level -= LED_STEP;              ////DECREMENTA O NIVEL DO LED
            if (led_level <= LED_STEP)
                up_down = 1;                    //MUDA PARA AUMENTAR QUANDO ATINGIR O PERIODO MINIMO
        }

        if(gpio_get(SW) == 0){                  //VERIFICACAO SE O BOTAO DE INTERRUPCAO FOI ACIONADO
            pwm_set_gpio_level(LED_B, 0);       //DESLIGA O LED AZUL ANTES DE SAIR
            ExibirDisplay(3);                   //CHAMADA DA FUNCAO DE EXIBICAO DO PROGRAMA 3
            break;                              //ENCERRA A FUNCAO E VOLTA AO PROGRAMA PRINCIPAL 
        }
    }
}

void inicializacao(){                           //FUNCAO DE INICIALIZACAO DOS PARAMETROS DA PLACA
    stdio_init_all();

    //INICIALIZACAO DO I2C
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    //INICIALIZACAO DOS LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    //INICIALIZACAO DO BOTAO DO JOYSTICK
    gpio_init(SW);
    gpio_set_dir(SW, GPIO_IN);
    gpio_pull_up(SW);

    //INICIALIZACAO DO DISPLAY OLED SSD1306
    ssd1306_init();

    //INICIALIZACAO DO ADC
    adc_init();
    adc_gpio_init(26);                      //USO APENAS DO CANAL Y NO PROGRAMA PRINCIPAL EXIBICAO DO MENU
}

int main() {                                //PROGRAMA PRINCIPAL DE EXIBICAO DO MENU E CHAMADA DE FUNCOES

    inicializacao();                        //INICIALIZACAO DOS PARAMETROS

    int pos = 20;                           //POSICAO INICIAL DO CURSOR (Y = 20) POSICAO DO PROGRAMA 1
    bool mudou = false;                     //VARIAVEL DE VERIFICACAO DA MUDANCA DE POSICAO DO CURSOR

    ExibirDisplay(1);                       //EXIBE O MENU INICIAL COM CURSOR NA POSICAO 1

    while (true) {
        adc_select_input(0);                        //SELECAO DO CANAL 0 PARA O ADC
        uint adc_y_raw = adc_read();                //LEITURA DO VALOR DO EIXO Y DO JOYSTICK NO ADC

        if(adc_y_raw > 4000){
            pos = pos - 15;                         //SOBE A POSICAO DO CURSOR NO DISPLAY
            if(pos < 20){                           //SE ATINGIR O MINIMO VALOR, MANTEM ESSE VALOR
                pos = 20;
            }
            mudou = true;                           //INFORMA QUE HOUVE MUDANCA NA POSICAO DO CURSOR
        } 
        else if(adc_y_raw < 100){
            pos = pos + 15;                         //DESCE A POSICAO DO CURSOR NO DISPLAY
            if(pos > 50){                           //SE ATINGIR O MAXIMO VALOR, MANTEM ESSE VALOR
                pos = 50;
            }
            mudou = true;                           //INFORMA QUE HOUVE MUDANCA NA POSICAO DO CURSOR
        }
        else{
            mudou = false;                          //NAO HOUVE MUDANCA NA POSICAO DO CURSOR
        }

        if(mudou){                          //FUNCAO DE EXIBICAO DO MENU DE ACORDO COM A POSICAO DO CURSOR
            if(pos == 20){
                ExibirDisplay(1);           //EXIBE O MENU COM O CURSOR NO PROGRAMA 1
            }
            else if(pos == 35){
                ExibirDisplay(2);           //EXIBE O MENU COM O CURSOR NO PROGRAMA 2
            }
            else if(pos == 50){
                ExibirDisplay(3);           //EXIBE O MENU COM O CURSOR NO PROGRAMA 3
            }
        }

        sleep_ms(75);                               //ATRASO DE 70 MILISSEGUNDOS PARA VERIFICACAO DO BOTA SW
        if(gpio_get(SW) == 0 && pos == 20){         //SE O BOTAO FOI APERTADO E ESTA NO PROGRAMA 1  
            RodarPrograma(1);                       //CHAMADA DA FUNCAO QUE INFORMA O PROGRAMA 1 EXECUTANDO
            joystick_pwm();                         //CHAMADA DA FUNCAO DO PROGRAMA 1 LED COM JOYSTICK E PWM
        }
        else if(gpio_get(SW) == 0 && pos == 35){    //SE O BOTAO FOI APERTADO E ESTA NO PROGRAMA 2
            RodarPrograma(2);                       //CHAMADA DA FUNCAO QUE INFORMA O PROGRAMA 2 EXECUTANDO
            buzzer();                               //CHAMADA DA FUNCAO DO PROGRAMA 2 BUZZER COM PWM
        }
        else if(gpio_get(SW) == 0 && pos == 50){    //SE O BOTAO FOI APERTADO E ESTA NO PROGRAMA 3
            RodarPrograma(3);                       //CHAMADA DA FUNCAO QUE INFORMA O PROGRAMA 3 EXECUTANDO
            led_pwm();                              //CHAMADA DA FUNCAO DO PROGRAMA 3 LED COM PWM
        }
    }
}
