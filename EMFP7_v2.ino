/*
    Code found on https://www.norwegiancreations.com/2017/08/what-is-fft-and-how-can-you-implement-it-on-an-arduino/. Code has been altered for W&L's Capstone Project.

    Author: Nick St. Pierre
    Filename: EMFP7_v2
    Description: This program is designed to perform Fast Fourier Transform analysis on live data coming
    from a signal generator to dial in the code and understand arduinoFFT library. 

*/


#include "arduinoFFT.h"
 
#define SAMPLES 256             //Must be a power of 2
#define SAMPLING_FREQUENCY 500   //Hz, must be less than 10000 due to ADC


arduinoFFT FFT = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;

int pinInput = A0;
int pinOut = A1;

double vReal[SAMPLES];
double vImag[SAMPLES]; 

void setup() {
    Serial.begin(115200);
    pinMode(pinOut, OUTPUT); //Output pin to the RasPi. This will increase the voltage output for a GPIO on RasPi.
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
}
 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(pinInput);
        vImag[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
 
    /*PRINT RESULTS*/
    if(peak < 12 && peak > 9){
      Serial.print("Correct Frequency: ");
      Serial.println(peak); //Prints out the peak frequency
    }

    double sensitivity = 100*(areaUnderCurve(5,7)/areaUnderCurve(2,11)); //Calculates the ratio of the magnitude around 10.3Hz/magnitude between 3-14 Hz

    //Maps out the areaUnderCurve ratio from 0 to 255 
    int senseVal = map(sensitivity, 0, 82, 0, 255);

    // Prints out the sensitivity value at current sensor placement
    Serial.print("Sensitivity: ");
    //Serial.println(sensitivity);
    Serial.println(senseVal);

    // SImple logic to send output to LED based off of threshold 
    if(senseVal > 63){
      digitalWrite(pinOut, HIGH);
    }
    else if (senseVal < 63){
      digitalWrite(pinOut, LOW);
    }
    
    delay(20);  //Repeat the process every second OR:
    //while(1);       //Run code once
}

double areaUnderCurve(int firstBin, int lastBin){
    double totalArea = 0.0; //Initializes the area for calculation
    for(int i=firstBin; i<lastBin; i++){
      totalArea += vReal[i]; //Add each dx value of the denoted bin location
    }
    return totalArea; //Return the total area of the desired bin range
}
