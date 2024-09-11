//=====================================================
//Autor: Arthur Aurelio 
//Contato: arthuralmeida@alunos.utfpr.edu.br
//=====================================================

//Impotação de bibliotecas
#include <OneWire.h>
#include <DallasTemperature.h>
#include <CO2Sensor.h>
#include <dht.h>
#include <EEPROM.h>
#include <GravityTDS.h>
#include <RTClib.h>
#include <WiFiEsp.h>
#include <SoftwareSerial.h>

//Definição de váriaveis para os pinos do relé
const int luz1 = 8;
const int refri = 9;
const int luz2 = 10;
const int vent = 11;

//Definição de variáveis para os pinos do DS18B20
const int dsene = 7;
const int ds18 = 2;

//Definição de variáveis para os pinos do sensor TDS
const int tdsene = 6;
const int TdsSensorPin = A0;

//Definição de variáveis para os pinos do MG811
const int mg811 = A1;

//Definição de variáveis para os pinos do DHT22
const int dhtPin = A7;

//Definições de variáveis para o bluetooth
SoftwareSerial ArduinoMaster(13, 12);

//Definições de variáveis para o PH
const int phene = 5;
const int phPin = A3;

//Definições de variáveis da chave bluetooth
const int chave = 3;
const int led = 4;
  int botton = 0;

//Configuração e variáveis para sensor DS18B20
OneWire ourWire(ds18);
DallasTemperature sensors(&ourWire);

//Configuração e variáveis para sensor TDS
GravityTDS gravityTds;

//Configurações e variáveis para sensor MG811
CO2Sensor co2Sensor(mg811, 0.99, 100);

//Configuração e variáveis para sensor DHT22
dht DHT;

//Definições de variáveis para o RTC
RTC_DS3231 rtc;

//Definições de variáveis para o ESP-01
//SoftwareSerial ESP(4, 5); //TROCAR RX POR PINO DO RX E TX POR PINO DO TX
//char ssid[] = "REDE_WIFI"; //NOME DO WIFI
//char pass[] = "SENHA_WIFI"; //SENHA DO WIFI
//int status = WL_IDLE_STATUS;
//WiFiEspServer server(80);
//RingBuffer buf(8);

//Variaveis globais
int ligarVent = 0, esquentar = 0;
int hora = 0, estado = 1;

void setup() {
  //Inicio da porta Serial
  Serial.begin(19200);

  //Inicia a porta serial do bluetooth
  ArduinoMaster.begin(9600);

  //Configuração do tipo da chave
  pinMode(chave, INPUT_PULLUP);
  pinMode(led, OUTPUT);

  //Configuração do tipo dos pinos do relé
  pinMode(luz1, OUTPUT);
  pinMode(refri, OUTPUT);
  pinMode(luz2, OUTPUT);
  pinMode(vent, OUTPUT);

  //Liga todas as portas do relé
  digitalWrite(luz1, LOW);
  digitalWrite(refri, LOW);
  digitalWrite(luz2, LOW);
  digitalWrite(vent, LOW);

  //Configuração do tipo dos pinos do DS18B20
  pinMode(dsene, OUTPUT);
  pinMode(ds18, INPUT);

  //Configuração do tipo dos pinos do TDS
  pinMode(tdsene, OUTPUT);
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);
  gravityTds.setAdcRange(1024);
  gravityTds.begin();

  //Configuração do tipo dos pinos do MG811
  pinMode(mg811, INPUT);
  co2Sensor.calibrate();

  //Configuração do tipo dos pinos do DHT22
  pinMode(dhtPin, INPUT);

  //Inicio dos sensores
  sensors.begin();  

  //Inicio do PH
  pinMode(phPin, INPUT);
  pinMode(phene, OUTPUT);

  //Inicio do RTC
  if(! rtc.begin()) {
    while(1);
  }
  if(rtc.lostPower()){
    //AJUSTE RTC
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    //rtc.adjust(DateTime(2023, 10, 04, 17, 10, 45)); //(ANO), (MÊS), (DIA), (HORA), (MINUTOS), (SEGUNDOS)
  }

  //Mantem sensores D18B20, PH e TDS desligados até serem necessarios
  digitalWrite(phene, LOW);
  digitalWrite(dsene, LOW);
  digitalWrite(tdsene, LOW);

  DateTime now = rtc.now();
}

//Função que liga o sensor de temperatura DS18B20 e retorna o valor da leitura em graus Celcius
float DS18B20Sensor() {
  digitalWrite(phene, LOW);
  digitalWrite(tdsene, LOW);
  digitalWrite(dsene, HIGH);
  delay(300);
  sensors.requestTemperatures();
  return (sensors.getTempCByIndex(0));
}

//Função que liga o sensor de temperatura TDS v1.0 e retorna o valor da leitura em ppm
float TDSSensor(float temp) {
  float tdsValue = 0;
  digitalWrite(phene, LOW);
  digitalWrite(dsene, LOW);
  digitalWrite(tdsene, HIGH);
  delay(300);
  gravityTds.setTemperature(temp);
  gravityTds.update();
  tdsValue = gravityTds.getTdsValue();
  digitalWrite(tdsene, LOW);
  return (tdsValue);
}

