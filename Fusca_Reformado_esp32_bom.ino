#include <Bridge.h>
#include <BridgeClient.h>
#include <BridgeServer.h>
#include <BridgeSSLClient.h>
#include <BridgeUdp.h>
#include <Console.h>
#include <FileIO.h>
#include <HttpClient.h>
#include <Mailbox.h>
#include <Process.h>
#include <YunClient.h>
#include <YunServer.h>

#include <BLEDevice.h>
#define ADDRESS "c1:b4:70:76:1b:ae" //Endereço do iTag, conseguido pelo próprio scan
#define ADDRESS1 "c1:b4:70:73:cc:3c" //Endereço do iTag, conseguido pelo próprio scan
#define SCAN_INTERVAL 1000 //intervalo entre cada scan
#define TARGET_RSSI -125 //RSSI limite para ligar o relê.
#define MAX_MISSING_TIME 6000 //Tempo para desligar o relê desde o momento que o iTag não for encontrado

BLEScan* pBLEScan; //Variável que irá guardar o scan
uint32_t lastScanTime = 0; //Quando ocorreu o último scan
boolean found = false; //Se encontrou o iTag no último scan
uint32_t lastFoundTime = 0; //Tempo em que encontrou o iTag pela última vez
int rssi = 0;

//Callback das chamadas ao scan
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
{
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //Sempre que um dispositivo for encontrado ele é mostrado aqui
        //Serial.print("Dispositivo : ");      
        //Serial.println(advertisedDevice.toString().c_str());

        //Se for o dispositivo que esperamos
        if(advertisedDevice.getAddress().toString() == ADDRESS or advertisedDevice.getAddress().toString() == ADDRESS1)
        {
            //Marcamos como encontrado, paramos o scan e guardamos o rssi
            found = true;
            advertisedDevice.getScan()->stop();
            rssi = advertisedDevice.getRSSI();
           // Serial.println("RSSI: " + rssi);
        }
    }
};



//*********************************************************************************************
//                                VARIAVEIS E DECLARACOES
//*********************************************************************************************
 
    #define ChavePresenca 2 
    #define BotaoPorta 15
    #define BotaoPartida 4
    #define Trava 5
    #define Destrava 18
    #define PriEstPainel 19
    #define FreioEmb 21
    #define Partida 22
    #define Buzzer 32
    #define FuncMotor 23
    #define SegEstPainel 13
    #define CarroMovimento 14
    #define SwithPorta 12
    #define Buzina 26
    #define Trava_Alarme 34
    #define Destrava_Alarme 35
    void (*funcReset)() = 0;
    boolean Acesso;
    int ControlePorta = 0;
    int Click = 0;
    int ControleTrava = 0;
    int TempoPartida = 200;
    int ControleAlarme = 0;
    unsigned long PreviousMillisTempo = 0;
    const long IntervaloTempo = 5000;
    // -------- Utilizado pelo temporizador da trava ------------
    const long intervaloTrava = 10000;
    boolean isTemporizaTrava = false;
    unsigned long millisInicioTemporizadorTrava = 0;
    // -----------------------------------------------------------

//*********************************************************************************************
/*
#define ChavePresenca 2 
#define BotaoPorta 15
#define BotaoPartida 4
#define Trava 16
#define Destrava 17
#define PriEstPainel 5
#define FreioEmb 18
#define Partida 19
#define Buzzer 32
#define FuncMotor 23
#define SegEstPainel 13
#define CarroMovimento 14
#define SwithPorta 12
#define Buzina 26
void (*funcReset)() = 0;
boolean Acesso;
int ControlePorta = 0;
int Click = 0;
int ControleTrava = 0;
int TempoPartida = 200;
*/
//*********************************************************************************************
//                                           SETUP
//*********************************************************************************************


