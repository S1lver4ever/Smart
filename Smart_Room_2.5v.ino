//Bibliotecas
#include <NTPClient.h> //Biblioteca para pegar o horario da internet
#include <WiFiUdp.h> //Biblioteca complementar para pegar o horario da internet
#include <DHT.h> //Biblioteca do sensor de temperatura e umidade
#include <IRremoteESP8266.h> //Biblioteca para controle do infravermelho
#include <IRremoteInt.h> //Biblioteca complementar para controle do infravermelho

//Escolha uma placa e comente a outra
//#define ESP32
#define ESP8266

#ifdef ESP32
  #include <WiFi.h>
  #include <FirebaseESP32.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <FirebaseESP8266.h>
#endif

//Macros
#define WIFI_SSID "UPE_2.4"
#define WIFI_PASS "5m@rtup3"
#define FIREBASE_HOST "https://smartroom-629d6-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "aCuOdd9CX5Z0gWADeRhlsX6v48w7VSv0FRkzmmlv"
#define TIME_ZONE -3 //Fuso Horario
#define SERVER_NTP "0.br.pool.ntp.org" //Site de onde pega o horario
#define THING_NAME "Thing030"
#define PATH "/data"

//Pinos 
#define DHT_PIN 5
#define IRSEND_PIN 4
#define LAMPADA_UM 12
#define LAMPADA_DOIS 0
#define LED 2 //Led que pisca, mostrando o status do Wi-Fi

//Objetos
WiFiUDP ntpUDP; //Objeto da biblioteca de tempo
NTPClient timeClient(ntpUDP, SERVER_NTP, TIME_ZONE * 3600, 60000); //Objeto da biblioteca de tempo
DHT dht(DHT_PIN, DHT11); //Objeto do sendor de temperatura e umidade
IRsend irsend(IRSEND_PIN); //Objeto que envia o comando IR
FirebaseData firebaseData; // Objeto de dados do Firebase

//Comandos Ar-condicionado
unsigned int ligaPwr[199] = {4384, 4328, 480, 1680, 584, 528, 532, 1608, 508, 1680, 536, 548, 532, 528, 560, 1628, 532, 556, 532, 552, 536, 1632, 480, 604, 480, 580, 560, 1628, 532, 1628, 536, 528, 504, 1688, 532, 528, 560, 1604, 560, 1604, 584, 1576, 560, 1604, 556, 528, 584, 1604, 588, 1552, 576, 1592, 492, 620, 536, 524, 508, 600, 560, 524, 564, 1600, 480, 584, 584, 524, 536, 1608, 560, 1628, 532, 1628, 484, 604, 532, 552, 536, 524, 556, 532, 556, 556, 532, 528, 560, 552, 532, 552, 560, 1604, 536, 1628, 532, 1632, 480, 1680, 536, 1628, 456, 5260, 4384, 4328, 560, 1628, 536, 548, 532, 1612, 556, 1604, 560, 528, 556, 552, 560, 1604, 532, 528, 588, 528, 532, 1608, 504, 604, 484, 576, 560, 1628, 564, 1576, 556, 552, 480, 1664, 584, 528, 480, 1684, 532, 1628, 536, 1628, 560, 1600, 536, 552, 560, 1580, 560, 1604, 556, 1612, 560, 552, 528, 556, 536, 548, 536, 524, 564, 1624, 536, 552, 532, 552, 564, 1604, 460, 1680, 560, 1604, 556, 528, 480, 628, 560, 528, 532, 552, 536, 528, 560, 552, 532, 552, 480, 604, 536, 1632, 536, 1628, 480, 1680, 560, 1604, 560, 1580, 480};
unsigned int desligaPwr[199] = {4388, 4332, 560, 1604, 584, 524, 568, 1600, 536, 1604, 556, 548, 540, 548, 536, 1624, 484, 604, 536, 552, 536, 1604, 564, 548, 564, 496, 588, 1600, 560, 1600, 540, 548, 560, 1584, 588, 1576, 564, 548, 564, 1580, 584, 1600, 564, 1572, 564, 1624, 484, 1656, 560, 1604, 560, 528, 504, 1660, 560, 524, 560, 524, 564, 548, 536, 524, 588, 520, 536, 528, 564, 548, 564, 496, 560, 1608, 556, 528, 560, 548, 536, 548, 536, 528, 560, 524, 588, 1604, 536, 1600, 508, 604, 528, 1636, 536, 1628, 560, 1576, 508, 1680, 536, 1600, 484, 5260, 4412, 4300, 560, 1628, 536, 548, 536, 1604, 588, 1600, 540, 520, 564, 548, 532, 1628, 484, 604, 564, 524, 592, 1552, 560, 548, 564, 520, 540, 1600, 588, 1576, 560, 548, 536, 1608, 588, 1604, 540, 544, 564, 1580, 560, 1604, 588, 1572, 588, 1576, 560, 1624, 568, 1572, 564, 552, 560, 1576, 588, 524, 588, 496, 480, 604, 536, 548, 540, 548, 560, 528, 536, 548, 460, 604, 560, 1628, 564, 520, 564, 496, 588, 500, 560, 548, 540, 548, 536, 1604, 564, 1600, 560, 548, 564, 1604, 480, 1656, 588, 1576, 560, 1604, 560, 1604, 480};

