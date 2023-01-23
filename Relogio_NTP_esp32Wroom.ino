/* Projeto Relogio_NTP – ESP32Wroom: Protocolo NTP */

#include <NTPClient.h> /* https://github.com/arduino-libraries/NTPClient */
#include <WiFi.h> /* Biblioteca do WiFi. */
#include <time.h> /* Biblioteca do Relogio interno. */

/*-------- Configurações de Wi-fi----------- */
const char* WIFI_ssid = "elitec_testes"; /* Substitua pelo nome da rede */
const char* WIFI_password = "12345678";    /* Substitua pela senha */
String nome_ssid = WIFI_ssid;

/* -------- Configurações de relógio on-line----------- */
WiFiUDP udp;        
const char* _serverP = "pool.ntp.org";                          //NTP GLOBAL      //SERVIDOR NTP PRINCIPAL
const char* _serverB = "gps.ntp.br";                            //NTP BR          //SERVIDOR NTP SECUNDARIO
const char* _serverP2 = "189.45.192.3";                         //PARA TESTES DE DECINCRONISMO //ACABOU FICANADO NO CODIGO! //SERVER PROVEDOR UNIFIQUE

/*OUTROS SERVIDORES*/
//const char* _serverP = "a.st1.ntp.br";                        //NTP BR
//const char* _serverB = "0.br.pool.ntp.org";                   //NTP GLOBAL      

String _server_ativo = "erro Desconhecido nos Servidores NTP!"; //ARMAZENA O ESTADO DO SERVIDOR PARA FUTURAS BUSCAS DE HORARIO ATUALIZADO E ATUALIZAÇOES
String hora = "Ainda Desconhecida";                             //ARMAZENA A HORA CERTA ATUALIZADA OU DO SISTEMA PARA FINS DE IMPRESSAO E CONTROLE
String _timestamp_S_NTP = "Ainda desconhecido";                 //ARMAZENA O TIMESTAMP CERTO ATUALIZADO OU DO NTP PARA FINS DE IMPRESSAO E CONTROLE 
long _time_stamp_CTR = 1;
long _time_stamp_60s = -1;  
char hora_formatada[64];                                        //PARA MANIPUNAR A HORA TIMESTAMP INTERNA DO MICROCONTROLADOR
char data_formatada[64];                                        //PARA MANIPUNAR A DATA TIMESTAMP INTERNA DO MICROCONTROLADOR
boolean conexao_web_ntp = false;                                //SETA ESTADO DA CONEXAO COM O SERVER NTP O QUE VALIDA TAMBEM A CONEXAO COM A INTERNET
boolean _assincronia_NTP = false;                               //SETA ESTADO DO SINCRONISMO PARA QUE FAÇA TENTATIVA DE SINCRONIA A CADA MINUTO

time_t _time_stamp;
NTPClient  ntp_P(udp, _serverP, -3 * 3600, 60000);  //SETA UM OBJETO COM AS CONFIGURAÇÕES DO SERVER PRINCIPAL
NTPClient  ntp_B(udp, _serverB, -3 * 3600, 60000);  //SETA O SEGUNDO OBJETO COM AS CONF DO SERVER DE BACKUP
NTPClient ntp_P2(udp,_serverP2, -3 * 3600, 60000);  //SETA UM TERCEIRO OBJETO COM AS CONFIGURAÇÕES DO SERVER DE BACKUP 2

struct tm data; //CRIA UMA INFRAESTRUTURA QUE CONTEM AS INFORMAÇOES DA DATA.
          /*
           struct tm {
             int tm_sec;         // segundos,  faixa de 0 até 59        
             int tm_min;         // minutos, faixa de 0 até 59           
             int tm_hour;        // hora, faixa de 0 até 23             
             int tm_mday;        // dia do mês, faixa de 1 até 31  
             int tm_mon;         // mês, faixa de 0 até 11             
             int tm_year;        // O número para o ano depois de 1900   
             int tm_wday;        // dia da semana, faixa de 0 até 6    
             int tm_yday;        // dia no ano, faixa de 0 até 365  
             int tm_isdst;       // horário de verão             
          };
          */

