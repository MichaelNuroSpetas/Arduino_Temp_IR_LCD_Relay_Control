/*
By Michael Nuro Spetås
Michael.Nuro.Spetas@gmail.com 

Feel free to use and modify
*/


#include <IRremote.h>
#include <LiquidCrystal.h>

// Define the number of samples to keep track of. Higher number equals more smoothing
// .. But it takes more time to display the average temperature.
const int numReadings = 12;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average

const int setTemp = 26;         // temperature when the relay turns on
const int hysteresis = 3.5;      // hysteresis

const int RECV_PIN = 10;
const int relayPin = 8;                
const int rPin = 6;             // pin connected to the red LED
const int gPin = 9;             // pin connected to the green LED

float temp;
float celcius;
const int inputPin = A0;

IRrecv irrecv(RECV_PIN);
decode_results results;
boolean IRoverride = false;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define lcdPin 7

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);
  pinMode(rPin, OUTPUT);
  pinMode(gPin, OUTPUT);
  pinMode(relayPin, OUTPUT); 
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("Calibrating sensor:");
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  delay(500);
  digitalWrite(gPin, LOW); // Blinking while waiting for the sensor to calibrate
  digitalWrite(rPin, HIGH);
  delay(500);
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  delay(500);
  digitalWrite(gPin, HIGH);
  digitalWrite(rPin, HIGH);
  lcd.begin(16, 2);
  lcd.print("Temperature:");
  lcd.setCursor(6, 1);
  lcd.print("Standby");
  
  irrecv.enableIRIn(); // Start the receiver
  
  // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;          
}

void loop() {  
 // subtract the last reading:
  total= total - readings[index];         
  // read from the sensor:  
  readings[index] = analogRead(inputPin); 
  // add the reading to the total:
  total= total + readings[index];       
  // advance to the next position in the array:  
  index = index + 1;                    
  // if we're at the end of the array...
  if (index >= numReadings)            
  //index &= (array_size-1);  
    // ...wrap around to the beginning: 
    index = 0;                           
  // calculate the average:
  average = total / numReadings;
  temp = average * 0.48828125;
  //temp = (average*5000)/1024; //Converts raw sensos value to voltage value
  //celcius = temp/10;//sensor value*5000/bit^10 =1024, 10mV/1 degree celcius  
  Serial.println(temp);
  Serial.print("\t");
  delay(25);        // delay in between reads for stability 
  if (irrecv.decode(&results)) {
      if (results.value == 0xFF52AD || temp >= setTemp) // #9 on remote
      {
        digitalWrite(rPin, HIGH);
        digitalWrite(gPin, LOW);
        digitalWrite(relayPin, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(temp);
        lcd.setCursor(4, 1);
        lcd.print((char)223); 
        Serial.print("off");
        lcd.setCursor(6, 1);
        lcd.println("       OFF");
        
      } 
      else if (results.value == 0xFF4AB5 || temp - hysteresis <= setTemp ) // #8 on remote
      {
        digitalWrite(relayPin, LOW);
        digitalWrite(rPin, LOW);
        digitalWrite(gPin, HIGH);
        lcd.setCursor(0, 1);
        lcd.print(temp + 4);  // temp adjusted after a 40 mili voltage drop when the fan turns on
        lcd.setCursor(4, 1);
        lcd.print((char)223); 
        Serial.print("on");
        lcd.setCursor(6, 1);
        lcd.println("        ON ");
      }   
    irrecv.resume(); // Receive the next IR signal
 }
}