//Função que liga o sensor de PH e retorna o valor da leitura
float PHSensor() {
  float phLeit;
  float phValue;
  digitalWrite(dsene, LOW);
  digitalWrite(tdsene, LOW);
  digitalWrite(phene, HIGH);
  delay(500);
  phLeit = analogRead(phPin);
  phValue = (phLeit * 5.0) / 1024.0;
  digitalWrite(phene, LOW);
  return(phValue);
}

//Função que liga o sensor de CO2 MG811 e retorna o valor da leitura ppm
float MG811Sensor() {
  return (co2Sensor.read());
}

//Função que liga o sensor de umidade DHT22 e retorna o valor da leitura em %
float DHTUmiSensor() {
  DHT.read11(dhtPin);
  return (DHT.humidity);
}

//Função que liga o sensor de temperatura DHT e retorna o valor da leitura em graus Celcius
float DHTTempSensor() {
  DHT.read11(dhtPin);
  return (DHT.temperature);
}

//Loop Principal (função main)
void loop(){
  char x;
  char msg;

  if (botton == 0){
    digitalWrite(led, LOW);

    //if (x != 'a')
      //manual(x);
    //else
      autonomo();

    if(digitalRead(chave) == LOW){
      botton = 1;
      digitalWrite(led, HIGH);
      delay(3000);
    }
  }

  else{
    digitalWrite(led, HIGH);
    msg = bt();
    leituraBT(msg);

    if(digitalRead(chave) == LOW){
      botton = 0;
      digitalWrite(led, LOW);
    }
  }
}

float bt(){
 	while (ArduinoMaster.available()) {
 			delay(10);
 			if (ArduinoMaster.available() > 0) {
 					char c = ArduinoMaster.read();
 					return c;
 			}
 	}
 	ArduinoMaster.flush();
}

float leituraBT(char msg){
 if (msg == 'L'){
      digitalWrite(luz1, LOW);
      digitalWrite(refri, LOW);
  }
  else if (msg == 'N'){
      digitalWrite(luz1, HIGH);
      digitalWrite(refri, HIGH);
  }
  else if (msg == 'V'){
      digitalWrite(vent, LOW);
  }
  else if (msg == 'X'){
      digitalWrite(vent, HIGH);
  }
  else if (msg == 'M'){
      digitalWrite(luz2, LOW);
  }
  else if (msg == 'T'){
      digitalWrite(luz2, HIGH);
  }
}

//Função para sistema autônomo
void autonomo(){
	float temperature, nivtds, nivco2, nivumi, temperar, nivph;
	
  temperature = DS18B20Sensor();  	//Medir temperatura da agua
  nivtds = TDSSensor(temperature);  //Medir nivel do TDS
  nivtds = nivtds * 2 / 1000;       //converter de ppm para ec
  nivph = PHSensor(); 				      //Medir nivel do PH
  nivco2 = MG811Sensor();  			    //Medir nivel do CO2
  nivumi = DHTUmiSensor();  			  //Medir nivel de umidade
  temperar = DHTTempSensor();  		  //Medir temperatura do ar

  delay(1000);
	
    ligarVent = 0;
    esquentar = 0;

    int statusTempAgua, statusTDS, statusCO2, statusUmid, statusTempAr;

    //Verifica qualidade da temperatura da agua
    statusTempAgua = veriAgua(temperature);
    //Verifica qualidade do nivel de TDS
    statusTDS = veriTDS(nivtds);
    //Verifica qualidade do nivel de CO2
    statusCO2 = veriCO2(nivco2);
    //Verifica qualidade do nivel de umidade
    statusUmid = veriUmi(nivumi);
    //Verifica qualidade da temperatura do ar
    statusTempAr = veriAr(temperar);

    int statusCooler, statusLuz;

    //Verifica se será necessario ligar a ventilação
    statusCooler = ligarCooler();
    //Verifica se será necessario ligar ou desligar a iluminação
    statusLuz = iluminacao();
    //Verifica se será necessario aquecer
    aquece();
	
	print(temperature, nivco2, nivumi, temperar, nivtds, nivph, statusTempAgua, statusTDS, statusCO2, statusUmid, statusTempAr, statusCooler, statusLuz);
}

