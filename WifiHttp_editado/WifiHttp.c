#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/init.h"


#define WIFI_SSID "LIGHT SIDE"
#define WIFI_PASS "AnakinSkywalker13"
#define THINGSPEAK_HOST "api.thingspeak.com"
#define THINGSPEAK_PORT 80

#define API_KEY "31Y51JU2F6713Z2P"  // Chave de escrita do ThingSpeak


struct tcp_pcb *tcp_client_pcb;
ip_addr_t server_ip;

// Função para ler a temperatura do sensor interno (ADC4)
int conta_pulsos(){
    int pulsos = 0;
    float tempo_inicial = 0, tempo_final = 0, duracao = 0;
    float inicio = 0, fim = 0, tempo_total = 0;
    bool mudou = false, nivel = false;

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
            }
        } 
        else{
            nivel = false;                              //Alterna o nivel para baixo
            if(!nivel && mudou){                        //Testa a descida do pulso
                tempo_final = get_absolute_time();      //Recebe o tempo final do pulso
                mudou = false;                          //Altera para nao entrar mais nesse teste

                duracao = (tempo_final - tempo_inicial)/1000000;    //Obtencao da largura em segundos
                if(duracao >= 1.5){                       //Se a contracao dura mais que 1.5 segundos
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
        printf("\n\nProcure o hospital. Voce esta em trabalho de parto");
    }
    else{
        printf("\n\nFique tranquila. O parto nao esta proximo.");
    }

    return pulsos;
}

// Callback quando recebe resposta do ThingSpeak
static err_t http_recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }
    printf("\n\nResposta do ThingSpeak: %.*s\n", p->len, (char *)p->payload);
    pbuf_free(p);
    return ERR_OK;
}

// 📌 Callback quando a conexão TCP é estabelecida
static err_t http_connected_callback(void *arg, struct tcp_pcb *tpcb, err_t err) {
    if (err != ERR_OK) {
        printf("Erro na conexão TCP\n");
        return err;
    }

    printf("Conectado ao ThingSpeak!\n");

    //int pulsos = conta_pulsos();  // Lê a temperatura
   
    int pulsos = conta_pulsos();
    char request[256];
    snprintf(request, sizeof(request),
        "GET /update?api_key=%s&field1=%i HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Connection: close\r\n"
        "\r\n",
        API_KEY, pulsos, THINGSPEAK_HOST);

    tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);
    tcp_recv(tpcb, http_recv_callback);

    return ERR_OK;
}

// 📌 Resolver DNS e conectar ao servidor
static void dns_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr) {
        printf("Endereço IP do ThingSpeak: %s\n", ipaddr_ntoa(ipaddr));
        tcp_client_pcb = tcp_new();
        tcp_connect(tcp_client_pcb, ipaddr, THINGSPEAK_PORT, http_connected_callback);
    } else {
        printf("Falha na resolução de DNS\n");
    }
}

int main() {
    stdio_init_all();
    adc_init();                                    // Inicializa o ADC para ler o sensor de temperatura
    adc_gpio_init(26);                             //Apenas usando o canal Y para verificar o pulso

    if (cyw43_arch_init()) {
        printf("Falha ao iniciar Wi-Fi\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return 1;
    }

    printf("Wi-Fi conectado!\n");

    while(true){
        dns_gethostbyname(THINGSPEAK_HOST, &server_ip, dns_callback, NULL);
        sleep_ms(10000);  // Espera 15 segundos antes de enviar novamente
    }        
    return 0;
}