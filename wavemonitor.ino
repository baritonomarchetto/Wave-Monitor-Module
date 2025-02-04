/*
Wave Monitor for modular synthesizers
Based on "Simple oscilloscope (_20190212_OLEDoscilloscope.ino)" by radiopench
http://radiopench.blog96.fc2.com/blog-entry-893.html

Mods to the original sketch:
- single button action (time base)
- auto voltage range mode only
- removed all measurement capabilities and visual informations for faster execution and higher view area
- reduced horizontal ranges
- some minor change to face hardware modifications

Software-wise, this was mainly a trimming of an existing code work, so all credits goes 
to radiopench.

Hardware modifications: any incoming signal is forced in the 0 to +5V range by an active 
comparator. Signals in the range +/-12V where planned, but a different range is possible changing
some resistor value.
The signal input goes to analog pin "A0". The single button here used is connected 
to D2. Software debounce is implemented. No other component is required.
SSD1306 OLED screen is connected to SDA and SCL pins (A4 and A5 respectively in arduino nano) and 
powered by +5V and GND.

by barito, oct 2021
www.synthbrigade.altervista.org
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128        // OLED display width
#define SCREEN_HEIGHT 64        // OLED display height
#define REC_LENGTH 250          // Waveform data buffer size

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1      // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int waveInPin = A0;      // analog input pin
const int hRangePin = 2;       //h-range selection switch

int waveBuff[REC_LENGTH];      // Waveform data recording memory (RAM is barely available)

volatile int hRange;           // Horizontal range 0:50m, 1:20m, 2:10m, 3:5m, 4;2m, 5:1m, 6:500u, 7;200u
volatile int trigD;            // Trigger direction (polarity) 0: positive, 1: negative

int dataMin;                   // Minimum buffer value (min: 0)
int dataMax;                   // Maximum buffer value (max: 1023)
int trigP;                     // Trigger position on the data buffer

boolean hRangeState;           // h-range button state
unsigned long hRangeTime;      // h-range button last press (software debounce)
int debTime = 20;              // ms; Debounce time
boolean NOSIGNAL;              // no incoming signal

void setup() {
  pinMode(2, INPUT_PULLUP);                // h-range pin
  pinMode(13, OUTPUT);                     // Status display
  hRangeState = digitalRead(hRangePin);    // initialize h-range button status
  hRange = 3;
 //     Serial.begin(115200);        // Using this consumes a lot of RAM
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    //       Serial.println(F("SSD1306 failed"));
    for (;;);                              // Don't proceed, loop forever
  }
  startScreen();                           // Draw common parts
}

void loop() {
  display.clearDisplay();                  // Clear all screen
  digitalWrite(13, HIGH);
  readWave();                              // Waveform read (minimum 1.6ms)
  digitalWrite(13, LOW);
  dataAnalize();                           // Collect various data information 
  plotData();                              // Graph plot
  display.display();                       // Transfer the buffer and display it
  ButtonHandling();
}

void readWave() {                          // Record waveform in memory
switch (hRange) {                          // Change the recording speed according to the range

  case 0: {                                // 50ms range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x07;              // set prescaler to 128 (arduino original, higher prescaler)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(1888);           // Sampling cycle adjustment
      }
      break;
    }

  case 1: {                                // 20ms range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x07;              // set prescaler to 128 (arduino original, higher prescaler)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(688);            // Sampling cycle adjustment
      }
      break;
    }

  case 2: {                                // 10 ms range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x07;              // set prescaler to 128 (arduino original, higher prescaler)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(288);            // Sampling cycle adjustment
      }
      break;
    }

  case 3: {                                // 5 ms range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x07;              // set prescaler to 128 (arduino original, higher prescaler)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(88);             // Sampling cycle adjustment
      }
      break;
    }

  case 4: {                                // 2 ms range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x06;              // set prescaler to 64 (0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(24);             // Sampling cycle adjustment
      }
      break;
    }

  case 5: {                                // 1 ms range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x05;              // set prescaler to 32 (0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(12);             // Sampling cycle adjustment
      }
      break;
    }
    
  case 6: {                                // 500us range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x04;              // set prescaler to 16 (0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
        delayMicroseconds(4);              // Sampling cycle adjustment
      // Time fine adjustment 1.875 μs (1 clock with one nop, 0.0625 μs @ 16 MHz)
      asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
      asm("nop"); asm("nop"); asm("nop");
      }
    break;
  }

  case 7: {                                // 200us range
      ADCSRA = ADCSRA & 0xf8;              // enable AD convertion
      ADCSRA = ADCSRA | 0x02;              // set prescaler to 4(0x1=2, 0x2=4, 0x3=8, 0x4=16, 0x5=32, 0x6=64, 0x7=128)
      for (int i = 0; i < REC_LENGTH; i++) {
        waveBuff[i] = analogRead(waveInPin);
      // Time fine adjustment 1.875 μs (1 clock with one nop, 0.0625 μs @ 16 MHz)
      asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
      asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
      asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop");
      }
    break;
  }
}
}

void dataAnalize() {                      //Set various information for drawing
  int d;
  long sum = 0;

  // Find the maximum and minimum values
  dataMin = 1023;                         // Initialize minimum
  dataMax = 0;                            // Initialize maximum
  for (int i = 0; i < REC_LENGTH; i++) {  // Find the maximum and minimum values
    d = waveBuff[i];
    sum = sum + d;
    if (d < dataMin) {
      dataMin = d;                        // minimum value
    }
    if (d > dataMax) {
      dataMax = d;                        // maximum value
    }
  }
  if (dataMin + 5 > dataMax){            // the value added to dataMin avoids to display very low volume signals. The lower the volume, the lower the wave resolution. 
    NOSIGNAL = true;
  }
  else {
    NOSIGNAL = false;
  }

if(NOSIGNAL == false){
  // Find the trigger position
  for (trigP = ((REC_LENGTH / 2) - 51); trigP < ((REC_LENGTH / 2) + 50); trigP++) { // Find the point that straddles the median in the center of the data range
    if (trigD == 0) {                     // If the trigger direction is 0 (positive trigger)
      if ((waveBuff[trigP - 1] < (dataMax + dataMin) / 2) && (waveBuff[trigP] >= (dataMax + dataMin) / 2)) {
        break;                            // Rise trigger detection!
      }
    } else {                              // If not 0 (negative trigger)
      if ((waveBuff[trigP - 1] > (dataMax + dataMin) / 2) && (waveBuff[trigP] <= (dataMax + dataMin) / 2)) {
        break;
      }                                   // Fall trigger detection!
    }
  }
  if (trigP >= ((REC_LENGTH / 2) + 50)) { // If the trigger is not found, center it
    trigP = (REC_LENGTH / 2);
  }
}
}

void startScreen() {                 // Screen display at start
  display.clearDisplay();
  display.setTextSize(2);            // Double-angle characters
  display.setTextColor(WHITE);       
  display.setCursor(10, 15);         
  display.println(F("WAVE MON"));    // welcome message
  display.setCursor(10, 35);         
  display.setTextSize(1);            // Standard font size 
  display.println(F("v0.1b"));
  display.display();
  delay(7000);
  display.clearDisplay(); 
}

void plotData() {                    // Plot data based on array values
  long y1, y2;
if (NOSIGNAL){
    display.drawLine(0, 32, SCREEN_WIDTH, 32, WHITE);                                 // straight line at screen center
}
else{ //signal is present
  for (int x = 0; x <= SCREEN_WIDTH; x++) {
    y1 = map(waveBuff[x + trigP -50], dataMin, dataMax, SCREEN_HEIGHT, 0); // Convert wave buffer values to plot coordinates
    y1 = constrain(y1, 2, 62);                                             // Constrain values withing the display area
    y2 = map(waveBuff[x + trigP -51], dataMin, dataMax, SCREEN_HEIGHT, 0); // Very next point, for line connection
    y2 = constrain(y2, 2, 62);                                             // Constrain values withing the display area
    display.drawLine(x, y1, x, y2, WHITE);                                 // Connect the two points with a line
  }
}
}

void ButtonHandling() {                   // Buttons handling
if(digitalRead(hRangePin) != hRangeState && millis()- hRangeTime > debTime){
  hRangeState = !hRangeState;
  hRangeTime = millis();
  if(hRangeState == LOW){       // Button pressed (input pullup)
    hRange++;    
    if (hRange > 5) {           //reduced range
      hRange = 2;               // Cycle back 
    }
  }
}
}
