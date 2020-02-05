#include "FirebaseESP8266.h"
#define FREQUENCY    80                  // valid 80, 160

#include <pcf8574_esp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPI.h>
#include "RF24.h"
#include <PubSubClient.h>

#define FIREBASE_HOST "" //Do not include https:// in FIREBASE_HOST
#define FIREBASE_AUTH ""

#define COUNT 10 
//#define DATARATE RF24_2MBPS
//#define DATARATE RF24_1MBPS
#define DATARATE RF24_250KBPS
unsigned long buzz2 = 0;
const char* mqttServer = "farmer.cloudmqtt.com"; 
const int mqttPort = 16879;
const char* mqttUser = "vzykxtyc";
const char* mqttPassword = "KtLns1-gf2wH";


// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};


RF24 radio(D4,D2); //CE,CSN //(2,4)

boolean wakeUp = false;
boolean open_ = false;
boolean close_= false;
boolean changePin_ = false;
int _success = 0;
int _fail = 0;
unsigned long _startTime = 0;


struct lockDef
{
  byte status_;
  byte response_;
  byte ack_;
};



struct hubDef
{
  byte command_ ;
  byte response_ ;
  byte ack_;
  
};

hubDef hubPak;
lockDef lockPak;

   unsigned long buzzTime = 0;
boolean buzz = false;
String devID =  "devID5";
char * ssid_ap = "Lunar";
char * password_ap = "lunar1234";
String freezeState = "";
int server_setup = 0;
int server_run = 0;
int currtouched = 0;
int k = 0;
int holdTime  = 0;
String command = "";
uint32_t rut = 0;
///////////////////pinDef//////////////////////////
#define reset_ D3

//////////////////////////////////////////////////
IPAddress ip(192,168,11,4); // arbitrary IP address (doesn't conflict w/ local network)
IPAddress gateway(192,168,11,1);
IPAddress subnet(255,255,255,0);


ESP8266WebServer server;
FirebaseData firebaseData;
WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
Serial.begin(115200);
pinMode(D1,OUTPUT);
pinMode(D8,OUTPUT);
digitalWrite(D8,HIGH);

  radio.begin();
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1, pipes[0]);
  radio.setDataRate( DATARATE );
   radio.setRetries(1, 15);                // Smallest time between retries, max no. of retries

    radio.setAutoAck(true) ;

 radio.setPALevel( RF24_PA_MAX ) ;
  radio.setChannel(15);
  //radio.enableDynamicPayloads() ;
 radio.enableAckPayload();                  // not used here


  //radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  radio.powerUp();
  radio.startListening();

delay(100);

  WiFi.softAPdisconnect(true);
delay(100);

  WiFi.disconnect(true);
  delay(100);

  SPIFFS.begin();
 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  server.on("/update",HTTP_POST, handleSettingsUpdate);
  server.on("/devId336567",HTTP_POST, devId);
  server.begin();


  
   

   wifiConnect();

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
   if (client.connect("ESP8266Client" ,mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } 
   else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
  client.publish("message", "shounak Bhadwa to hai hi");
    client.publish("message", "bhosdiwala bhi hai");

  client.subscribe("devID5/command",1);
 
  digitalWrite(D8,LOW);

  }




  
  void wifiConnect(){
WiFi.softAPdisconnect(true);
  WiFi.disconnect();          
  delay(100);
  //check for stored credentials
  if(SPIFFS.exists("/config.json")){
    const char * _ssid = "", *_pass = "";
    File configFile = SPIFFS.open("/config.json", "r");
    if(configFile){
      size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      configFile.close();

      DynamicJsonBuffer jsonBuffer;
      JsonObject& jObject = jsonBuffer.parseObject(buf.get());
      if(jObject.success())
      {
       _ssid = jObject["ssid"];
       _pass = jObject["password"];
      Serial.print("SSID = ");
      Serial.println(_ssid);
      Serial.print("PASSWORD = ");
      Serial.println(_pass);
        
        WiFi.mode(WIFI_STA);
        WiFi.hostname("lunar_V1");
        WiFi.begin(_ssid, _pass);
        unsigned long startTime = millis();
         while (WiFi.status() != WL_CONNECTED) 
        {
     

       Serial.print(".");
       delay(500);
      

     if((millis()-startTime) > 15000) break;     

        }
             Serial.println("");

      Serial.print("ConnectionTime  = ");
      Serial.println(millis()-startTime);
      
      }
    }
  }

  
  Serial.println("");
  if(WiFi.status() == WL_CONNECTED){
        Serial.print("IP address : ");
        Serial.println(WiFi.localIP());
  
    }
    else{
      
      while(1){}
      }
  
    }

