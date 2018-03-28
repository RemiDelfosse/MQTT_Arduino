#include <SPI.h>

#include <RFM69_OTA.h>
#include <RFM69_ATC.h>
#include <RFM69.h>
#include <RFM69registers.h> 

#define trigPin 15
#define echoPin 16
long  duration, distance; 


//****************************************************************************************************************
//**** IMPORTANT RADIO SETTINGS - YOU MUST CHANGE/CONFIGURE TO MATCH YOUR HARDWARE TRANSCEIVER CONFIGURATION! ****
//****************************************************************************************************************
#define NODEID         3    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID     1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
#define FREQUENCY     RF69_868MHZ
//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//*****************************************************************************************************************************
#define ENABLE_ATC      //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI        -75
//*****************************************************************************************************************************
#define ACK_TIME      30 // max # of ms to wait for an ack

#ifdef __AVR_ATmega1284P__
  #define LED           15 // Moteino MEGAs have LEDs on D15
  #define FLASH_SS      23 // and FLASH SS on D23
#else
  #define LED           9 // Moteinos have LEDs on D9
  #define FLASH_SS      8 // and FLASH SS on D8
#endif

#define SERIAL_BAUD   115200

int TRANSMITPERIOD = 2000; //transmit a packet to gateway so often (in ms)
char buff[50];
byte sendSize=0;

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

void  setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.println("== Debut du programme ==");
 
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  radio.encrypt(ENCRYPTKEY);
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
}


long lastPeriod = 0;

void loop() {
  // Envoie de l'onde
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Réception de l'écho
  duration = pulseIn(echoPin, HIGH);
  // Calcul de la distance
  distance = (duration/2) / 29.1;
  if(distance >= 1018 || distance <= 0){    
    Serial.println("Hors plage");
  } else
  {
    Serial.print("distance = ");
    Serial.print(distance);
    Serial.println(" cm");
  }
   
  
 

  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    lastPeriod=currPeriod;

    sprintf(buff, "{'distance':%d}", distance);
    sendSize = strlen(buff);
    
    if (radio.sendWithRetry(GATEWAYID, buff, sendSize))
     Serial.print(" ok!");
    else Serial.print(" nothing...");

    Serial.println();
  }
  // délai entre deux mesures
  delay(10000);
}
