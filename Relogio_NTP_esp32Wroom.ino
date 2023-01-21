/* Projeto Curto Circuito – ESP32: Protocolo NTP */

#include <NTPClient.h> /* https://github.com/arduino-libraries/NTPClient */
#include <WiFi.h> /* Biblioteca do WiFi. */

/*-------- Configurações de Wi-fi----------- */
const char* WIFI_ssid = "Elitec_Huawei_AC"; /* Substitua pelo nome da rede */
const char* WIFI_password = "10503879";    /* Substitua pela senha */
String nome_ssid = WIFI_ssid;
/* -------- Configurações de relógio on-line----------- */
WiFiUDP udp;
//NTPClient ntp(udp, "pool.ntp.org", -3 * 3600, 60000); /* Cria um objeto "NTP" com as configurações.utilizada no Brasil */
//NTPClient ntp(udp, "pool.ntp.org", 0, 60000); /* Cria um objeto "NTP" com as configurações.utilizada no Brasil */
//NTPClient ntp(udp, "gps.ntp.br", -3 * 3600, 60000);
const char* _serverP = "gps.ntp.br";   //NTP br
const char* _serverB = "189.45.192.3"; //NTP Unifique
String server ="";

String hora;            /* Variável que armazena */

void setup()
{
  Serial.begin(9600);
  Serial.print("Start...Iniciado Serial a 9600...Iniciando tentativas de conexão WIFI SSID:");
  Serial.write(WIFI_ssid);
  Serial.print(" password:");
  Serial.write(WIFI_password);
  Serial.println();
  
  WiFi.begin(WIFI_ssid, WIFI_password);
   /* Espera a conexão. */
      while ( WiFi.status() != WL_CONNECTED ) {
        delay ( 500 );
        Serial.print ( " ... (((o))) <>" );
      }
  Serial.println();    
  Serial.println("WIFI Conectado com Sucesso!");
  Serial.println("Iniciando tentativa de conexão no servidor NTP...");
  
      if (conexao_NTP(_serverP)) {  /*Tentetivas de conexão NTP */
        delay ( 500 );
        Serial.print( "Server NTP Principal Falhou...tentando conexão de backup!" );
      }else{
        Serial.println("Iniciado Server Principal NTP com Sucesso!");          
      } 
      
}
void loop()
{
  /* Armazena na variável hora, o horário atual. */
  hora = ntp.getFormattedTime(); 
  Serial.println(hora);     /* Escreve a hora no monitor serial. */
  delay(1000);              /* Espera 1 segundo. */
}

void conexao_NTP(char* S){
  //NTPClient ntp(udp, S, -3 * 3600, 60000);
  ntp.begin();               /* Inicia o protocolo */  
  if ( !ntp.forceUpdate()) {  /*Tentetivas de conexão NTP */
        //return false;
        //Serial.print( "Server NTP Principal Falhou...tentando conexão de backup!" );
      }else{
        //Serial.println("Iniciado Server Principal NTP com Sucesso!");
        return true;          
      }    
}