void handleSettingsUpdate()
  {
  String data = server.arg("plain");
  DynamicJsonBuffer jBuffer;
  
  JsonObject& jObject = jBuffer.parseObject(data);
  Serial.println(data);
  File configFile = SPIFFS.open("/config.json", "w");
  jObject.printTo(configFile);  
  configFile.close();
  delay(500);
  server_run = 0;
  wifiConnect();
 
}

void devId(){
  
  //server.send(200, "application/json", "{\"devId\" : \"ok\",\"Model\" : \"8ltr\"}");

 }
void server_(){
while(server_setup == 0){
    Serial.println("Initiating Server");
    WiFi.disconnect();
     WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip, gateway, subnet);
    WiFi.softAP(ssid_ap, password_ap); 
    server_setup = 1;
    radio.startListening();

  }
}

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  
char receiveBuf[10]{};  
   for (int i = 0; i < length; i++) {
    receiveBuf[i]= payload[i];
   }
String stringBuf = "";
int j = 0;
while(j<10){
  
  if(!receiveBuf[j])break;
  stringBuf += receiveBuf[j];
  
  
  j++;
  
  
  }
Serial.print("Data made = ");
Serial.print(stringBuf);
Serial.println();
   

   
if(stringBuf == "ready"){ 
   Serial.println("READY");
   wakeUp = true;  
   
}

else if(stringBuf == "open"){
     Serial.println("OPEN");
      open_ = true;
  
   }

 else if(stringBuf == "close"){
    
     Serial.println("CLOSE");
     close_ = true;
   }
   
else if(stringBuf == "reset")
  {
    Serial.println("RESET");
    changePin_ = true;
    
    } 
  Serial.println();
  Serial.println("-----------------------");
 
}

void loop(){
if(radio.available())
   {
            //Firebase.setString(firebaseData,"Locks/"+devID+"/status","unlocked");

    radio.read(&lockPak,sizeof(lockPak));
    Serial.print("Received Status = ");
    Serial.println(lockPak.status_);
    ping_();

    if(lockPak.status_ == 111){
   buzz2 = millis();
   buzzTime = millis();
   buzz = true;
   analogWriteFreq(1600);
    analogWrite(D1,500);
    tone(D8,5,1000);
    
    }
    


}

  if(buzz){

    
    Serial.println("Turning on buzzer for 5 seconds");
    
    int time_ = (millis()-buzz2);
   if(time_ < 400 && time_ > 100){
     analogWriteFreq(1000);
     analogWrite(D1,500);
      
    }
   else if((millis()-buzz2) >=200){
         
          analogWriteFreq(1600);
          analogWrite(D1,500);
         buzz2 = millis();

    }
   
  
    
    }

    
    if((millis()-buzzTime)>= 5000){
      buzz = false;
      digitalWrite(D1,LOW);
      digitalWrite(D8,LOW);
      buzzTime = 0;

      }

}




void firebaseOperation(){
radio.flush_rx();

if( Firebase.getString(firebaseData,"Locks/"+devID+"/command")){

  
    }  
    
  
  
  
  }


void button(){

int i = 0;
while(digitalRead(reset_)==HIGH){
  delay(10);
  i++;
  Serial.println(i);
if(i>500){
  Serial.println("Initiating Server");
  i = 0;
  server_run = 1;
  server_();
      }
Serial.println("something1");
   } 

}

void writeData(){
  radio.stopListening();
  radio.flush_rx();
  radio.write(&hubPak,sizeof(hubPak));
  radio.startListening();
  
  }



  void resetData(){
   hubPak.command_ = 0;
   hubPak.response_ =0;
   hubPak.ack_ = 0;
   lockPak.status_ = 0;
   lockPak.response_ = 0;
   hubPak.ack_ = 0; 
   }

void ping_(){
  radio.stopListening();

  
   hubPak.command_ = 133;

   if(wakeUp == true){
  hubPak.command_ = 115;

    
    }
   hubPak.response_ = 133;
   hubPak.ack_ = 111;
   radio.flush_rx();
   radio.flush_tx();

if(radio.write(&hubPak,sizeof(hubPak))){
  Serial.println("Ping Sent ");
 }
 
 else{
  Serial.println("Ping Failed");
  
  }
  radio.startListening();
}
