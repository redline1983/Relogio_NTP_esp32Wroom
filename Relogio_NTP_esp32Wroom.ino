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
const char* _serverP = "pool.ntp.org?";             //NTP GLOBAL      //SERVIDOR NTP PRINCIPAL
const char* _serverB = "gps.ntp.br?";               //NTP BR          //SERVIDOR NTP SECUNDARIO
const char* _serverP2 = "189.45.192.3";             //PARA TESTES DE DECINCRONISMO

/*OUTROS SERVIDORES*/
//const char* _serverP = "189.45.192.3";           //NTP Unifique   
//const char* _serverB = "0.br.pool.ntp.org";      //NTP GLOBAL      

String _server_ativo = "erro Desconhecido nos Servidores NTP!";           //ARMAZENA O ESTADO DO SERVIDOR PARA FUTURAS BUSCAS DE HORARIO ATUALIZADO E ATUALIZAÇOES
String hora = "Ainda Desconhecida";               //ARMAZENA A HORA CERTA ATUALIZADA OU DO SISTEMA
String _timestamp_S_NTP = "Ainda desconhecido";
char hora_formatada[64];                          //PARA MANIPUNAR A TIMESTAMP INTERNO DO MICROCONTROLADOR
char data_formatada[64];                          //PARA MANIPUNAR A TIMESTAMP INTERNO DO MICROCONTROLADOR

