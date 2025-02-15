#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "ssd1306.h"
#define I2C_PORT i2c1
#define I2C_SDA 15
#define I2C_SCL 14
#define LED 12
#define WIFI_SSID "brisa-1870184"  // Substitua pelo nome da sua rede Wi-Fi
#define WIFI_PASS "vouoc9ke" // Substitua pela senha da sua rede Wi-Fi

#define REDE "brisa-1870184"
#define SENHA "vouoc9ke"

ssd1306_t tela;//instancia do display Oled
// Buffer para resposta HTTP
char http_response[1024];

//função para desenhar texto no Oled
void print_texto(int x, int y, int tam, char *msg, bool cls){
    if(cls){
    ssd1306_clear(&tela);
    }
    ssd1306_draw_string(&tela,x,y,tam,msg);
    ssd1306_show(&tela);
}
// Função para criar a resposta HTTP
void create_http_response() {
    snprintf(http_response, sizeof(http_response),
             "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\n\r\n"
             "<!DOCTYPE html>"
             "<html>"
             "<head>"
             "  <meta charset=\"UTF-8\">"
             "  <title>Embarcatech Natal</title>"
             "</head>"
             "<body>"
             "  <h1>Controle do LED e Botões</h1>"
             "  <p><a href=\"/led/on\">Ligar LED</a></p>"
             "  <p><a href=\"/led/off\">Desligar LED</a></p>"
             "</body>"
             "</html>\r\n");
}
// Função de callback para processar requisições HTTP
static err_t http_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p == NULL) {
        // Cliente fechou a conexão
        tcp_close(tpcb);
        return ERR_OK;
    }
    // Processa a requisição HTTP
    char *request = (char *)p->payload;

    if (strstr(request, "GET /led/on")) {
        gpio_put(LED, 1);  // Liga o LED
    } else if (strstr(request, "GET /led/off")) {
        gpio_put(LED, 0);  // Desliga o LED
    }
    // Atualiza o conteúdo da página
    create_http_response();
    // Envia a resposta HTTP
    tcp_write(tpcb, http_response, strlen(http_response), TCP_WRITE_FLAG_COPY);
    // Libera o buffer recebido
    pbuf_free(p);

    return ERR_OK;
}
// Callback de conexão: associa o http_callback à conexão
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
    tcp_recv(newpcb, http_callback);  // Associa o callback HTTP
    return ERR_OK;
}
// Função de setup do servidor TCP
static void start_http_server(void) {
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        print_texto(2,2,2,"Erro ao criar PCB",true);        
        return;
    }

    // Liga o servidor na porta 80
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK) {
        print_texto(5,8,1,"Erro do servidor na",true);
        print_texto(5,20,1,"porta 80",false); 
        return;
    }
    pcb = tcp_listen(pcb);  // Coloca o PCB em modo de escuta
    tcp_accept(pcb, connection_callback);  // Associa o callback de conexão
    print_texto(2,26,1,"Servidor HTTP ON na",false);
    print_texto(2,34,1,"porta 80.",false);
    
}
//Função que faz a conexão com o Wifi
bool conexao_wifi(){
    sleep_ms(500);
    print_texto(1,2,1,"Start conexao Wi-Fi.",true);
    //Iniciando o Wi-Fi
    if (cyw43_arch_init()) {
        print_texto(1,2,2,"Erro ao inicializar o Wi-Fi.",true);
        return false;
    }
    cyw43_arch_enable_sta_mode();//Modo estação na rede
    print_texto(2,12,1,"Conectando ao Wi-Fi...",false);

    if (cyw43_arch_wifi_connect_timeout_ms(REDE, SENHA, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        print_texto(2,6,1,"Falha na conexao WiFi",true);
        return false;
    }else {
        ssd1306_clear(&tela);
        print_texto(2,2,1,"Conectado!",false);
        uint8_t *ip_address = (uint8_t*)&(cyw43_state.netif[0].ip_addr.addr);
        char ip_str[20];
        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
        print_texto(2,12,1,"IP:",false);
        print_texto(26,12,1,ip_str,false);
        sleep_ms(200);
    }
   return true;
}
//Setup e Inicialização
void inicializa(){
    stdio_init_all();
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init(&tela, 128, 64, 0x3C, I2C_PORT);
    ssd1306_clear(&tela);//limpeza inicial

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);

    if(conexao_wifi()){
        start_http_server();
    }

}

int main()
{
    inicializa();

    while (true) {
        cyw43_arch_poll();  // Necessário para manter o Wi-Fi ativo
        sleep_ms(100);      // Reduz o uso da CPU
    }

    cyw43_arch_deinit();  // Desliga o Wi-Fi (não será chamado, pois o loop é infinito)
    return 0;
}