//Variaveis Globais
String timestamp;
float value[2] = {0, 0};
int estado = 0;
int novoEstado = 0;
int lampadaUm = 0;
int lampadaDois = 0;

// inicia variáveis para controle de tempo
unsigned long oldMillisLed = 0;
unsigned long oldMillisTime = 0;
unsigned long oldMillisTest = 0;

void setup() {
  
  Serial.begin(9600);
  dht.begin();
  irsend.begin();
  
  pinMode(LED, OUTPUT);
  pinMode(LAMPADA_UM, OUTPUT);
  pinMode(LAMPADA_DOIS, OUTPUT);
  
  //Inicia Wifi
  configWiFi();
  
  //Inicia relogio on-line
  timeClient.begin();
  timeClient.forceUpdate();
  
  //Inicia Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true); //Habilita o auto reconexão do WiFi quando perdido
  
}

void loop() {
  
  statusWiFi();
  atuador();
  timestamp = (String)timeClient.getEpochTime();
  
  //Envia dados a cada 1 minuto
  if ((millis() - oldMillisTest) > 60000) {
    oldMillisTest = millis();
    sendDataTest();
  }
  //Envia dados a cada 5 minutos
  if ((millis() - oldMillisTime) > 300000){
    oldMillisTime = millis();
    sendDataFirebase();
  }
  debug();
  
}

void configWiFi(){

  //Conectando Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Thing macAddress :");
  Serial.println(WiFi.macAddress());
  Serial.print("/AP macAddress :");
  Serial.println(WiFi.softAPmacAddress());
  
}

void statusWiFi(){

  // pisca um led pra a gente ter um retorno visual que a placa ta ligada
  if (WiFi.status() == WL_CONNECTED) {
    if ((millis() - oldMillisLed) > 1000) {
      digitalWrite(LED, !digitalRead(LED));
//      Serial.println("c");
      oldMillisLed = millis();
    }
  } else {
    if ((millis() - oldMillisLed) > 250) {
      digitalWrite(LED, !digitalRead(LED));
//      Serial.println("o");
      oldMillisLed = millis();
    }
  }
  
}

void sendDataFirebase(){

  FirebaseJson obj;
  value[0] = dht.readTemperature();
  value[1] = dht.readHumidity();    

  for(int i = 0; i < 2; i++){
    obj.set("thingName", THING_NAME);
    obj.set("sensorId", i + 3);
    obj.set("timestamp", timestamp);
    obj.set("value", value[i]);
    Firebase.set(firebaseData, PATH, obj);
  }
}

void sendDataTest(){

  FirebaseJson obj;
  value[0] = dht.readTemperature();
  value[1] = dht.readHumidity();    

  for(int i = 0; i < 2; i++){
    obj.set("thingName", THING_NAME);
    obj.set("sensorId", i + 3);
    obj.set("timestamp", timestamp);
    obj.set("value", value[i]);
    Firebase.set(firebaseData, PATH, obj);
  }
}

void atuador(){

  Firebase.get(firebaseData, "/atuador/Thing030/lampadaUm");
  lampadaUm = firebaseData.intData();
  Firebase.get(firebaseData, "/atuador/Thing030/lampadaDois");
  lampadaDois = firebaseData.intData();
  Firebase.get(firebaseData, "/atuador/Thing030/arCondicionado");
  novoEstado = firebaseData.intData();

  if(estado != novoEstado){
    if(novoEstado == 1){
      irsend.sendRaw(ligaPwr, 199, 38);
    }else{
      irsend.sendRaw(desligaPwr, 199, 38);
    }
  }
  estado = novoEstado;
  
  digitalWrite(LAMPADA_UM, lampadaUm);
  digitalWrite(LAMPADA_DOIS, lampadaDois);
  
}

void debug(){

  Serial.print("\n--------------------------------------------------------------");
  Serial.print("\nTemperatura: ");
  Serial.print(value[0]);
  Serial.print("\nUmidade: ");
  Serial.print(value[1]);
  Serial.print("\nLampada 1: ");
  Serial.print(lampadaUm);
  Serial.print("\nLampada 2: ");
  Serial.print(lampadaDois);
  Serial.print("\nEstado: ");
  Serial.print(estado);
  Serial.print("\nNovo estado: ");
  Serial.print(novoEstado);
  Serial.print("\nTimestamp: ");
  Serial.print(timestamp);
  
}
