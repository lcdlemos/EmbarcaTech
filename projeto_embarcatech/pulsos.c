#include "hardware/adc.h"
#include "pico/time.h"

#include "display.c"

int conta_pulsos(){
    int pulsos = 0;
    float tempo_inicial = 0, tempo_final = 0, duracao = 0;
    float inicio = 0, fim = 0, tempo_total = 0;
    bool mudou = false, nivel = false;

    leds_init();                                        //Inicializa os LEDs
    adc_init();                                         //Inicializa os canais ADC
    adc_gpio_init(26);                                  //Apenas usando o canal Y para verificar o pulso

    inicio = get_absolute_time();

    while (tempo_total < 10) {                          //Duracao de 60 segundos
        adc_select_input(0);
        uint adc_y_raw = adc_read();
        
        if(adc_y_raw > 4000){                           //Verificar se houve pulso
            nivel = true;                               //Alterna a variavel nivel para alto
            if(nivel && !mudou){                        //Testa se e subida do pulso, seu inicio
                tempo_inicial = get_absolute_time();    //Recebe o tempo inicial do pulso
                mudou = true;                           //Altera para nao entrar mais no teste
                set_leds(0,1,0);
            }
        } 
        else{
            nivel = false;                              //Alterna o nivel para baixo
            if(!nivel && mudou){                        //Testa a descida do pulso
                tempo_final = get_absolute_time();      //Recebe o tempo final do pulso
                mudou = false;                          //Altera para nao entrar mais nesse teste
                set_leds(0,0,0);
                duracao = (tempo_final - tempo_inicial)/1000000;    //Obtencao da largura em segundos
                if(duracao >= 1){                       //Se a contracao dura mais que 1.5 segundos
                    pulsos++;                           //Conta os pulsos acima de 10 segundos
                }
            }   
        }
        
        fim = get_absolute_time();
        tempo_total = (fim - inicio)/1000000;
        printf("\nTempo: %.2f s", tempo_total);
        
        sleep_ms(100);

    }

    printf("\n\nContracoes com 30 segundos ou mais: %i", pulsos);
    
    if(pulsos >= 3){
        printf("\nProcure o hospital. Voce esta em trabalho de parto");
        set_leds(0,0,1);
    }
    else{
        printf("\nFique tranquila. O parto nao esta proximo.");
        set_leds(1,0,0);
    }
    sleep_ms(200);

    resultado(pulsos);

    buzzer();                                           //Sinal sonoro indicando inicio do programa
    sleep_ms(2000);

    dados_nuvem();

    return pulsos;
}