//Função para sistema manual
void manual(char x){
	float temperature, nivtds, nivco2, nivumi, temperar, nivph;

  temperature = DS18B20Sensor();  	//Medir temperatura da agua
  nivtds = TDSSensor(temperature);  	//Medir nivel do TDS
  nivtds = nivtds * 2 / 1000;     	//converter de ppm para ec
  nivph = PHSensor(); 				//Medir nivel do PH
  nivco2 = MG811Sensor();  			//Medir nivel do CO2
  nivumi = DHTUmiSensor();  			//Medir nivel de umidade
  temperar = DHTTempSensor();  		//Medir temperatura do ar

  int statusTempAgua, statusTDS, statusCO2, statusUmid, statusTempAr;
  //Verifica qualidade da temperatura da agua
  statusTempAgua = veriAgua(temperature);
  //Verifica qualidade do nivel de TDS
  statusTDS = veriTDS(nivtds);
  //Verifica qualidade do nivel de CO2
  statusCO2 = veriCO2(nivco2);
  //Verifica qualidade do nivel de umidade
  statusUmid = veriUmi(nivumi);
  //Verifica qualidade da temperatura do ar
  statusTempAr = veriAr(temperar);
  
  int statusCooler, statusLuz;

  if(x == 'l'){
    digitalWrite(luz1, LOW);
    digitalWrite(refri, LOW);
    digitalWrite(luz2, LOW);
    statusLuz = 1;
  }
  else if(x == 'd'){
    digitalWrite(luz1, HIGH);
    digitalWrite(refri, HIGH);
    digitalWrite(luz2, HIGH);
    statusLuz = 0;
  }
  else if(x == 'v'){
    digitalWrite(vent, LOW);
    statusCooler = 1;
  }
  else if(x == 'w'){
    digitalWrite(vent, HIGH);
    statusCooler = 0;
  }

  print(temperature, nivco2, nivumi, temperar, nivtds, nivph, statusTempAgua, statusTDS, statusCO2, statusUmid, statusTempAr, statusCooler, statusLuz);
}

//Verificação da temperatura da Agua
int veriAgua(float temp) {
  int maxAgua = 28, minAgua = 18;
  if (temp >= maxAgua) {
    return 1;
  } 
  else if (temp <= minAgua) {
      return -1;
  } 
  else {
      return 0;
  }
}

//Verificação do nivel de TDS
int veriTDS(float tds) {
  int maxTDS = 2.1, minTDS = 1.2;
  if (tds >= maxTDS) {
    return 1;
  } 
  else if (tds <= minTDS) {
      return -1;
  } 
  else {
      return 0;
  }
}

//Verificação do nivel de CO2
int veriCO2(float co2) {
  int maxCO2 = 1000, minCO2 = 375;
  if (co2 >= maxCO2) {
    return 1;
  } 
  else if (co2 <= minCO2) {
      ligarVent = ligarVent + 1;
      return -1;
  } 
  else {
      return 0;
  }
}

//Verificação do nivel de Umidade
int veriUmi(float umi) {
  int maxUmi = 80, minUmi = 45;
  if (umi >= maxUmi) {
    ligarVent = ligarVent + 1;
    return 1;
  } 
  else if (umi <= minUmi) {
      return -1;
  } 
  else {
     return 0;
  }
}

//Verificação da temperatura do ar
int veriAr(float temp) {
  int maxAr = 26, minAr = 22;
  if (temp >= maxAr) {
    ligarVent = ligarVent + 1;
    return 1;
  } 
  else if (temp <= minAr) {
      esquentar = esquentar + 1;
      return -1;
  } 
  else {
      return 0;
  }
}

int iluminacao() {

  DateTime now = rtc.now();

  int hora = now.hour();
  int min = now.minute();

  if (hora < 6 && hora >0) {
    digitalWrite(luz1, HIGH);
    digitalWrite(refri, HIGH);
    digitalWrite(luz2, HIGH);
    return -1;

  }
  else if (hora == 12 && min <= 10){
      digitalWrite(luz1, HIGH);
      digitalWrite(refri, HIGH);
      digitalWrite(luz2, LOW);
      return 0;
  }
  else if (hora == 18 && min <= 10){
      digitalWrite(luz1, HIGH);
      digitalWrite(refri, HIGH);
      digitalWrite(luz2, LOW);
      return 0;
  }
  else if (hora == 6 && min <= 10){
      digitalWrite(luz1, HIGH);
      digitalWrite(refri, HIGH);
      digitalWrite(luz2, LOW);
      return 0;
  }
  else{
      digitalWrite(luz1, LOW);
      digitalWrite(refri, LOW);
      digitalWrite(luz2, LOW);
      return 1;
  }
}

//Função que liga os coolers da PFC e permite ventilação e resfriamento da caixa
int ligarCooler() {
  if (ligarVent > 0) {
    digitalWrite(vent, LOW);
    return 1;
  } else {
    digitalWrite(vent, HIGH);
    return 0;
  }
}

//Função que desliga resfriação da led para aquecer ambiente
void aquece() {
  if (esquentar > 0) {
    digitalWrite(refri, HIGH);
  } else {
    digitalWrite(refri, LOW);
  }
}

//Função que faz o print das informações para a saída serial
char print(float tempA, float co2, float umi, float tempB, float tds, float ph, int sTempA, int sTDS, int sCO2, int sUmi, int sTempB, int sCooler, int sLuz){
  DateTime now = rtc.now();

  int hora = now.hour();
  int min = now.minute();

  Serial.println(hora);
  Serial.println(min);
  Serial.println(tempA);
  Serial.println(co2);
  Serial.println(umi);
  Serial.println(tempB);
  Serial.println(tds);
  Serial.println(ph);
  Serial.println(sTempA);
  Serial.println(sTDS);
  Serial.println(sCO2);
  Serial.println(sUmi);
  Serial.println(sTempB);
  Serial.println(sCooler);
  Serial.println(sLuz);
}