void setup()
{
Serial.begin(115200);
Serial.print("Sistema Eletrônico de Partida");
pinMode(SwithPorta,      INPUT_PULLUP);
pinMode(ChavePresenca,         OUTPUT);
pinMode(BotaoPorta,             INPUT);
pinMode(BotaoPartida,           INPUT);
pinMode(Trava,                 OUTPUT);
pinMode(Destrava,              OUTPUT);
pinMode(PriEstPainel,          OUTPUT);
pinMode(FreioEmb,              OUTPUT);
pinMode(Partida,               OUTPUT);
pinMode(Buzina,                OUTPUT);
pinMode(Buzzer,                OUTPUT);
pinMode(FuncMotor,              INPUT);
pinMode(SegEstPainel,          OUTPUT);
pinMode(CarroMovimento,         INPUT);
pinMode(Trava_Alarme,           INPUT);
pinMode(Destrava_Alarme,        INPUT);

digitalWrite(SwithPorta,         HIGH);
digitalWrite(ChavePresenca,       LOW);
digitalWrite(BotaoPorta,          LOW);
digitalWrite(BotaoPartida,        LOW);
digitalWrite(Trava,               LOW);
digitalWrite(Destrava,            LOW);
digitalWrite(FreioEmb,            LOW);
digitalWrite(Partida,             LOW);
digitalWrite(Buzzer,              LOW);
digitalWrite(PriEstPainel,        LOW);
digitalWrite(Buzina,              LOW);
digitalWrite(FuncMotor,           LOW);
digitalWrite(SegEstPainel,        LOW);
digitalWrite(CarroMovimento,      LOW);
digitalWrite(Trava_Alarme,        LOW);
digitalWrite(Destrava_Alarme,     LOW);

//Guardamos a referência e configuramos o objeto scan
    BLEDevice::init(""); 
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
}

//************************************DECLARAÇÃO DAS FUNÇÕES**********************************
bool BotPar();
bool BotPor();
void Travar_Alarme();
void Destravar_Alarme();
void Presenca();
void Botao_Porta();
void Abre_Porta();
void Destravar();
void Travar();
void PartidaCarro();
void IgnicaoMotor(); //Somente utilizado para temporizar a partida....
void Ausencia();
void Buzinar();
void travaTemporizada();
void desligaTemporizadorTrava();
//********************************************************************************************

void loop(){   
    unsigned long CurrentMillis = millis();
    uint32_t now = millis(); //Tempo em milissegundos desde o boot

    if(found){ //Se econtrou o iTag no último scan
        lastFoundTime = millis(); //Tempo em milissegundos de quando encontrou
        found = false;
        
        if(rssi > TARGET_RSSI){ //Se está dentro do limite, ligamos o relê
            digitalWrite(ChavePresenca, HIGH);

        }
        else{ //senão desligamos
            digitalWrite(ChavePresenca, LOW);
        }
    }
    //Se não encontrou e o tempo desde a última vez que econtrou for maior que o tempo máximo de desaparecido
    else if(now - lastFoundTime > MAX_MISSING_TIME){
        digitalWrite(ChavePresenca, LOW);  //Desligamos o relê
    }
    
    if(now - lastScanTime > SCAN_INTERVAL){ //Se está no tempo de fazer scan
        //Marca quando ocorreu o último scan e começa o scan
        lastScanTime = now;
        pBLEScan->start(1);
    }

    if (digitalRead(Destrava_Alarme) == HIGH && ControleAlarme == 0){
        Destravar_Alarme();
        isTemporizaTrava = true;
        millisInicioTemporizadorTrava = millis();
    }
    if (digitalRead(Trava_Alarme) == HIGH && ControleAlarme == 0){
        Travar_Alarme();
    }
    Presenca();
    Botao_Porta();
    PartidaCarro();
    Abre_Porta();
    Ausencia();
    if (isTemporizaTrava){
      travaTemporizada();
    }
}
//***********************************************************************************************

