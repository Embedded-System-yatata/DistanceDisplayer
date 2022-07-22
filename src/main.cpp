#include <Arduino.h>
#include <math.h>


#define pinSonarTriger 2
#define pinSonarEcho 3
#define pinButtonMeasure 5

#define ZSCORELIMIT 2.5

int prevState_ButtonMeasure = 0;

float speedWave = 343;
int nMeasures = 50;


long durationMS; // variable for the duration of sound wave travel
double distance; // variable for the distance measurement



typedef struct DynamicArray{

  float *data;
  int size = 0;

}dynamicArray;


DynamicArray *new_Array(){

  DynamicArray *novo = (DynamicArray*) malloc(sizeof(DynamicArray));
  novo->data = (float*) malloc(sizeof(float) * nMeasures);
  novo->size = nMeasures;
  return novo;

}

void delete_Array(DynamicArray *array){

  free(array);

}



DynamicArray *measureArrayPtr;





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


//TODO determinar size (elementos) do array
float average(DynamicArray *array){

  //int n = sizeof(numbers)/sizeof(numbers[0]);
  float av = 0;
  /*Serial.println(numbers[5]);
  Serial.println(sizeof(numbers));
  Serial.println(sizeof(float));
  Serial.println(n);
  */
  int i;
  for(i=0; i < array->size; i++){
    av += array->data[i];
  }

  return av/array->size;
}

float median (float *numbers){

  int n = sizeof(numbers)/sizeof(float);

  if ( n % 2 == 0)
    return (numbers[n/2] + numbers[n/2+1])/2.0;
  else
    return numbers[n/2 + 1];

}

float standard_Deviation(DynamicArray *array, float av){

  float SD = 0;
  int i;

  for (i = 0; i < array->size; i++) {
        SD += pow(array->data[i] - av, 2);
    }
    return sqrt(SD / array->size);

}


void swap(float* xp, float* yp)
{
    float temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void orderAsc(DynamicArray *array){

  int i, j, min_idx;
 
    // One by one move boundary of unsorted subarray
    for (i = 0; i < array->size - 1; i++) {
 
        // Find the minimum element in unsorted array
        min_idx = i;
        for (j = i + 1; j < array->size; j++)
            if (array->data[j] < array->data[min_idx])
                min_idx = j;
 
        // Swap the found minimum element
        // with the first element
        swap(&array->data[min_idx], &array->data[i]);
    }

}


float Q1 (DynamicArray *array){

  //int n = sizeof(numbers)/sizeof(float);
  return array->data[round((array->size + 1)/4)]; 

}

float Q3 (DynamicArray *array){

  //int n = sizeof(numbers)/sizeof(float);
  return array->data[round(3*(array->size + 1)/4)]; 

}

float IQR (DynamicArray *array){


  return Q3(array)-Q1(array);
}

float upperFence (DynamicArray *array){

  return (Q3(array) + (1.5 * IQR(array)));
}

float lowerFence (DynamicArray *array){

  return (Q1(array) - (1.5 * IQR(array)));
}



void arrayDynamicRemove (DynamicArray *array, int index){


  int i;
  for(i = index; i < array->size - 1; i++){
    array->data[i] = array->data[i + 1];
  }


  realloc(array->data, (array->size - 1) * sizeof(float));
  array->size -= 1;
  

}

float *zScores(float *z, DynamicArray *array, float av, float sd){

  int i;
  for (i = 0; i < array->size; i++){
    z[i] = (array->data[i]-av)/sd;
  }

  return z;
}


DynamicArray *removeOutliers(DynamicArray *array, float upFence, float downFence){

  float av = average(array);
  float *z;
  z = (float*) malloc(array->size * sizeof(float));
  z = zScores(z, array, av, standard_Deviation(array, av));

  int i;
  Serial.print("\nZ-Scores: ");
  for (i = 0; i < array->size; i++){
    Serial.print(z[i]);
    Serial.print(" ");
  }

  for (i = 0; i < array->size; i++){

    if (array->data[i] > upFence || array->data [i] < downFence || z[i] > ZSCORELIMIT || z[i] < -ZSCORELIMIT){
      //TODO remover elemento
      
      Serial.print("\n Remove ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(array->data[i]);
      Serial.print("  ");
      Serial.print(array->data[i] > upFence);
      Serial.print(array->data [i] < downFence);
      Serial.print(z[i] > 2);
      Serial.print(z[i] < -2);
      arrayDynamicRemove(array,i);
    }
  }



  return array;
}


float measure (DynamicArray *array){


  int i;
  for (i=0; i < array->size; i++){

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
    array->data[i] = durationMS*pow(10,-6) * speedWave / 2; // Speed of sound wave divided by 2 (go and back)
    Serial.print(array->data[i]);
    Serial.print(" ");

  }

  orderAsc(measureArrayPtr);

  Serial.print("\n\n");
  
  for(i = 0; i < array->size; i++){
    Serial.print(array->data[i]);
    Serial.print(" ");
  }



  //outliers id.
  Serial.print("\nQ1: ");
  Serial.println(Q1(array));
  Serial.print("Q3: ");
  Serial.println(Q3(array));
  Serial.print("ISR: ");
  Serial.println(IQR(array));
  Serial.print("Up Fence: ");
  Serial.println(upperFence(array));
  Serial.print("Low Fence: ");
  Serial.println(lowerFence(array));

 //average
  removeOutliers(array, upperFence(array), lowerFence(array));
  Serial.print("\n");
  for(i = 0; i < array->size; i++){
    Serial.print(array->data[i]);
    Serial.print(" ");
  }

  return average(array);
}




void setup() {
  pinMode(pinSonarTriger, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(pinSonarEcho, INPUT); // Sets the echoPin as an INPUT
  pinMode(pinButtonMeasure, INPUT);
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");

}




void loop() {
  
  //Serial.printf("%0.8f",float(1/pow(durationMS,6)));
  //distance = durationMS*0.034/2;
  // Displays the distance on the Serial Monitor
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println("m");

  if (digitalRead(pinButtonMeasure) == true && prevState_ButtonMeasure == 0){

    //measureArrayPtr = (float*) malloc(nMeasures * sizeof(float));
    measureArrayPtr = new_Array();
    
    if(measureArrayPtr == NULL) {
      Serial.println("Error! memory not allocated.");
      exit(0);
    }

    prevState_ButtonMeasure = 1;
    
    Serial.print("\n");
    Serial.print(measure(measureArrayPtr));
    Serial.println(" = Final Measure");
    delete_Array(measureArrayPtr);

  }else if (digitalRead(pinButtonMeasure) == false){
    prevState_ButtonMeasure = 0;
  }


}
