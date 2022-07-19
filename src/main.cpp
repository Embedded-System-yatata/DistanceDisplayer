#include <Arduino.h>
#include <math.h>



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



float* orderAsc(float numbers []){

  int i = 0;
  int j = 0;
  int a = 0;

  int n = sizeof(numbers)/sizeof(float);

  for (i = 0; i < n; ++i){
 
    for (j = i + 1; j < n; ++j){
 
      if (numbers[i] > numbers[j]){
 
        a =  numbers[i];
        numbers[i] = numbers[j];
        numbers[j] = a;
 
      }
    }
  }

  return numbers;

}

float median (float numbers []){

  

  if ( n % 2 == 0)
    return (numbers[n/2] + numbers[n/2+1])/2.0;
  else
    return numbers[n/2 + 1];

}

float Q1 (float numbers[]){

  int n = sizeof(numbers)/sizeof(float);
  return numbers[round((n + 1)/4)]; 

}

float Q3 (float numbers[]){

  int n = sizeof(numbers)/sizeof(float);
  return numbers[round(3*(n + 1)/4)]; 

}


float IQR (float Q1, float Q3){

  return Q3-Q1;
}


float upperFence (float IQR, float Q3){

  return Q3 + (1.5 * IQR);
}

float lowerFence (float IQR, float Q1){

  return Q1 - (1.5 * IQR);
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