time_t _time_stamp;
NTPClient ntp_P(udp, _serverP, -3 * 3600, 60000); //SETA UM OBJETO COM AS CONFIGURAÇÕES DO SERVER PRINCIPAL
NTPClient ntp_B(udp, _serverB, -3 * 3600, 60000); //SETA O SEGUNDO OBJETO COM AS CONF DO SERVER DE BACKUP
NTPClient ntp_P2(udp,_serverP2, -3 * 3600, 60000); //SETA UM OBJETO COM AS CONFIGURAÇÕES DO SERVER PRINCIPAL

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
        Serial.print ( " ... (((o))) ..." );
      }
  Serial.println();    
  Serial.println("WIFI Conectado com Sucesso!");
  Serial.println("Iniciando tentativa de conexão no servidor NTP...");
  verifica_atualiza_NTP();
   
      
}
void loop()
{  
                if (_server_ativo == _serverP){
                  hora = ntp_P.getFormattedTime();        /* PEGA A HORA DO SERVER NTP */
                  _timestamp_S_NTP = ntp_P.getEpochTime();/* PEGA A TIMESTAMP DO SERVER NTP */
                  //Serial.println("_serverP");
                }else{
                  hora = ntp_B.getFormattedTime();        /* PEGA A HORA DO SERVER NTP */
                  _timestamp_S_NTP = ntp_B.getEpochTime();/* PEGA A TIMESTAMP DO SERVER NTP */
                  //Serial.println("_serverB");
                }
                
                _time_stamp = time(NULL);  //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
                data = *gmtime(&_time_stamp);     //Converte o tempo atual e atribui na estrutura
                if (data.tm_sec == 30){  //NO SEGUNDO 50 DA DATA INTERNA DO ESP EXECUTA A FUNÇÃO TEMPOS(); 
                    Tempos();                                 
                }
                if ((data.tm_min ==  1)&&(data.tm_sec == 2)){                                         
                    Serial.println("Corrigido server p para 189...");
                    conexao_NTP("_serverP2");                    
                }
                strftime(data_formatada, 64, "%d/%m/%Y", &data);   //strftime(data_formatada, 64, "%d/%m/%Y %H:%M:%S", &data);   //Cria uma String formatada da estrutura "data" //Cria uma String formatada da estrutura "data"
                strftime(hora_formatada, 64, "%H:%M:%S", &data);   //Cria uma String formatada da estrutura "data"                
                

  if (_server_ativo.indexOf("Falha!")==0){                   
          Serial.print("Hora: ");
          Serial.write(hora_formatada);             //Mostra na Serial a data formatada    
          Serial.print(" <> ");        
          Serial.write(data_formatada);             //Mostra na Serial a data formatada            
          Serial.print(" <> ");
          Serial.print(" "+ String(_server_ativo));     /* Escreve a hora no monitor serial. */
                          
          Serial.println();
          delay(1000);   /* Espera 1 segundo. */                   
  }else{
          Serial.print("Hora: ");
          Serial.write(hora_formatada);             //Mostra na Serial a data formatada    
          Serial.print(" <> ");        
          Serial.write(data_formatada);             //Mostra na Serial a data formatada            
          Serial.print(" <> Server NTP Ativo:");
          Serial.print(" "+ String(_server_ativo));     /* Escreve a hora no monitor serial. */
          Serial.print(" <Hora nele:> ");        
          Serial.print(hora);
          Serial.print(" <timestamp NTP:> ");        
          Serial.print(_timestamp_S_NTP);
          Serial.print(" <timestamp ESP:> ");        
          Serial.print(_time_stamp);
          
          Serial.print(" <NTP - ESP:> ");
          if ((_time_stamp -(_timestamp_S_NTP.toInt ()) <= -10 )||(_time_stamp - (_timestamp_S_NTP.toInt ()) >= 10 )){
            Serial.print(_time_stamp - (_timestamp_S_NTP.toInt ()));
            Serial.print(" <<< DECINCRONISMO DETECTADO! ");
            Serial.println();            
            delay(1000);   /* Espera 1 segundo. */ //PARECE IMPORTANTE DAR UM DELAY ANTES DE USAR O SINCRONISMO, NÃO BUGA O SISTEMA            
            verifica_atualiza_NTP();               
          }else{
            Serial.print(_time_stamp - (_timestamp_S_NTP.toInt ())); 
            Serial.println();
            delay(1000);   /* Espera 1 segundo. */                                 
          }                                     
  }
}
void Tempos(){
    if ((_time_stamp - (_timestamp_S_NTP.toInt ()) <= -60 )||(_time_stamp - (_timestamp_S_NTP.toInt ()) >= 60 )){
      Serial.println("Sincronizando com servidore NTP...");        
      delay(1000);   /* Espera 1 segundo. */ //PARECE IMPORTANTE DAR UM DELAY ANTES DE USAR O SINCRONISMO, NÃO BUGA O SISTEMA            
      verifica_atualiza_NTP();               
    }else{
       //                               
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
            ntp_P.forceUpdate();
            hora = ntp_P.getFormattedTime();/* Armazena na variável hora, o horário atual. */
            
              timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
              tv.tv_sec = ntp_P.getEpochTime();     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
              settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.
                            
            _server_ativo = _serverP;
            return true;          
          } 
      }else{
        if (S == "_serverB"){
          ntp_B.begin(); 
          if ( !ntp_B.forceUpdate()) {  /*Tentetivas de conexão NTP */
            //Serial.print("Server NTP Principal Falhou!" );
            //_server_ativo = "Falha! dados não obtidos! Servidores NTP offline";
            return false;            
          }else{
            //Serial.println("Obtido dados do Server Principal NTP com Sucesso!");
            ntp_B.forceUpdate();
            hora = ntp_B.getFormattedTime();/* Armazena na variável hora, o horário atual. |||ntp_B.getEpochTime();|||*/

               timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
               tv.tv_sec = ntp_B.getEpochTime();     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
               settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.               
               
            _server_ativo = _serverB;
            return true;          
          }
        }else{
           if (S == "_serverP2"){
                                  ntp_P2.begin(); 
                                  if ( !ntp_P2.forceUpdate()) {  /*Tentetivas de conexão NTP */
                                    //Serial.print("Server NTP Principal Falhou!" );
                                    return false;            
                                  }else{
                                    //Serial.println("Obtido dados do Server Principal NTP com Sucesso!");
                                    ntp_P2.forceUpdate();
                                    hora = ntp_P2.getFormattedTime();/* Armazena na variável hora, o horário atual. */
                                    
                                      timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
                                      tv.tv_sec = ntp_P2.getEpochTime();     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
                                      settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.
                                                    
                                    _server_ativo = S;
                                    return true;          
                                  }
           }           
        }
      }
 }

void verifica_atualiza_NTP(){       
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
             if( _server_ativo == "erro Desconhecido nos Servidores NTP!"){
                _server_ativo = "Falha! dados não obtidos! Servidores NTP offline";
                timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
                tv.tv_sec = 397656000;     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
                settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.               
             }
          }     
      }
  }

  
//void app_main()
//{
  //timeval tv;  //Cria a estrutura temporaria para funcao abaixo.
  
  //tv.tv_sec = 1551355200;//Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo!
  //settimeofday(&tv, NULL);//Configura o RTC para manter a data atribuida atualizada.
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
//}
          /*
           struct tm {
             int tm_sec;         // seconds,  range 0 to 59        
             int tm_min;         // minutes, range 0 to 59           
             int tm_hour;        // hours, range 0 to 23             
             int tm_mday;        // day of the month, range 1 to 31  
             int tm_mon;         // month, range 0 to 11             
             int tm_year;        // The number of years since 1900   
             int tm_wday;        // day of the week, range 0 to 6    
             int tm_yday;        // day in the year, range 0 to 365  
             int tm_isdst;       // daylight saving time             
          };
          */
