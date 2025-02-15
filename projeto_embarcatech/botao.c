#include <stdio.h>
#include "pico/stdlib.h"

#define BTN_B_PIN 6                     //DEFINE O BOTAO B A SER USADO

void botao(){
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    while(true){
        sleep_ms(100);
        if(gpio_get(BTN_B_PIN) == 0){
            break;
        }
    }
}