/* Projeto Relogio_NTP – ESP32Wroom: Protocolo NTP */

#include <NTPClient.h> /* https://github.com/arduino-libraries/NTPClient */
#include <WiFi.h> /* Biblioteca do WiFi. */
#include <time.h> /* Biblioteca do Relogio interno. */

/*-------- Configurações de Wi-fi----------- */
const char* WIFI_ssid = "Elitec_Huawei_AC"; /* Substitua pelo nome da rede */
const char* WIFI_password = "10503879";    /* Substitua pela senha */
String nome_ssid = WIFI_ssid;

/* -------- Configurações de relógio on-line----------- */
WiFiUDP udp;        
const char* _serverP = "gps.ntp.br2";             //NTP br      //SERVIDOR NTP PRINCIPAL
const char* _serverB = "189.45.192.3";           //NTP Unifique//SERVIDOR NTP SECUNDARIO
String _server_ativo = "Desconhecido!";           //ARMAZENA O ESTADO DO SERVIDOR PARA FUTURAS BUSCAS DE HORARIO ATUALIZADO E ATUALIZAÇOES
String hora = "Ainda Desconhecida";               //ARMAZENA A HORA CERTA ATUALIZADA OU DO SISTEMA
char data_formatada[64];                          //PARA MANIPUNAR A TIMESTAMP INTERNO DO MICROCONTROLADOR
NTPClient ntp_P(udp, _serverP, -3 * 3600, 60000); //SETA UM OBJETO COM AS CONFIGURAÇÕES DO SERVER PRINCIPAL
NTPClient ntp_B(udp, _serverB, -3 * 3600, 60000); //SETA O SEGUNDO OBJETO COM AS CONF DO SERVER DE BACKUP

struct tm data; //Cria a estrutura que contem as informacoes da data.

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
         
      if (conexao_NTP("_serverP")) {  
         Serial.println("Obtido dados do Server Principal NTP |"+String(_server_ativo)+"| com Sucesso!");
      }else{
          Serial.println("Server NTP Principal  |"+String(_serverP)+"| Falhou!" ); 
          Serial.println("Iniciando tentativa de conexão no servidor NTP de BACKUP...");    
          if (conexao_NTP("_serverB")) {  
             Serial.println("Obtido dados do Server NTP BACKUP |"+String(_server_ativo)+"| com Sucesso!");
          }else{
             Serial.println("Server NTP BACKUP  |"+String(_serverB)+"| Falhou!" );
             Serial.println("Sem hora atualizada no momento! nova tentativa em 1 minuto..." ); 
             
              timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
              tv.tv_sec = 397656000;     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
              settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.               
          }     
      }
   
      
}
void loop()
{
          time_t tt = time(NULL);  //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
          data = *gmtime(&tt);     //Converte o tempo atual e atribui na estrutura                    
          //strftime(data_formatada, 64, "%d/%m/%Y %H:%M:%S", &data);   //Cria uma String formatada da estrutura "data"
          char data_formatada2[64];
          strftime(data_formatada2, 64, "%d/%m/%Y", &data);   //Cria uma String formatada da estrutura "data"
          strftime(data_formatada, 64, "%H:%M:%S", &data);   //Cria uma String formatada da estrutura "data"

  if (_server_ativo.indexOf("Falha!")==0){                   
          Serial.print("Hora: ");
          Serial.write(data_formatada);             //Mostra na Serial a data formatada              
          Serial.print(" <>");
          Serial.print(" "+ String(_server_ativo));     /* Escreve a hora no monitor serial. */
          Serial.print(" <> ");        
          Serial.write(data_formatada2);             //Mostra na Serial a data formatada                
          Serial.println();
          delay(1000);   /* Espera 1 segundo. */                   
  }else{
     Serial.print("Hora: " + String(hora));
     Serial.print(" <> ");        
     Serial.write(data_formatada2);             //Mostra na Serial a data formatada                
     Serial.println();
     delay(1000);  /* Espera 1 segundo. */       
  }
}

bool conexao_NTP(String S){   
      if (S == "_serverP"){
          ntp_P.begin(); 
          if ( !ntp_P.forceUpdate()) {  /*Tentetivas de conexão NTP */
            //Serial.print("Server NTP Principal Falhou!" );
            return false;            
          }else{
            //Serial.println("Obtido dados do Server Principal NTP com Sucesso!");
            hora = ntp_P.getFormattedTime();/* Armazena na variável hora, o horário atual. */
            
              timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
              tv.tv_sec = ntp_B.getEpochTime();     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
              settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.
                            
            _server_ativo = _serverP;
            return true;          
          } 
      }else{
        if (S == "_serverB"){
          ntp_B.begin(); 
          if ( !ntp_B.forceUpdate()) {  /*Tentetivas de conexão NTP */
            //Serial.print("Server NTP Principal Falhou!" );
            _server_ativo = "Falha! dados não obtidos! servers NTP offline";
            return false;            
          }else{
            //Serial.println("Obtido dados do Server Principal NTP com Sucesso!");
            hora = ntp_B.getFormattedTime();/* Armazena na variável hora, o horário atual. |||ntp_B.getEpochTime();|||*/

               timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
               tv.tv_sec = ntp_B.getEpochTime();     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
               settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.
               
            _server_ativo = _serverB;
            return true;          
          }
        }else{
           //
        }
      }
 }


void app_main()
{
  timeval tv;  //Cria a estrutura temporaria para funcao abaixo.
  
  tv.tv_sec = 1551355200;//Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo!
  settimeofday(&tv, NULL);//Configura o RTC para manter a data atribuida atualizada.
  /*
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));//Espera 1 seg
    time_t tt = time(NULL);  //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
    data = *gmtime(&tt);     //Converte o tempo atual e atribui na estrutura
       
    strftime(data_formatada, 64, "%d/%m/%Y %H:%M:%S", &data);   //Cria uma String formatada da estrutura "data"
    printf("\nUnix Time: %d\n", int32_t(tt));                   //Mostra na Serial o Unix time
    printf("Data formatada: %s\n", data_formatada);             //Mostra na Serial a data formatada
    //
      //Com o Unix time, podemos facilmente controlar acoes do MCU por data, visto que utilizaremos os segundos
      //e sao faceis de usar em IFs
      //Voce pode criar uma estrutura com a data desejada e depois converter para segundos (inverso do que foi feito acima)
      //caso deseje trabalhar para atuar em certas datas e horarios
      //No exemplo abaixo, o MCU ira printar o texto **APENAS** na data e horario (28/02/2019 12:00:05) ate (28/02/2019 12:00:07)
    //
    if (tt >= 1551355205 && tt < 1551355208)//Use sua data atual, em segundos, para testar o acionamento por datas e horarios
    {
      printf("Acionando carga durante 3 segundos...\n");
    }
  }
  */
}