void Travar_Alarme(){
   if (digitalRead(Trava_Alarme) == HIGH){
       if(ControleTrava == 1){
           digitalWrite(Trava, HIGH);
           delay(300);
           digitalWrite(Trava, LOW);
           delay(50);
           digitalWrite(Buzzer, HIGH);
           delay(100);
           digitalWrite(Buzzer, LOW);
           delay(100);
           digitalWrite(Buzzer, HIGH);
           delay(100);
           digitalWrite(Buzzer, LOW);
           delay(500);
           ControleTrava = 0;
           ControlePorta = 0;
           ControleAlarme = 0;
       }
   }
}

//***********************************************************************************************

void Destravar_Alarme(){
   if (digitalRead(Destrava_Alarme) == HIGH){
       if(ControleTrava == 0){
           digitalWrite(Destrava, HIGH);
           delay(300);
           digitalWrite(Destrava, LOW);
           delay(50);
           digitalWrite(Buzzer, HIGH);
           delay(300);
           digitalWrite(Buzzer, LOW);
           delay(500);
           ControleTrava = 1;
           ControlePorta = 1;   //0
           ControleAlarme = 0;
       }
   }
}
//***********************************************************************************************

void Presenca(){
   if(digitalRead(ChavePresenca) == LOW ){
      Serial.println("AUSENTE...");
      Acesso=0;
   }else{
      Serial.println("PRESENTE...");
      Acesso=1;
   }
  // delay(500);
}
//**********************************************************************************************
void Botao_Porta(){
   if(digitalRead(BotaoPorta) == LOW){//Se for com botão de partida o valor será HIGH e tem que ativar função BOTPOR se for sensor o valor será LOW
     if(Acesso == 1 && digitalRead(PriEstPainel) == LOW && ControleTrava == 0){
           ControleTrava = 1;
           ControlePorta = 1;    //0
           ControleAlarme = 0;
           Destravar();
           isTemporizaTrava = true;
           millisInicioTemporizadorTrava = millis();
     }
   }
}



//**********************************************************************************************
void PartidaCarro(){
    if(Acesso == 1 && digitalRead(BotaoPartida) == HIGH && digitalRead(FreioEmb) == LOW && digitalRead(FuncMotor) == LOW){
        digitalWrite(PriEstPainel, !digitalRead(PriEstPainel));
        digitalWrite(SegEstPainel, LOW);
        ControleTrava = 1;
        ControlePorta = 1;
        delay(1000);
    }else{
        if(Acesso == 1 && digitalRead(BotaoPartida) == HIGH && digitalRead(FreioEmb) == HIGH && digitalRead(FuncMotor) == LOW){
            digitalWrite(PriEstPainel, HIGH);
            digitalWrite(SegEstPainel, HIGH);
            IgnicaoMotor();
            //delay(3000);
            if(digitalRead(SwithPorta) == HIGH) {
              Travar();
            }
        }else{
            if(Acesso == 1 && digitalRead(BotaoPartida) == HIGH && digitalRead(FreioEmb) == HIGH && digitalRead(FuncMotor) == HIGH){
                //digitalWrite(PriEstPainel, LOW);
                digitalWrite(SegEstPainel, LOW);
                delay(1500);
                 Destravar();
            }
        }
    }
    if(Acesso == 0 && digitalRead(FuncMotor) == HIGH && digitalRead(BotaoPartida) == HIGH){
        Serial.print("Ausência Chave...");
    }
}
//**********************************************************************************************
void Abre_Porta(){
  if(Acesso == 1 && digitalRead(SwithPorta) == LOW && digitalRead(FuncMotor) == LOW){
     digitalWrite(PriEstPainel, LOW);
     ControleAlarme = 1;
     ControlePorta = !ControlePorta;
     Serial.println(ControlePorta);
     desligaTemporizadorTrava();
  } else if(digitalRead(SwithPorta) == LOW && digitalRead(FuncMotor) == LOW){
    desligaTemporizadorTrava();
  }
}
//**********************************************************************************************
void Travar(){
     digitalWrite(Trava, HIGH);
     delay(200);
     digitalWrite(Trava, LOW);
     delay(50);
     digitalWrite(Buzzer, HIGH);
     delay(100);
     digitalWrite(Buzzer, LOW);
     delay(100);
     digitalWrite(Buzzer, HIGH);
     delay(100);
     digitalWrite(Buzzer, LOW);
     delay(500);

}
//**********************************************************************************************
void Destravar(){
     digitalWrite(Destrava, HIGH);
     delay(200);
     digitalWrite(Destrava, LOW);
     delay(50);
     digitalWrite(Buzzer, HIGH);
     delay(300);
     digitalWrite(Buzzer, LOW);
     delay(500);
}
//***********************************************************************************************
void Ausencia(){
   if(Acesso == 0 && ControleTrava == 1 && digitalRead(FuncMotor) == LOW && digitalRead(SwithPorta) == HIGH && ControleAlarme == 1){
       digitalWrite(PriEstPainel, LOW); 
       digitalWrite(SegEstPainel, LOW); 
       Travar();
       ControleAlarme = 0;
       ControleTrava = 0;
   }else{
      if(Acesso == 0 && digitalRead(FuncMotor) == LOW && digitalRead(SwithPorta) == LOW && ControlePorta == 1 && ControleAlarme == 1){
             ControlePorta = 0;
             Buzinar();
      }
   }
}
//***********************************************************************************************

