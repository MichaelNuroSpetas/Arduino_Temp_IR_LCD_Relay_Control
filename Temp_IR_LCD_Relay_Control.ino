/*
By Michael Nuro Spetås
Michael.Nuro.Spetas@gmail.com 

Feel free to use and modify
*/


#include <IRremote.h>
#include <LiquidCrystal.h>

// Define the number of samples to keep track of. Higher number equals more smoothing
// .. But it takes more time to display the average temperature.
const int numReadings = 16;

int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                 // the average

byte setTemp = 25;               // temperature when the relay turns on
const byte hysterese = 3.5;      // hysterese

const byte RECV_PIN = 10;
const byte relayPin = 8;         // pin conntected to the relay    
const byte rPin = 6;             // pin connected to the red LED
const byte gPin = 9;             // pin connected to the green LED
const byte inputPin = A0;

int tempAdj;                     // temperature adjusted after a voltage drop of 40mV
int temp;

IRrecv irrecv(RECV_PIN);
decode_results results;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const byte lcdPin = 7;

void setup()
{
  Serial.begin(9600);            // serial communication with the computer for debugging
  pinMode(rPin, OUTPUT);         // declaring the pins as outputs, default is input
  pinMode(gPin, OUTPUT);
  pinMode(relayPin, OUTPUT); 
  pinMode(lcdPin, OUTPUT);
  digitalWrite(lcdPin, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("Calibrating sensor");
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  delay(500);
  digitalWrite(gPin, LOW);      // Blinking while waiting for the sensor to calibrate
  digitalWrite(rPin, HIGH);
  delay(500);
  digitalWrite(rPin, LOW);
  digitalWrite(gPin, HIGH);
  delay(500);
  digitalWrite(gPin, HIGH);
  digitalWrite(rPin, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Standby:");
  lcd.setCursor(0, 1);
  lcd.print("smash the remote");
  
  
  irrecv.enableIRIn();          // Start the receiver
  
  // initialize all the readings to 0: 
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;          
}

void loop() {  
  total= total - readings[index];           // subtract the last reading:    
  readings[index] = analogRead(inputPin);   // read from the sensor:  
  total= total + readings[index];           // add the reading to the total   
  index = index + 1;                        // continue to the next reading  
  if (index >= numReadings)                 // if we're at the end      
    index = 0;                              // start over again                  
  average = total / numReadings;            // calculating the average:
  temp = (5.0 * average * 100.0) / 1024;
  Serial.println(temp);
  Serial.print("\t");
  // temperature printing moved, due to a small voltage drop when the fan is
  // if there was no voltage drop then all lcd printing should be done here
  delay(50);        // delay in between reads for stability 
  if (irrecv.decode(&results)) {
      if (results.value == 0xFF5AA5) // if #6 on remote is pressed
      {
        setTemp = setTemp + 1;     // increase setTemp by 1
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("/");
        lcd.setCursor(4, 1);
        lcd.print(setTemp);
        Serial.println("increasing setTemp");
        if (setTemp > 35)          // if setTemp becomes larger than 35
        {
          setTemp = 35;             // then set it back to 35
        }
      }
      if (results.value == 0xFF38C7) // #5 on remote
      {
        setTemp = setTemp - 1;      // decrease setTemp by 1
        lcd.clear();
        lcd.setCursor(3, 1);
        lcd.print("/");
        lcd.setCursor(4, 1);
        lcd.print(setTemp);
        Serial.println("decreasing setTemp");
        if (setTemp < 20)          // if setTemp becomes smaller than 20
        {
          setTemp = 20;             // then set it back to 20
        }
      }
      if (results.value == 0xFF52AD || temp >= setTemp) // #9 on remote
      {
        digitalWrite(relayPin, HIGH);    // fan is connected normally closed (NC)
        digitalWrite(rPin, HIGH);
        digitalWrite(gPin, LOW);
        lcd.clear();
        lcd.begin(16, 2);
        lcd.print("Temperature:");
        lcd.setCursor(0, 1);
        lcd.print(temp);
        lcd.setCursor(2, 1);
        lcd.print((char)223);
        lcd.setCursor(6, 1);
        lcd.print((char)223); 
        lcd.setCursor(13, 1);
        lcd.println("OFF");
        lcd.setCursor(3, 1);
        lcd.print("/");
        lcd.setCursor(4, 1);
        lcd.print(setTemp);
        Serial.print("off");
        
      } 
      if (results.value == 0xFF4AB5 || temp - hysterese <= setTemp ) // #8 on remote
      {
        digitalWrite(relayPin, LOW);      // fan is connected normally closed (NC)
        digitalWrite(rPin, LOW);
        digitalWrite(gPin, HIGH); 
        lcd.clear();
        lcd.begin(16, 2);
        lcd.print("Temperature:");
        lcd.setCursor(0, 1);
        tempAdj = temp + 4;
        lcd.print(tempAdj);
        lcd.setCursor(2, 1);
        lcd.print((char)223);
        lcd.setCursor(6, 1);
        lcd.print((char)223);
        lcd.setCursor(13, 1);
        lcd.println("ON ");
        lcd.setCursor(3, 1);
        lcd.print("/");
        lcd.setCursor(4, 1);
        lcd.print(setTemp);
        Serial.print("on");
      }   
    irrecv.resume(); // Receive the next value
 }
}
