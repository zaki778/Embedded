#include <Arduino_FreeRTOS.h>
#include <SPI.h>
#include <MFRC522.h>

//pins
int LDRLEDPin = 6;
int ONOFFPin = 26;
int ONOFFLEDPin = 24;
int BELTPin = 28;
int BELTOUTPin = 30;

//variables
bool dayra = 1;
bool unlocked = 1;
bool beltOn = 0;
bool frontObstacle = 0;


//orientation variables
bool north=true;
bool east = false;
bool west = false;

//turning & motion variables
bool front=true;
bool right= false;
bool left=false;

//H-Bridge variables
#define frontr1 44
#define frontr2 45
#define backr1 42
#define backr2 43
#define frontl1 46
#define frontl2 47
#define backl1 48
#define backl2 49


//Defines RFID
#define SS_PIN 53
#define RST_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

//Defines Ultrasonic Sensors
float durationFront, distanceFront, durationRight, distanceRight, durationLeft, distanceLeft;
#define trigPinFront 12
#define echoPinFront 13
#define trigPinRight 11
#define echoPinRight 10
#define trigPinLeft 9
#define echoPinLeft 8


void setup() {
  Serial.begin(9600);
  SPI.begin();

  //pin setup
  pinMode(LDRLEDPin,OUTPUT); // LDR
  pinMode(ONOFFPin,INPUT); // on/off switch
  pinMode(ONOFFLEDPin,OUTPUT); // on/off indicator
  pinMode(BELTPin,INPUT); // belt switch
  pinMode(BELTOUTPin,OUTPUT); // belt indicator

  //H-Bridge
  pinMode(frontr1, OUTPUT);
  pinMode(frontr2, OUTPUT);
  pinMode(backr1, OUTPUT);
  pinMode(backr2, OUTPUT);
  pinMode(frontl1, OUTPUT);
  pinMode(frontl2, OUTPUT);
  pinMode(backl1, OUTPUT);
  pinMode(backl2, OUTPUT);

  //RFID
  mfrc522.PCD_Init();   // Initiate MFRC522

  //Ultrosonic Sensors
  pinMode(trigPinFront, OUTPUT);
  pinMode (echoPinFront, INPUT);

  //task creation
  //why 100? maybe it's not enought to handle the functionality
 // xTaskCreate(LDRTask, "LDR", 100, NULL, 2, NULL); // handles switching the LED on LDR change
  //xTaskCreate(ONOFF, "On/off Switch", 100, NULL, 2, NULL); // handles turning on the car with the start stop switch
  xTaskCreate(Belt, "Belt alarm", 100, NULL, 2, NULL); // handles belt alarm
  xTaskCreate(UltraSonicTask, "ultrasonic sensors", 50, NULL, 2, NULL); // ultrasonic sensor
  //xTaskCreate(MotionTask, "motion  as idle task", 1, NULL, 4, NULL); // motion

  vTaskStartScheduler();

}

void loop() {}

static void Belt(void* vParameters){

//  TichType_t xLastWakeTime;
//  xLastWakeTime = xTastGetTickCount();

  while(1){
    Serial.print("Belt");
    int delay = 500;
    int button = digitalRead(BELTPin);
    if(button == HIGH && !beltOn){
      beltOn = 1;
      digitalWrite(BELTOUTPin, LOW);
    }
    else if(button == HIGH && beltOn){
      beltOn = 0;
    }
    if(!beltOn){
      digitalWrite(BELTOUTPin, HIGH);
      vTaskDelay(250/portTICK_PERIOD_MS);
      digitalWrite(BELTOUTPin, LOW);
      vTaskDelay(125/portTICK_PERIOD_MS);
      delay = 125;
    }
    vTaskDelay(delay/portTICK_PERIOD_MS);
    //maybe we could try delay untill instead as delay untill start the delay from the moment we enter the task
    //however taskDelay start delay from the moment we call it!
    //vTaskDelayUntill(&xLastWakeTime, pdMS_TO_TICKS(1000));
  }
}

