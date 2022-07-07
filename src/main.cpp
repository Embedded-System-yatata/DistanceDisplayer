#include <Arduino.h>



#define pinSonarTriger 2
#define pinSonarEcho 3


float speedWave = 343;


long durationMS; // variable for the duration of sound wave travel
double distance; // variable for the distance measurement



float expotential (float base, float expoente){

  int i = 0;
  float result = 1;

  if (expoente > 0){

    for(i = 0; i < expoente; i++){

      result *= base;
    }


  }else if (expoente < 0){

    for(i = 0; i < -expoente; i++){

      result /= base;
    }

  }

  return result;

}



float average(float numbers[]){

  int n = sizeof(numbers)/sizeof(float);
  float av = 0;

  int i;
  for(i=0; i<n; i++){
    av += numbers[n];
  }

  return av/n;
}


void setup() {
  pinMode(pinSonarTriger, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(pinSonarEcho, INPUT); // Sets the echoPin as an INPUT
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
}


void loop() {
  // Clears the trigPin condition
  digitalWrite(pinSonarTriger, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(pinSonarTriger, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinSonarTriger, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  durationMS = pulseIn(pinSonarEcho, HIGH);
  // Calculating the distance
  //distance = expotential(durationMS,-6) * speedWave / 2; // Speed of sound wave divided by 2 (go and back)
  distance = durationMS*pow(10,-6) * speedWave / 2;
  //Serial.printf("%0.8f",float(1/pow(durationMS,6)));
  //distance = durationMS*0.034/2;
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println("m");
}
