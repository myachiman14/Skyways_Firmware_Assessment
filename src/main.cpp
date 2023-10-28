#include <Arduino.h>

//define pin configuration
#define tempSensorPin 3
#define coolerPin 16

//Control Modes
#define manualMode 0x00 //enabled - Cooler ON; disabled - Cooler OFF
#define bangBangMode 0x01 //enabled - Cooler in Bang Bang Mode; disabled - Cooler OFF
bool coolerEnable = false;
bool status = 0; // 0-cooler is off; 1- cooler is on
int temp;

void setup() {
  // put your setup code here, to run once:
  pinMode(coolerPin,OUTPUT); //set cooler outpin pin
  Serial1.begin(115200); //begin serial communication on rx1 and tx1 (UART 1) - default configuration
}

void loop() {
  if(Serial1.available() >= 2){
    byte byte0 = Serial1.read();//Read in first byte
    byte byte1 = Serial1.read();//Read in second byte 

    if(byte0 == 0x01){ //First check if cooler should be enabled or disabled
      if(byte1 == 0x00){
        coolerEnable = false;
      }
      else if(byte1 == 0x01){
        coolerEnable = true;
      }
    }

    if(coolerEnable && (byte0==0x00)){ //If Mode Command is sent and cooler is enabled
      controlModes(byte1);
    }
    else{
      digitalWrite(coolerPin,LOW);
      status = 0;//Update Status
    }

    sendTelemetry();
    delay(1000); //Delay to update input and send telemtry data as often as desired
       

  }
  
}

int getTemp(){
    //Get Average temp reading over 15 iterations to help reduce noisy output
    //If this does not give accurate enough readings, consider using a filter
    int i = 0;
    int iterations = 15;
    int sum = 0;
    int currentTemp = 0;
    while( i <iterations){
      int voltage = analogRead(tempSensorPin);
      currentTemp = map(voltage,0,1023,0,100);
      int sum = sum+currentTemp;
      i++;
    }
    currentTemp = sum/iterations;
    return currentTemp;
}

void controlModes(byte mode){
  switch(mode){ 
  case manualMode:
    digitalWrite(coolerPin,HIGH); //ManualMode and Cooler Enable
    status = 1; //Update Status
  break;

  case bangBangMode:
    int currentTemp = getTemp();
    if(currentTemp<12){ //12 as low end threshold to allow for 2 degree overshoot
      digitalWrite(coolerPin,LOW); //Turn cooler off if below threshold
      status = 0;//Update Status
    }
    else if(currentTemp>18){//12 as low end threshold to allow for 2 degree overshoot
      digitalWrite(coolerPin,HIGH); //Turn cooler on if above threshold 
      status =1;//Update Status
    }
  break;
  //add additional cases for different modes
}
}

void sendTelemetry(){
  byte byte0 = 0x10; //Status Message
  byte byte1 = 0x00; //Assume cooler is off
  if(status ==1){
    byte byte1 = 0x01; //Change byte1 if cooler is on
  }
  Serial1.write(byte0);
  Serial1.write(byte1);

  byte0 = 0x11; //Temperature Message
  unsigned int temp = getTemp();
  byte1 = temp;
  Serial1.write(byte0);
  Serial1.write(byte1);
    
}