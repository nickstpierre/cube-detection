/*
    Code found on https://www.norwegiancreations.com/2017/08/what-is-fft-and-how-can-you-implement-it-on-an-arduino/. Code has been altered for W&L's Capstone Project.

    Author: Nick St. Pierre
    Filename: EMFP6_v1
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

double vReal[SAMPLES];
double vImag[SAMPLES]; 

void setup() {
    Serial.begin(115200);
    
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
    Serial.println(peak);     //Print out what frequency is the most dominant.
 
    for(int i=0; i<(SAMPLES/2); i++)
    {
        /*View all these three lines in serial terminal to see which frequencies has which amplitudes*/
         
        //Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
        //Serial.print(" ");
        //Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
    }
    
    //delay(500);  //Repeat the process every second OR:
    while(1);       //Run code once
}

double areaUnderCurve(firstBin, lastBin){
    double totalArea = 0.0;
    double tempArea;
    int totalBins = lastBin - firstBin;
    for(int i=0; i<totalBins; i++){
      tempArea = vReal[i];
      totalArea += tempArea;
    }
    return totalArea;
}
