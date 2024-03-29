#include <Arduino.h>
#include <TM1637Display.h>
#include <math.h>
#include <stdio.h>

#define pinSonarTriger 2
#define pinSonarEcho 3
#define pinButtonMeasure 5
#define pinCLK 9
#define pinDIO 10


#define DECIMAL_PLACES 2
#define ZSCORELIMIT 2.5
#define DISPLAY_INIT_DELAY 1500
#define DISPLAY_DELAY   3500
#define N_MEASURES 50
#define SLEEP_TIME 100


float speedWave = 343;


int prevState_ButtonMeasure = 0;


long durationMS; // variable for the duration of sound wave travel
double distance; // variable for the distance measurement

typedef struct DynamicArray{

  float *data;
  int size = 0;

}dynamicArray;


DynamicArray *new_Array(){

  DynamicArray *novo = (DynamicArray*) malloc(sizeof(DynamicArray));
  novo->data = (float*) malloc(sizeof(float) * N_MEASURES);
  novo->size = N_MEASURES;
  return novo;

}

void reset_Array (DynamicArray *array){

  array->data = (float*) realloc(array->data, sizeof(float) * N_MEASURES);
  array->size = N_MEASURES;

}

void delete_Array(DynamicArray *array){

  free(array);

}



DynamicArray *measureArrayPtr;




 
const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};
 
TM1637Display display(pinCLK, pinDIO);

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };




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
  z = (float*) malloc(N_MEASURES * sizeof(float));
  z = zScores(z, array, av, standard_Deviation(array, av));

  int i;
  Serial.print("\nZ-Scores: ");
  for (i = 0; i < N_MEASURES; i++){
    Serial.print(z[i],5);
    Serial.print(" ");
  }

  for (i = 0; i < array->size; i++){

    if (array->data[i] > upFence || array->data [i] < downFence || z[i] > ZSCORELIMIT || z[i] < -ZSCORELIMIT){

      
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

  free(z);

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

  return float(average(array));
}



void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
 
// Converts a given integer x to string str[].
// d is the number of digits required in the output.
// If d is more than the number of digits in x,
// then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x) {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
 
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';
 
    reverse(str, i);
    str[i] = '\0';
    return i;
}
 
// Converts a floating-point/double number to a string.
void float_To_String(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    float fpart = n - (float)ipart;
 
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
    Serial.print("\nintToString i: ");
    Serial.println(i);
    // check for display option after point
    if (afterpoint != 0) {
      if(i == 0){
        res[i] = '0'; // add dot
        res[i+1] = '.';

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
 
        intToStr((int)fpart, res + i + 2, afterpoint);

      }else{
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter
        // is needed to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);
 
        intToStr((int)fpart, res + i + 1, afterpoint);
      }
        
    }
}





void display_Init(){

  display.setBrightness(0x0f);
  display.setSegments(data);
  delay(DISPLAY_INIT_DELAY);
  display.setSegments(blank);
  data[0] = 0x00;
}

void displayMeasure (int a, int b, int c){


  
  data[1] = display.encodeDigit(a);
  data[2] = display.encodeDigit(b);
  data[3] = display.encodeDigit(c);
  display.setSegments(data);
  delay(DISPLAY_DELAY);
  display.setSegments(blank);

}



void setup() {
  pinMode(pinSonarTriger, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(pinSonarEcho, INPUT); // Sets the echoPin as an INPUT
  pinMode(pinButtonMeasure, INPUT);
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");

  display_Init();

  measureArrayPtr = new_Array();

  if(measureArrayPtr == NULL) {
    Serial.println("Error! memory not allocated.");
    exit(0);
  }
}




void loop() {
  
  //Serial.printf("%0.8f",float(1/pow(durationMS,6)));
  //distance = durationMS*0.034/2;
  // Displays the distance on the Serial Monitor
  //Serial.print("Distance: ");
  //Serial.print(distance);
  //Serial.println("m");

  if (digitalRead(pinButtonMeasure) == true && prevState_ButtonMeasure == 0){

    //measureArrayPtr = (float*) malloc(N_MEASURES * sizeof(float));
    
    
    

    prevState_ButtonMeasure = 1;
    
    float result = measure(measureArrayPtr);
    Serial.print("\nFinal Measure = ");
    Serial.println(result,5);
    Serial.println(float(round(result * pow(10, DECIMAL_PLACES)))/pow(10, DECIMAL_PLACES),5);
    char word[DECIMAL_PLACES+1];
    
    float_To_String(float(round(result * pow(10, DECIMAL_PLACES)))/pow(10, DECIMAL_PLACES), word, DECIMAL_PLACES+1);
    
    Serial.print("\nWord: ");
    Serial.print(word);
    displayMeasure(int(word[0]), int(word[2]), int(word[3]));


    reset_Array(measureArrayPtr);

  }else if (digitalRead(pinButtonMeasure) == false && prevState_ButtonMeasure != 0){
    prevState_ButtonMeasure = 0;
  }

  sleep(SLEEP_TIME);

}