//***********************************************************************************************
void IgnicaoMotor(){
  int cont = 1;
  while(cont < TempoPartida and digitalRead(FuncMotor) == LOW){
         digitalWrite(Partida, HIGH);
         cont++;
         delay(10);
         Serial.println(cont);
  }
  digitalWrite(Partida, LOW);
    //BotPar();
}
//***********************************************************************************************
void Buzinar(){
   digitalWrite(Buzina, HIGH);
   delay(50);
   digitalWrite(Buzina, LOW);
   delay(50);
   digitalWrite(Buzina, HIGH);
   delay(50);
   digitalWrite(Buzina, LOW);
}


/*
bool BotPor() {
   #define tempoDebounce 50 //(tempo para eliminar o efeito Bounce EM MILISEGUNDOS)

   bool estadoBotao;
   static bool estadoBotaoAnt; 
   static bool estadoRet = true;
   static unsigned long delayBotao = 0;

   if ( (millis() - delayBotao) > tempoDebounce ) {
       estadoBotao = digitalRead(BotaoPorta);
       if ( estadoBotao && (estadoBotao != estadoBotaoAnt) ) {
          estadoRet = !estadoRet;
          delayBotao = millis();
       }
       estadoBotaoAnt = estadoBotao;
   }

   return estadoRet;
}
*/
//***********************************************************************************************

bool BotPar() {
   #define tempoDebounce 50 //(tempo para eliminar o efeito Bounce EM MILISEGUNDOS)

   bool estadoBotao;
   static bool estadoBotaoAnt; 
   static bool estadoRet = true;
   static unsigned long delayBotao = 0;

   if ( (millis() - delayBotao) > tempoDebounce ) {
       estadoBotao = digitalRead(BotaoPartida);
       if ( estadoBotao && (estadoBotao != estadoBotaoAnt) ) {
          estadoRet = !estadoRet;
          delayBotao = millis();
       }
       estadoBotaoAnt = estadoBotao;
   }

   return estadoRet;
}

void travaTemporizada(){
  if(millis() - millisInicioTemporizadorTrava >= intervaloTrava){
    desligaTemporizadorTrava();
    Travar();
    ControleTrava = 0;
  }
}

void desligaTemporizadorTrava(){
  isTemporizaTrava = false;
  millisInicioTemporizadorTrava = 0;
}

// codigo de exemplo
