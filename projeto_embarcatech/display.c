#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"

const uint I2C_SDA = 14;                //DEFICAO DAS CONEXOES I2C UTILIZADAS PARA O DISPLAY
const uint I2C_SCL = 15;

// Inicialização do i2c
void display_init(){
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Processo de inicialização completo do OLED SSD1306
    ssd1306_init();
}

void intro(){
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

    //
    ssd1306_draw_string(ssd, 5, 10, "   Bem Vinda");
    ssd1306_draw_string(ssd, 5, 32, "  Programa de");
    ssd1306_draw_string(ssd, 5, 40, " Monitoramento");
    ssd1306_draw_string(ssd, 5, 48, "  de Contracao");
    render_on_display(ssd, &frame_area);

}

void espera(){
    //PREPARACAO DA AREA DE RENDERIZACAO DO DISPLAY
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    ssd1306_draw_string(ssd, 5, 20, "Aperte o Botao");
    ssd1306_draw_string(ssd, 5, 32, " Para Iniciar");
    render_on_display(ssd, &frame_area);
}

void espera_wifi(){
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

    //
    ssd1306_draw_string(ssd, 5, 20, "   Conectando");
    ssd1306_draw_string(ssd, 5, 32, "    ao WiFi");
    render_on_display(ssd, &frame_area);

}

void conectado_wifi(){
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

    //
    ssd1306_draw_string(ssd, 5, 20, "   WiFi");
    ssd1306_draw_string(ssd, 5, 32, " Conectado");
    render_on_display(ssd, &frame_area);

}

void espera_contagem(){
    //PREPARACAO DA AREA DE RENDERIZACAO DO DISPLAY
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    ssd1306_draw_string(ssd, 20, 20, "Monitorando");
    ssd1306_draw_string(ssd, 20, 35, " Contracoes");
    ssd1306_draw_string(ssd, 20, 50, "  Aguarde");
    render_on_display(ssd, &frame_area);
}

void resultado(int contracoes){
    //PREPARACAO DA AREA DE RENDERIZACAO DO DISPLAY
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    if(contracoes >= 3){
        if(contracoes == 3){
            ssd1306_draw_string(ssd, 2, 8, "  3 Contracoes");
            //render_on_display(ssd, &frame_area);
        }
        else{
            ssd1306_draw_string(ssd, 2, 8, "  4 Contracoes");
            //render_on_display(ssd, &frame_area);
        }
        ssd1306_draw_string(ssd, 2, 32, " Va ao Hospital");
        ssd1306_draw_string(ssd, 2, 40, "    Trabalho");
        ssd1306_draw_string(ssd, 2, 48, "    de Parto");
        render_on_display(ssd, &frame_area);
    }
    else{
        if(contracoes == 0){
            ssd1306_draw_string(ssd, 2, 10, " Sem Contracoes");
            //render_on_display(ssd, &frame_area);
        }
        else if(contracoes == 1){
            ssd1306_draw_string(ssd, 2, 10, "  1 Contracao");
            //render_on_display(ssd, &frame_area);
        }
        else{
            ssd1306_draw_string(ssd, 2, 10, "  2 Contracoes");
            //render_on_display(ssd, &frame_area);
        }
        ssd1306_draw_string(ssd, 2, 32, "Fique tranquila");
        ssd1306_draw_string(ssd, 2, 40, " Parto Nao Esta");
        ssd1306_draw_string(ssd, 2, 48, "    Proximo");
        render_on_display(ssd, &frame_area);
    }
}

void dados_nuvem(){
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

    //
    ssd1306_draw_string(ssd, 5, 20, "Dados Enviados");
    ssd1306_draw_string(ssd, 5, 32, "  Para Nuvem");
    render_on_display(ssd, &frame_area);

}