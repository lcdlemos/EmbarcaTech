#include "pico/stdlib.h"

#define LED_G_PIN 11
#define LED_B_PIN 12
#define LED_R_PIN 13

void set_leds(bool green, bool blue, bool red){
    gpio_put(LED_G_PIN, green);
    gpio_put(LED_B_PIN, blue);
    gpio_put(LED_R_PIN, red);
}

void leds_init(){
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
}