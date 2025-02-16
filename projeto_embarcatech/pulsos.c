#include "hardware/adc.h"
#include "pico/time.h"

#include "display.c"

int conta_pulsos(){
    // Declaração das variáveis utilizadas
    int pulsos = 0;
    float tempo_inicial = 0, tempo_final = 0, duracao = 0;
    float inicio = 0, fim = 0, tempo_total = 0;
    bool mudou = false, nivel = false;

    espera_contagem();                                  //Informa no display que o monitoramento está sendo executado

    leds_init();                                        //Inicializa os LEDs
    adc_init();                                         //Inicializa os canais ADC
    adc_gpio_init(26);                                  //Apenas usando o canal Y para verificar o pulso

    inicio = get_absolute_time();

    //Na medicina, as contracoes sao contadas no intervalo de 10 minutos (600 segundos)
    //Com contrações que durem ao menos 30 segundos
    //Para efeito de demonstração, o tempo usado será de 1 minutos (60 segundos)
    //E a contração será representada por pulsos com duração de 3 segundos

    //Para uma aplicação de um caso real, altera-se o tempo do laço while de 60 para 600
    //Altera-se o teste dos if de 3 para 30
    //Troca-se a entrada do controle do joystick no ADC por um sensor de pressão

    while (tempo_total < 60) {                          //Duracao de 60 segundos
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
                if(duracao >= 3){                       //Se a contracao dura mais que 1.5 segundos
                    pulsos++;                           //Conta os pulsos acima de 3 segundos
                }
            }   
        }
        
        fim = get_absolute_time();
        tempo_total = (fim - inicio)/1000000;           //Obtença da duração do pulso em segundos
        printf("\nTempo: %.2f s", tempo_total);
        
        sleep_ms(100);

    }

    // Impressão da quatidade de pulsos medidos dentro dos padrões médicos
    // Pulsos com duração de mais de 30 segundos
    printf("\n\nContracoes com 30 segundos ou mais: %d", pulsos);
    
    // Impressão da mensagem via porta serial
    if(pulsos >= 3){
        printf("\nProcure o hospital. Voce esta em trabalho de parto");
        set_leds(0,0,1);
    }
    else{
        printf("\nFique tranquila. O parto nao esta proximo.");
        set_leds(1,0,0);
    }
    sleep_ms(200);

    resultado(pulsos);                                  // Chamada do display para exibir a mensagem do resultado em tela

    buzzer();                                           //Sinal sonoro indicando inicio do programa
    sleep_ms(2000);                                    //Espera de 2 segundos para atualizar display

    dados_nuvem();                                      //Informa no display que os dados já estão na nuvem

    return pulsos;
}