#include <Arduino.h>
#include <math.h>



#define pinSonarTriger 2
#define pinSonarEcho 3
#define pinButtonMeasure 5


int prevState_ButtonMeasure = 0;


float speedWave = 343;
int nMeasures = 50;


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

  //int n = sizeof(numbers)/sizeof(numbers[0]);
  float av = 0;
  /*Serial.println(numbers[5]);
  Serial.println(sizeof(numbers));
  Serial.println(sizeof(float));
  Serial.println(n);
  */
  int i;
  for(i=0; i < nMeasures; i++){
    av += numbers[i];
  }

  return av/nMeasures;
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

  int n = sizeof(numbers)/sizeof(float);

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
  pinMode(pinButtonMeasure, INPUT);
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
}




float measure (){

  float m = 0;
  float measures [nMeasures];


  int i;
  for (i=0; i < nMeasures; i++){

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
    measures[i] = durationMS*pow(10,-6) * speedWave / 2; // Speed of sound wave divided by 2 (go and back)
    //Serial.print(measures[i]);
    //Serial.print(" ");

  }


  //outliers id.
  //average


  return average(measures);
}


void loop() {
  
  //Serial.printf("%0.8f",float(1/pow(durationMS,6)));
  //distance = durationMS*0.034/2;
  // Displays the distance on the Serial Monitor
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println("m");

  if (digitalRead(pinButtonMeasure) == true && prevState_ButtonMeasure == 0){
    prevState_ButtonMeasure = 1;
    
    Serial.print(measure());
    Serial.println(" = Final Measure");
  }else if (digitalRead(pinButtonMeasure) == false){
    prevState_ButtonMeasure = 0;
  }


}