void setup()
{ 
  ////POR HORA NO COD INICIALIZA E DEBUGA O COD VIA SERIAL!/////////////////////////////////////////// 
  Serial.begin(9600);
  Serial.println("Start...Iniciado Serial a 9600...");  
  ////-fim-POR HORA NO COD INICIALIZA E DEBUGA O COD VIA SERIAL!///////////////////////////////////////////
  
  ////POR HORA NO COD TENTA UMA CONEXÃO WIFI POR INSTANTES!///////////////////////////////////////////
  tentativa_conectar_WIFI();
  ////-fim-POR HORA NO COD TENTA UMA CONEXÃO WIFI POR INSTANTES!///////////////////////////////////////////   

  
      
}
                void temos_conexao(char* _ntp_){
                    if (_ntp_ == "ntp_P"){
                      ntp_P.begin(); 
                      if (ntp_P.forceUpdate()) { 
                        if (conexao_web_ntp == false){
                           _assincronia_NTP = true;
                           Tempos_de_sincronismo_NTP();
                           _server_ativo = _serverP;                      
                        }
                        hora = ntp_P.getFormattedTime();        /* PEGA A HORA DO SERVER NTP */
                        _timestamp_S_NTP = ntp_P.getEpochTime();/* PEGA A TIMESTAMP DO SERVER NTP */       
                      }else{
                         //ATRIBUIR ERRO DE WEB AQUI
                         Serial.print("|NTP OFFLINE| "); //Serial.print("|NTP offline: "+String(_serverP)+"|");
                         conexao_web_ntp = false;
                      }
                    }else{
                      if (_ntp_ == "ntp_B"){
                        ntp_B.begin(); 
                        if (ntp_B.forceUpdate()) {
                            if (conexao_web_ntp == false){
                              _assincronia_NTP = true;
                               Tempos_de_sincronismo_NTP();
                               _server_ativo = _serverB;                      
                            }
                           hora = ntp_B.getFormattedTime();        /* PEGA A HORA DO SERVER NTP */
                           _timestamp_S_NTP = ntp_B.getEpochTime();/* PEGA A TIMESTAMP DO SERVER NTP */     
                        }else{
                           //ATRIBUIR ERRO DE WEB AQUI
                           Serial.print("|NTP OFFLINE| "); //Serial.print("|NTP offline: "+String(_serverB)+"|");
                           conexao_web_ntp = false;
                        }
                      }else{
                        if (_ntp_ == "ntp_P2"){
                          ntp_P2.begin(); 
                          if (ntp_P2.forceUpdate()) {
                             if (conexao_web_ntp == false){
                                _assincronia_NTP = true;
                                Tempos_de_sincronismo_NTP();
                                _server_ativo = _serverP2;                      
                             }
                             hora = ntp_P2.getFormattedTime();        /* PEGA A HORA DO SERVER NTP */
                             _timestamp_S_NTP = ntp_P2.getEpochTime();/* PEGA A TIMESTAMP DO SERVER NTP */    
                          }else{
                             //ATRIBUIR ERRO DE WEB AQUI
                             Serial.print("|NTP OFFLINE| "); //Serial.print("|NTP offline: "+String(_serverP2)+"|");
                             conexao_web_ntp = false;
                          }
                        }
                      }
                    }
                }
void loop()
{                  
                if (_server_ativo == _serverP){
                   temos_conexao("ntp_P");             
                }else{
                  if (_server_ativo == _serverB){
                    temos_conexao("ntp_B");                                        
                  }else{
                    if (_server_ativo == _serverP2){
                      temos_conexao("ntp_P2");                      
                    }
                  }
                }                
                
                _time_stamp = time(NULL);    //Obtem o tempo atual em segundos. Utilize isso sempre que precisar obter o tempo atual
                if (_time_stamp_CTR != _time_stamp){
                  if (_time_stamp_60s == -1){
                     _time_stamp_60s++;                 
                  }else{
                      _time_stamp_60s = _time_stamp_60s + (_time_stamp - _time_stamp_CTR);
                  }
                  if (_time_stamp_60s >= 61){
                    Serial.print(" |60S...CORRIDOS| ");
                    _assincronia_NTP = true; // libera o sincronismo com o NTP
                    Tempos_de_sincronismo_NTP();
                    _time_stamp_60s = 1;
                  }
                  _time_stamp_CTR = _time_stamp;
                  Serial.print(" |-t:");
                  Serial.print(String(_time_stamp_60s));
                  Serial.print("| ");
                }
                data = *gmtime(&_time_stamp);//Converte o tempo atual e atribui na estrutura
                if (data.tm_sec == 30){      //NO SEGUNDO 50 DA DATA INTERNA DO ESP EXECUTA A FUNÇÃO TEMPOS(); 
                    //                              
                }                
                if ((data.tm_min ==  2)&&(data.tm_sec == 1)){                                         
                    Serial.println("TENTATIVA DE SINCRONISMO 189...");
                    //conexao_NTP("_serverP2");
                    _server_ativo = _serverP2;                    
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
          Serial.print("|H: ");
          Serial.write(hora_formatada);             //Mostra na Serial a data formatada    
          Serial.print("|");        
          Serial.write(data_formatada);             //Mostra na Serial a data formatada            
          Serial.print("| |S-NTP:");
          Serial.print(" "+ String(_server_ativo));     /* Escreve a hora no monitor serial. */
          Serial.print("| |H_NTP: ");        
          Serial.print(hora);
          Serial.print("| |T-stamp NTP: ");        
          Serial.print(_timestamp_S_NTP);
          Serial.print("| |T-stamp ESP: ");        
          Serial.print(_time_stamp);
          
          Serial.print("| |NTP - ESP: ");
          if ((_time_stamp -(_timestamp_S_NTP.toInt ()) <= -30 )||(_time_stamp - (_timestamp_S_NTP.toInt ()) >= 30 )){
            Serial.print(_time_stamp - (_timestamp_S_NTP.toInt ()));
            Serial.print("|");
            Serial.print("< Alerta! |ASSINCRONIA| ");
            Serial.println();            
            delay(1000);   /* Espera 1 segundo. */ //PARECE IMPORTANTE DAR UM DELAY ANTES DE USAR O SINCRONISMO, NÃO BUGA O SISTEMA                                  
          }else{
            Serial.print(_time_stamp - (_timestamp_S_NTP.toInt ())); 
            Serial.println("|");
            delay(1000);   /* Espera 1 segundo. */                                 
          }                                     
  }
  
}
void Tempos_de_sincronismo_NTP(){
  if ( WiFi.status() != WL_CONNECTED ) {
       tentativa_conectar_WIFI();        
  }else{
    if ((_time_stamp - (_timestamp_S_NTP.toInt ()) <= -60 )||(_time_stamp - (_timestamp_S_NTP.toInt ()) >= 60 )){
      if (_assincronia_NTP == true){
          Serial.println("Sincronizando com servidor NTP...");        
          delay(100);   /* Espera 1 segundo. */ //PARECE IMPORTANTE DAR UM DELAY ANTES DE USAR O SINCRONISMO, NÃO BUGA O SISTEMA            
          verifica_atualiza_NTP();
          _assincronia_NTP = false;
      }               
    }else{
      //                                   
    } 
  }    
}