static void ONOFF(void* vParameters){
  while(1){
////    // Look for new cards
////    if ( ! mfrc522.PICC_IsNewCardPresent())
////    {
////      goto dest;
////    }
////    // Select one of the cards
////    if ( ! mfrc522.PICC_ReadCardSerial())
////    {
////      goto dest;
////    }
//    //Show UID on serial monitor
//  //Serial.print("UID tag :");
//  String content= "";
//  byte letter;
//  for (byte i = 0; i < mfrc522.uid.size; i++)
//  {
//     //Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//     //Serial.print(mfrc522.uid.uidByte[i], HEX);
//     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
//     content.concat(String(mfrc522.uid.uidByte[i], HEX));
//  }
//  //Serial.println();
//  //Serial.print("Message : ");
//  content.toUpperCase();
//      if (content.substring(1) == "F9 93 B2 8E" || content.substring(1) == "5C 1C EF 17"){
//        unlocked = 1;
//        }
      dest:int button = digitalRead(ONOFFPin);
//    dest:int button = HIGH;
//    //Serial.print(button);
    if(unlocked && button==HIGH && !dayra){
      Serial.print("daret ya seya3");
      dayra = 1;
      digitalWrite(ONOFFLEDPin,HIGH);
    }
    else if(unlocked && button==HIGH && dayra){
      Serial.print("ana matet");
      dayra = 0;
      digitalWrite(ONOFFLEDPin,LOW);
    }
    else if(!unlocked){
      dayra = 0;
      digitalWrite(ONOFFLEDPin,LOW);
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}


static void LDRTask(void* vParameters){
  while(1){
    int analogValue = analogRead(A0);
 
    Serial.print("Analog reading = ");
    Serial.print(analogValue);   // the raw analog reading
 
    // We'll have a few threshholds, qualitatively determined
    if (analogValue < 75) {
      Serial.println(" - Bright");
      digitalWrite(LDRLEDPin,LOW);
    }
    else {
      Serial.println(" - Dark");
      digitalWrite(LDRLEDPin,HIGH);
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

static void UltraSonicTask(void* vParameters){
  while(1){
     if(north){
      // Write a pulse to the HC-SR04 Trigger Pin
    digitalWrite(trigPinFront, LOW) ;
    delayMicroseconds(2);
    digitalWrite(trigPinFront, HIGH) ;
    delayMicroseconds(10);
    digitalWrite(trigPinFront, LOW) ;
   
    // Measure the response from the HC-SR04 Echo Pin
    durationFront = pulseIn(echoPinFront, HIGH) ;
    // Determine distance from duration
    // Use 343 metres per second as speed of sound
    distanceFront = (durationFront / 2) * 0.0343;
    // Send results to Serial Monitor
    Serial.print ("Distance Front Sensor = ");
    if (distanceFront < 15) {
      Serial.println("too close");
      String direction = checkLeftnRight();
      if(direction=="Left"){
        front=false;
        left=true;
      }
      else{
        front=false;
        right=true;
      }
    }
    else {
      Serial.println("ta3ala hatgeebak");
    }
   
     }
     else if(west){
        digitalWrite(trigPinRight, LOW) ;
        delayMicroseconds(2);
        digitalWrite(trigPinRight, HIGH) ;
        delayMicroseconds(10);
        digitalWrite(trigPinRight, LOW) ;
       
        // Measure the response from the HC-SR04 Echo Pin
        durationRight = pulseIn(echoPinRight, HIGH) ;
        // Determine distance from duration
        // Use 343 metres per second as speed of sound
        distanceRight = (durationRight / 2) * 0.0343;
        Serial.print(distanceRight);
        // Send results to Serial Monitor
        Serial.print ("Distance Right Sensor = ");
        if (distanceRight <0) {
        Serial.println("kamel kaman");
        }
        else {
          front =false;
          right=true;
        }
     }
     else if(east){
        digitalWrite(trigPinLeft, LOW) ;
        delayMicroseconds(2);
        digitalWrite(trigPinLeft, HIGH) ;
        delayMicroseconds(10);
        digitalWrite(trigPinLeft, LOW) ;
       
        // Measure the response from the HC-SR04 Echo Pin
        durationLeft = pulseIn(echoPinLeft, HIGH) ;
        // Determine distance from duration
        // Use 343 metres per second as speed of sound
        distanceLeft = (durationLeft / 2) * 0.0343;
        // Send results to Serial Monitor
        Serial.print ("Distance Left Sensor = ");
        if (distanceLeft < 15) {
        Serial.println(distanceLeft);
        }
        else {
          front=false;
          left=true;
        }
     }
    vTaskDelay(500/portTICK_PERIOD_MS);
     
   
  }
}

static String checkLeftnRight(){
        digitalWrite(trigPinLeft, LOW) ;
        delayMicroseconds(2);
        digitalWrite(trigPinLeft, HIGH) ;
        delayMicroseconds(10);
        digitalWrite(trigPinLeft, LOW) ;
        durationLeft = pulseIn(echoPinLeft, HIGH) ;
        distanceLeft = (durationLeft / 2) * 0.0343;
        digitalWrite(trigPinRight, LOW) ;
        delayMicroseconds(2);
        digitalWrite(trigPinRight, HIGH) ;
        delayMicroseconds(10);
        digitalWrite(trigPinRight, LOW) ;
        durationRight = pulseIn(echoPinRight, HIGH) ;
        distanceRight = (durationRight / 2) * 0.0343;
        if(distanceRight>distanceLeft){
          return "Right";
        }
        else{
          return "Left";
        }
}
static void MotionTask(void* vParameters){
        while(1){
           if(dayra){
           if(front){
              Serial.print("Going forward");
              digitalWrite(frontr1,0);
              digitalWrite(frontr2,1);
              digitalWrite(backr1,0);
              digitalWrite(backr2,1);
              digitalWrite(frontl1,0);
              digitalWrite(frontl2,1);
              digitalWrite(backl1,0);
              digitalWrite(backl2,1);
              }
              else if(right){
                for(int i=0;i<9750;i++){
              digitalWrite(frontr1,1);
              digitalWrite(frontr2,0);
              digitalWrite(backr1,1);
              digitalWrite(backr2,0);
              digitalWrite(frontl1,0);
              digitalWrite(frontl2,1);
              digitalWrite(backl1,0);
              digitalWrite(backl2,1);
               }
              digitalWrite(frontr1,0);
              digitalWrite(frontr2,0);
              digitalWrite(backr1,0);
              digitalWrite(backr2,0);
              digitalWrite(frontl1,0);
              digitalWrite(frontl2,0);
              digitalWrite(backl1,0);
              digitalWrite(backl2,0);
              right=false;
              front=true;
              if(north){
                north=false;
                east=true;    
              }
              else if(west){
                west=false;
                north=true;
              }
              }
              else if(left){
                for(int i=0;i<9750;i++){
               digitalWrite(frontr1,0);
              digitalWrite(frontr2,1);
              digitalWrite(backr1,0);
              digitalWrite(backr2,1);
              digitalWrite(frontl1,1);
              digitalWrite(frontl2,0);
              digitalWrite(backl1,1);
              digitalWrite(backl2,0);
                }
              digitalWrite(frontr1,0);
              digitalWrite(frontr2,0);
              digitalWrite(backr1,0);
              digitalWrite(backr2,0);
              digitalWrite(frontl1,0);
              digitalWrite(frontl2,0);
              digitalWrite(backl1,0);
              digitalWrite(backl2,0);
              left =false;
              front=true;
              if(north){
                north=false;
                west=true;  
              }
              else if(east){
                east=false;
                north=true;
              }
              }
          // vTaskDelay(200/portTICK_PERIOD_MS);  
        }
        else{
           digitalWrite(frontr1,0);
              digitalWrite(frontr2,0);
              digitalWrite(backr1,0);
              digitalWrite(backr2,0);
              digitalWrite(frontl1,0);
              digitalWrite(frontl2,0);
              digitalWrite(backl1,0);
              digitalWrite(backl2,0);
        }
        }
   


 
}
