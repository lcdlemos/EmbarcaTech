#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

void set_leds(bool red, bool green, bool blue){
    gpio_put(LED_R_PIN, red);
    gpio_put(LED_G_PIN, green);
    gpio_put(LED_B_PIN, blue);
}

int main() {
    stdio_init_all();

    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    adc_init();

    adc_gpio_init(26);
    adc_gpio_init(27);

    while (1) {
        adc_select_input(0);
        uint adc_y_raw = adc_read();
        adc_select_input(1);
        uint adc_x_raw = adc_read();

        if(adc_x_raw > 4000){
            set_leds(1,0,0);
        } 
        else if(adc_x_raw < 100){
            set_leds(0,0,1);
        }
        else if(adc_y_raw > 4000){
            set_leds(0,1,0);
        } 
        else if(adc_y_raw < 100){
            set_leds(1,1,0);
        }
        else{
            set_leds(0,0,0);
        }
        
    }
}
