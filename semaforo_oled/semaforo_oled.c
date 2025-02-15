#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

const uint I2C_SDA = 14;                //DEFINE AS CONEXOES I2C UTILIZADAS
const uint I2C_SCL = 15;

#define LED_R_PIN 13                    //DEFINE OS PINOS PARAS AS CORES DO LED RBG
#define LED_G_PIN 11
#define LED_B_PIN 12

//#define BTN_A_PIN 5                   //DEFINE O BOTAO A A SER USADO CASO DESEJASSE
#define BTN_B_PIN 6                     //DEFINE O BOTAO B A SER USADO

int B_state = 0;                        //DEFINE O ESTADO INICIAL DO BOTAO B COMO NAO PRESSIONADO

void SinalFechadoPedestre(){            //FUNCAO QUE FECHA O SINAL PARA TRAVESSIA DO PEDESTRE
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);   
}

void SinalAtencaoPedestre(){            //FUNCAO QUE SINALIZA PARA O PEDESTRE SE PREPARAR PARA TRAVESSIA
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

void SinalAbertoPedestre(){            //FUNCAO QUE ABRE O SINAL PARA TRAVESSIA DO PEDESTRE
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);
}

int EsperaChamada(int timeMS){          //FUNCAO QUE VERIFICA SE O BOTAO FOI APERTADO PELO PEDESTRE
    for(int i = 0; i < timeMS; i = i+100){
        B_state = !gpio_get(BTN_B_PIN);
        if(B_state == 1){
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

void ExibirDisplay(int codigo){         //FUNCAO QUE EXIBE A MENSAGEM NO DISPLAY PARA O PEDESTRE
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
    if(codigo == 1){                    //MENSAGEM PARA O SINAL VERDE E TRAVESSIA DO PEDESTRE
        ssd1306_draw_string(ssd, 5, 24, " SINAL ABERTO");
        ssd1306_draw_string(ssd, 5, 32, "ATRAVESSAR COM");
        ssd1306_draw_string(ssd, 5, 40, "   CUIDADO");
        render_on_display(ssd, &frame_area);
    }
    else if(codigo == 2) {              //MENSAGEM PARA O SINAL AMARELO E ATENCAO DO PEDESTRE
        ssd1306_draw_string(ssd, 5, 24, " SINAL DE");
        ssd1306_draw_string(ssd, 5, 32, "  ATENCAO");
        ssd1306_draw_string(ssd, 5, 40, "PREPARE SE");
        render_on_display(ssd, &frame_area);
    }
    else{                               //MENSAGEM PARA O SINAL VERMELHO E PEDESTRE AGUARDAR
        ssd1306_draw_string(ssd, 5, 32, "SINAL FECHADO");
        ssd1306_draw_string(ssd, 5, 40, "   AGUARDE");
        render_on_display(ssd, &frame_area);
    }  
}

void inicializa(){
    //INICIALIZACAO DOS STDIO PADROES
    stdio_init_all();

    //INICIALIZACAO DO I2C
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    //INICIANDO LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    //INICIANDO BOTAO B
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    //INICIALIZACAO DO DISPLAY OLED SSD1306
    ssd1306_init();
}

int main(){

    inicializa();

    while(true){
        SinalFechadoPedestre();         //ESPERA CHAMADA LENDO SE BOTAO B FOI APERTADO
        ExibirDisplay(3);
        B_state = EsperaChamada(8000);  //FUNCIONAMENTO NORMAL POR 8 SEGUNDOS

        if(B_state){                    //SE ALGUEM APERTOU O BOTAO - SAI DO MODO NORMAL DO SEMAFORO
            SinalAtencaoPedestre();     //SINAL DE ATENCAO AMARELO PARA OS PEDESTRES POR 5 SEGUNDOS
            ExibirDisplay(2);           //CHAMA EXIBICAO NO DISPLAY MANDANDO O CODIGO EQUIVALENTE
            sleep_ms(5000);
            

            SinalAbertoPedestre();      //SINAL VERDE PARA TRAVESSIA DOS PEDESTRES POR 15 SEGUNDOS
            ExibirDisplay(1);
            sleep_ms(15000);
        
        }
        else{                           //SE NINGUEM APERTOU O BOTAO - CONTINUA NO SEMAFORO MODO NORMAL          
            SinalAtencaoPedestre();     //SINAL DE ATENCAO AMARELO PARA OS PEDESTRES POR 2 SEGUNDOS
            ExibirDisplay(2);
            sleep_ms(2000);

            SinalAbertoPedestre();      //SINAL VERDE PARA TRAVESSIA DOS PEDESTRES POR 10 SEGUNDOS
            ExibirDisplay(1);
            sleep_ms(10000);
        }           
    }
    return 0;
}