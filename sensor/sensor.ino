/*
 *
 * NRF24L01_Bi-Directional_Communication
 *
 * Testbed for NRF24L01 boards
 *
 * Bi-directional traffic
 *
 * This sketch uses the same software for both nodes
 *
 * written by Andreas Spiess. Based on Example Sketch of RF24 Library

 */
#include<avr/wdt.h> /* Header for watchdog timers in AVR */

#include <SPI.h>
#include <RF24.h>
#include <printf.h>
#include <Wire.h>
#include<EEPROM.h>


boolean sent  = false;
int currtouched = 0;
int a,b,c,d,e,f,m = 0;
int k = 0;
uint16_t holdTime  = 0;

#define COUNT 10 // nomber for statistics

//#define DATARATE RF24_2MBPS
//#define DATARATE RF24_1MBPS
 #define DATARATE RF24_250KBPS

unsigned long startTime  = millis();


// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};



RF24 radio(7, 8);


int _success = 0;
int _fail = 0;
unsigned long _startTime = 0;


struct lockDef
{
  byte status_ ;
  byte response_ ;
  byte ack_ ;
};



 struct hubDef
 {
  byte command_ ;
  byte response_ ;
  byte ack_;
 
};
hubDef hubPak;
lockDef lockPak;

int sensor  = 4;

void setup(void)
{
 
//  WDTCSR = (24);//change enable and WDE - also reset
// WDTCSR = (33);//prescalers only - get rid of the WDE and WDCE bit
// WDTCSR |= (1<<6);//enable interrupt mode
// WDTCSR |= (1<<3);//enable interrupt mode
 
}


void loop(void){

Serial.begin(115200);
 pinMode(3,INPUT);
 radio.begin();
 radio.openWritingPipe(pipes[0]);
 radio.openReadingPipe(1, pipes[1]);
 radio.setRetries(1, 15);                // Smallest time between retries, max no. of retries
 radio.setAutoAck(true) ;
 radio.enableAckPayload();// not used here
 radio.setDataRate( DATARATE );
 radio.setPALevel( RF24_PA_LOW );
 radio.setChannel(15);
 radio.enableDynamicPayloads();
 radio.powerUp();
 radio.stopListening();
 ADCSRA &= ~(1 << 7);
 Serial.println("Sending");

if(radio.write(&lockPak,sizeof(lockPak))){
   Serial.println("Sent");
    radio.startListening();
    while(!radio.available()){
      delay(2);
      if((millis()-startTime) >= 10){
        break;
        }
     }
     
    if(radio.available()){
      radio.read(&hubPak,sizeof(lockPak));
      }
      Serial.print("Reaceived commend  = ");
      Serial.println(hubPak.command_);
  
  }


Serial.println("Awake");
ping_();
  
sleeep();
  


}
    
    
    




void ping_(){

 if(digitalRead(3) == HIGH ){
    lockPak.status_ = 111;
  
  }
  else if(digitalRead(3) == LOW){
     lockPak.status_ = 222;
   }
  lockPak.response_ = 133;
  lockPak.ack_ = 222;
}




void writeData(){
  
     radio.stopListening();
     radio.write(&lockPak,sizeof(lockPak));
     radio.flush_rx();
     radio.startListening();
     
  }

void resetData(){
   hubPak.command_ = 0;
   hubPak.response_ =0;
   hubPak.ack_ = 0;
   lockPak.status_ = 0;
   lockPak.response_ = 0;
   lockPak.ack_ = 0;
    }




    void digitalInterrupt(){
      
//      pinMode(10,OUTPUT);
//for(int i = 0;i<=10;i++){
//  digitalWrite(10,HIGH);
//  delay(250);
//  digitalWrite(10,LOW);
//  delay(250);
//  }
}



void sleeep(){
 

    for(int i = 0;i<=6;i++){
      
      pinMode(i,OUTPUT);
      digitalWrite(i,LOW);
      }
 attachInterrupt(1, digitalInterrupt, FALLING); //interrupt for waking up
   radio.powerDown();
   Serial.print("millis = ");
   Serial.println(millis());

Serial.end();
 WDTCSR = (24);//change enable and WDE - also reset
 WDTCSR = (32);//prescalers only - get rid of the WDE and WDCE bit
 WDTCSR |= (1<<6);//enable interrupt mode
 WDTCSR |= (1<<3);//enable interrupt mode
  //Disable ADC - don't forget to flip back after waking up if using ADC in your application ADCSRA |= (1 << 7);
 ADCSRA &= ~(1 << 7);
  
  //ENABLE sleeep - this enables the  mode
 SMCR |= (1 << 2); //power down mode
 SMCR |= 1;//enable sleep_
    
  MCUCR |= (3 << 5); //set both BODS and BODSE at the same time
  MCUCR = (MCUCR & ~(1 << 5)) | (1 << 6); //then set the BODS bit and clear the BODSE bit at the same time
  __asm__  __volatile__("sleep");//in l
  }
//ISR(WDT_vect){
//  //DON'T FORGET THIS!  Needed for the watch dog timer.  This is called after a watch dog timer timeout - this is the interrupt function called after waking up
//}// watchdog interrupt
  
