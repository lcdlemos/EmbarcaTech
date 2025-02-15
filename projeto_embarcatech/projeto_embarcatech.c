#include <stdio.h>
#include "pico/stdlib.h"

#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/init.h"

#include "leds.c"
#include "buzzer.c"
#include "botao.c"
#include "pulsos.c"

#define WIFI_SSID "LIGHT SIDE"
#define WIFI_PASS "AnakinSkywalker13"
#define THINGSPEAK_HOST "api.thingspeak.com"
#define THINGSPEAK_PORT 80

#define API_KEY "31Y51JU2F6713Z2P"  // Chave de escrita do ThingSpeak

struct tcp_pcb *tcp_client_pcb;
ip_addr_t server_ip;

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

// Callback quando a conexão TCP é estabelecida
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

// Resolver DNS e conectar ao servidor
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

    display_init();                                     //Inicializa o display OLED
    leds_init();                                        //Inicializa os LEDs
    
    intro();                                            //Mensagem de boas vindas
    buzzer();                                           //Sinal sonoro indicando inicio do programa
    sleep_ms(1500);
    
    espera();

    botao();
    sleep_ms(100);

    if (cyw43_arch_init()) {
        printf("Falha ao iniciar Wi-Fi\n");
        return 1;
    }

    espera_wifi();

    cyw43_arch_enable_sta_mode();
    printf("Conectando ao Wi-Fi...\n");

    if (cyw43_arch_wifi_connect_blocking(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_MIXED_PSK)) {
        printf("Falha ao conectar ao Wi-Fi\n");
        return 1;
    }

    conectado_wifi();
    printf("Wi-Fi conectado!\n");

    espera_contagem();
    //conta_pulsos();

    while(true){
        dns_gethostbyname(THINGSPEAK_HOST, &server_ip, dns_callback, NULL);
        sleep_ms(60000);  // Espera 30 segundos antes de enviar novamente
    }        
    return 0;
}