bool conexao_NTP(String S){   
      if (S == "_serverP"){
          ntp_P.begin(); 
          if ( !ntp_P.forceUpdate()) {  /*Tentetivas de conexão NTP */
            //Serial.print("Server NTP Principal Falhou!" );
            return false;            
          }else{
            conexao_web_ntp = true;
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
            conexao_web_ntp = true;
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
                                    conexao_web_ntp = true;
                                    ntp_P2.forceUpdate();
                                    hora = ntp_P2.getFormattedTime();/* Armazena na variável hora, o horário atual. */
                                    
                                      timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
                                      tv.tv_sec = ntp_P2.getEpochTime();     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
                                      settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.
                                                 
                                    _server_ativo = _serverP2;
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
             Serial.println("Iniciando tentativa de conexão no servidor NTP de BACKUP 2...");
             if (conexao_NTP("_serverP2")) {  
                Serial.println("Obtido dados do Server NTP BACKUP 2 |"+String(_server_ativo)+"| com Sucesso!");
             }else{
                Serial.println("Server NTP BACKUP  |"+String(_serverP2)+"| Falhou!" );
                Serial.println("Sem hora atualizada no momento! nova tentativa em instantes..." ); 
                if( _server_ativo == "erro Desconhecido nos Servidores NTP!"){
                  _server_ativo = "Falha! dados não obtidos! Servidores NTP offline";
                  timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
                  tv.tv_sec = 397656000;     //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! // 12:00 08/08/1982
                  settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.               
                }
             }
          }     
      }
  }
  
void tentando_NTP(){
    ////POR HORA NO COD TENTA UMA CONEXÃO NTP POR INSTANTES!///////////////////////////////////////////   
    Serial.println("Iniciando tentativa de conexão no servidor NTP...");  
    verifica_atualiza_NTP();
    ////-fim-POR HORA NO COD TENTA UMA CONEXÃO NTP POR INSTANTES!///////////////////////////////////////////   
  }

  void tentativa_conectar_WIFI(){
      Serial.print("Iniciando tentativas de conexão WIFI SSID:");
      Serial.write(WIFI_ssid);
      Serial.print(" password:");
      Serial.write(WIFI_password);
      Serial.println();
      WiFi.begin(WIFI_ssid, WIFI_password);
      for (int i = 0; i <= 10; i++) {   //TENTATIVA DE CONEÇÃO POR ALGUNS SEGUNDOS DEPOIS SEGUE COM O CODIGO     
       /* Espera a conexão. */
          if ( WiFi.status() != WL_CONNECTED ) {
            delay ( 500 );
            Serial.println(" ... (((o))) ...");
          }
      }    
      Serial.println();
      if ( WiFi.status() == WL_CONNECTED ) {    
        Serial.println("WIFI Conectado com Sucesso!");
        tentando_NTP();           //FAZ UMA TENTATIVA DE CONXÃO NTP SOMENTE SO WIFI ESTIVER OK!
      }else{
        Serial.println("WIFI DESCONECTADO!");
        Serial.println("Sem hora atualizada no momento! nova tentativa em instantes..." ); 
        if( _server_ativo == "erro Desconhecido nos Servidores NTP!"){
          _server_ativo = "Falha! dados não obtidos! Servidores NTP offline";
          timeval tv;                //Cria a estrutura temporaria para funcao abaixo.  
          tv.tv_sec = 397656000;     // 12:00 08/08/1982 //Atribui minha data atual. Voce pode usar o NTP para isso ou o site citado no artigo! 
          settimeofday(&tv, NULL);   //Configura o RTC para manter a data atribuida atualizada.               
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
          
