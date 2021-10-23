/*
  Tests if your arduino is receiving and reading analog signals from the op-amp
  Upload the sketch, then open serial monitor to see values. Set monitor baud rate at 9600.
*/

const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

int sensorValue = 0;        // value read from the pot

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
}

void loop() {
  sensorValue = analogRead(analogInPin);
  // print the results to the Serial Monitor:
  Serial.println("sensor = ");
  Serial.println(sensorValue);
  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  delay(2);
}
