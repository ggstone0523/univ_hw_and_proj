#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;
SoftwareSerial mySerial(10, 11);

int pins[] = {3, 5, 7, 8, 13};
byte pattern = 0x1F;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mySerial.begin(9600);
  while (!huskylens.begin(mySerial))
  {
      Serial.println(F("Begin failed!"));
      Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
      Serial.println(F("2.Please recheck the connection."));
      delay(100);
  }

  for(int i=0; i<5; i++){
    pinMode(pins[i], OUTPUT);  
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!huskylens.request()){
    turnOffLED();  
    Serial.println(F("Fail to request data from HUSKYLENS, recheck the connection!"));
  }
  else if(!huskylens.isLearned()){
    turnOffLED();  
    Serial.println(F("Nothing learned, press learn button on HUSKYLENS to learn one!"));
  }
  else if(!huskylens.available()){
    turnOffLED();  
    Serial.println(F("No block or arrow appears on the screen!"));
  }
  else
  {
    Serial.println(F("###########"));
    while (huskylens.available())
    {
        HUSKYLENSResult result = huskylens.read();
        if(result.ID == 1){
          turnOnLED();
          printResult(result);
        }
        else{
          turnOffLED();
          Serial.println(F("Not registered"));  
        }
    }    
  }
}

void turnOffLED(){
  for(int i=0; i<5; i++){
    digitalWrite(pins[i], LOW);  
  }
}

void turnOnLED(){
  for(int i=0; i<5; i++){
    boolean state = (pattern >> i) | 1;
    digitalWrite(pins[i], state);  
  }
}

void printResult(HUSKYLENSResult result){
    if (result.command == COMMAND_RETURN_BLOCK){
        Serial.println(String()+F("Block:xCenter=")+result.xCenter+F(",yCenter=")+result.yCenter+F(",width=")+result.width+F(",height=")+result.height+F(",ID=")+result.ID);
    }
    else if (result.command == COMMAND_RETURN_ARROW){
        Serial.println(String()+F("Arrow:xOrigin=")+result.xOrigin+F(",yOrigin=")+result.yOrigin+F(",xTarget=")+result.xTarget+F(",yTarget=")+result.yTarget+F(",ID=")+result.ID);
    }
    else{
        Serial.println("Object unknown!");
    }
}
