#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include "Timer.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#define USE_ARDUINO_INTERRUPTS true
#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library. 
   
#define DEBUG true
#define SSID "ifyoulike"     // "SSID-WiFiname"
#define PASS "iloveyou" // "password"
#define IP "184.106.153.149"      // thingspeak.com ip

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);


Timer t;
PulseSensorPlayground pulseSensor;

String msg = "GET /update?key=IGJ80KGP7E2XMB1W"; 
SoftwareSerial esp8266(10,11);

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

//Variables
const int PulseWire = A0;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           //for heart rate sensor
int buzzer = 9;
float myTemp;
int myBPM;
String BPM;
String temp;
int error;
int panic;
int raw_myTemp;
float Voltage;
float tempC;
int airquality;
int airval;
void setup()
{
  pinMode(buzzer, OUTPUT);
 
  Serial.begin(9600); 
  esp8266.begin(115200);
  sensors.begin();
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(" Patient Health");
  lcd.setCursor(0,1);
  lcd.print(" Monitoring ");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Initializing....");
  delay(5000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Getting Data....");
  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }
  Serial.println("AT");
  esp8266.println("AT");

  delay(3000);

  if(esp8266.find("OK"))
  {
    connectWiFi();
  }
  t.every(10000, getReadings);
   t.every(10000, updateInfo);
}

void loop()
{
//  panic_button();
start: //label
    error=0;
   t.update();
    //Resend if transmission is not completed
    if (error==1)
    {
      goto start; //go to label "start"
    } 
 delay(4000);
}

void updateInfo()
{
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  esp8266.println(cmd);
  delay(2000);
  if(esp8266.find("Error"))
  {
    return;
  }
  cmd = msg ;
  cmd += "&field1=";    //field 1 for BPM
  cmd += BPM;
  cmd += "&field2=";  //field 2 for temperature
  cmd += airval;
  cmd += "&field3=";  //field 3 for temp
  cmd += temp;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  esp8266.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  esp8266.println(cmd.length());
  if(esp8266.find(">"))
  {
    Serial.print(cmd);
    esp8266.print(cmd);
  }
  else
  {
    Serial.println("AT+CIPCLOSE");
    esp8266.println("AT+CIPCLOSE");
    //Resend...
    error=1;
  }
}

boolean connectWiFi()
{
  Serial.println("AT+CWMODE=1");
  esp8266.println("AT+CWMODE=1");
  delay(2000);
  String cmd="AT+CWJAP=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  Serial.println(cmd);
  esp8266.println(cmd);
  delay(5000);
  if(esp8266.find("OK"))
  {
    return true;
  }
  else
  {
    return false;
  }
}

void getReadings(){
   airval = analogRead(A1);
  //airval = (air / 10) * 100;
  //Voltage = (raw_myTemp / 1023.0) * 5000; // 5000 to get millivots.
  //tempC = Voltage * 0.1; 
  //myTemp = (tempC * 1.8) + 32; // conver to F
  //Serial.println(myTemp);
  int raw_BPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
  int myBPM = raw_BPM * 0.32;                                            // "myBPM" hold this BPM value now. 
                                               
if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
Serial.println(myBPM);                        // Print the value inside of myBPM.
}
  temp = sensors.getTempCByIndex(0);
  
  
  //delay(20);            
    char buffer1[10];
     //char buffer2[10];
    BPM = dtostrf(myBPM, 4, 1, buffer1);
    //airquality = dtostrf(airval, 4, 1, buffer2); 
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BPM:");
    lcd.setCursor(4,0);
    lcd.print(myBPM);
    lcd.setCursor(7, 0);
    
   if(airval >= 0 && airval<= 50){
    lcd.setCursor(0,1);
    lcd.print("Air:");
    lcd.setCursor(7,1);
    lcd.print("Healthy");
    digitalWrite(buzzer, LOW);
   }
   else if(airval > 50 && airval <= 100){
    lcd.setCursor(0,1);
    lcd.print("Air:");
    lcd.setCursor(7,1);
    lcd.print("Moderate");
    digitalWrite(buzzer, LOW);
   }
   else if(airval > 100 ){
    lcd.setCursor(0,1);
    lcd.print("Air:");
    lcd.setCursor(4,1);
    lcd.print("Unhealthy");
    digitalWrite(buzzer, HIGH);
   }
   
   sensors.requestTemperatures();
  lcd.setCursor(7, 0);
  lcd.print("Temp:");
  lcd.print(sensors.getTempCByIndex(0));
  //lcd.print((char)176);//shows degrees character
  lcd.print("C");
  
  }

  


/*void panic_button(){
  panic = digitalRead(9);
    if(panic == HIGH){
    Serial.println(panic);
      String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  Serial.println(cmd);
  esp8266.println(cmd);
  delay(2000);
  if(esp8266.find("Error"))
  {
    return;
  }
  cmd = msg ;
  cmd += "&field3=";    
  cmd += panic;
  cmd += "\r\n";
  Serial.print("AT+CIPSEND=");
  esp8266.print("AT+CIPSEND=");
  Serial.println(cmd.length());
  esp8266.println(cmd.length());
  if(esp8266.find(">"))
  {
    Serial.print(cmd);
    esp8266.print(cmd);
  }
  else
  {
    Serial.println("AT+CIPCLOSE");
    esp8266.println("AT+CIPCLOSE");
    //Resend...
    error=1;
  }
}
}*/
