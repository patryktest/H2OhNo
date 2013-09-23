/*
 1-14-2013
 Spark Fun Electronics
 Nathan Seidle
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 When the ATtiny senses water between two pins, go crazy. Make noise, blink LED.

 Created to replace the water sensor in my Nauticam 17401 underwater enclosure. The original board
 ate up CR2032s, sorta kinda worked some of the time, and had pretty low quality assembly. Did I mention it goes for $100?!
 We have the technology. We can make it better!
 
 To use this code you must configure the ATtiny to run at 8MHz so that serial and other parts of the code work correctly.
 
 We take a series of readings of the water sensor at power up. We then wait for a deviation of more than
 100 from the average before triggering the alarm.
 
 The alarm will run for a minimum of 2 seconds before shutting down.
 
 The original board had the following measurements @ 3.21V:
 In alarm mode: ~30mA with LED on and making sound
 Off: 10nA. Really? Wow. 
 
 This firmware doesn't yet put any power savings in place but should be possible (wake up every few second and take measurement).
 
*/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(4, 3); // RX, TX

//Pin definitions for regular Arduino Uno (used during development)
/*const byte buzzer1 = 8;const byte buzzer2 = 9;
const byte statLED = 10;
const byte waterSensor = A0;*/

//Pin definitions for actual ATtiny
const byte buzzer1 = 0;
const byte buzzer2 = 1;
const byte statLED = 4;
const byte waterSensor = A1;//2;

//Variables

//This is the average analog value found during startup. Usually ~995
//When hit with water, the analog value will drop to ~400. A diff of 100 is good.
int waterAvg = 0; 
int maxDifference = 100; //A diff of more than 100 in the analog value will trigger the system.

void setup()
{
  pinMode(buzzer1, OUTPUT);
  pinMode(buzzer2, OUTPUT);
  pinMode(statLED, OUTPUT);

//  pinMode(waterSensor, INPUT_PULLUP);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH); //Hack for getting around INPUT_PULLUP
  
  mySerial.begin(9600);
  mySerial.println("H2Ohno!");
  
  //Take a series of readings from the water sensor and average them
  waterAvg = 0;
  for(int x = 0 ; x < 8 ; x++)
  {
    waterAvg += analogRead(waterSensor);

    //During power up, blink the LED to let the world know we're alive
    if(digitalRead(statLED) == LOW)
      digitalWrite(statLED, HIGH);
    else
      digitalWrite(statLED, LOW);

    delay(50);
  }
  waterAvg /= 8;
  
  mySerial.print("Avg: ");
  mySerial.println(waterAvg);

  //During power up, beep the buzzer to verify function
  alarmSound();
  delay(100);
  alarmSound();
  
  /*while(1){
    digitalWrite(statLED, HIGH);
    delay(250);
    digitalWrite(statLED, LOW);
    delay(250);
  }*/
}

void loop() 
{
  //Check for water
  int waterDifference = abs(analogRead(waterSensor) - waterAvg);
  
  if(waterDifference > maxDifference) //Ahhh! Water! Alarm!
  {
    long startTime = millis(); //Record the current time
    long timeSinceBlink = millis(); //Record the current time for blinking
    digitalWrite(statLED, HIGH); //Start out with the uh-oh LED on

    //Loop until we don't detect water AND 2 seconds of alarm have completed
    while(waterDifference > maxDifference || (millis() - startTime) < 2000)
    {
      alarmSound(); //Make noise!!

      if(millis() - timeSinceBlink > 100) //Toggle the LED every 100ms
      {
        timeSinceBlink = millis();
        
        if(digitalRead(statLED) == LOW) 
          digitalWrite(statLED, HIGH);
        else
          digitalWrite(statLED, LOW);
      }
      
      waterDifference = abs(analogRead(waterSensor) - waterAvg); //Take a new reading
  
      mySerial.print("Read: ");
      mySerial.println(analogRead(waterSensor));
    } //Loop until we don't detect water AND 2 seconds of alarm have completed
    
    digitalWrite(statLED, LOW); //No more alarm. Turn off LED
  }
    
  delay(100);
}

// This is just a unique (annoying) sound we came up with, there is no magic to it
// Comes from the Simon Says game/kit actually
//250us to 79us
void alarmSound(void)
{
  // Toggle the buzzer at various speeds
  for (byte x = 250 ; x > 70 ; x--)
  {
    for (byte y = 0 ; y < 3 ; y++)
    {
      digitalWrite(buzzer2, HIGH);
      digitalWrite(buzzer1, LOW);
      delayMicroseconds(x);
      //microDelay(x);

      digitalWrite(buzzer2, LOW);
      digitalWrite(buzzer1, HIGH);
      delayMicroseconds(x);
      //microDelay(x);
    }
  }
}

//It turns out Arduino IDE v1.0.3 doesn't support 1MHz delayMicroseconds: 
//https://github.com/damellis/attiny/issues/12
//So we're going to fake it with nops()
void microDelay(byte amount)
{
  for(byte x = 0 ; x < amount ; x++)
    __asm__("nop\n\t");
}