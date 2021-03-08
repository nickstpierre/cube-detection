/*
    Code found on https://www.norwegiancreations.com/2017/08/what-is-fft-and-how-can-you-implement-it-on-an-arduino/. Code has been altered for W&L's Capstone Project.

    Author: Nick St. Pierre
    Filename: EMFP7_v3
    Description: This program is designed to perform Fast Fourier Transform analysis on live data coming
    from a signal generator to dial in the code and understand arduinoFFT library. \

    Version 3: Now capable to perform FFT analysis for two sensor inputs.

*/


#include "arduinoFFT.h"
 
#define SAMPLES 256             //Must be a power of 2
#define SAMPLING_FREQUENCY 500   //Hz, must be less than 10000 due to ADC
#define HWSERIAL Serial1

arduinoFFT FFT = arduinoFFT();
arduinoFFT FFT2 = arduinoFFT();
 
unsigned int sampling_period_us;
unsigned long microseconds;

/*  Set up the pin I/O and FFT analysis bins for 2nd 10.3 Hz EMF Detection Sensor */
int pinInput1 = A0;
int pinOut1 = 12;
double vReal[SAMPLES];
double vImag[SAMPLES]; 

/*  Set up the pin I/O and FFT analysis bins for 2nd 10.3 Hz EMF Detection Sensor  */
int pinInput2 = A1;
int pinOut2 = 13;
double vReal2[SAMPLES];
double vImag2[SAMPLES];

void setup() {
    HWSERIAL.begin(115200);
    pinMode(pinOut1, OUTPUT); //Output pin 1 to the RasPi. This will have a value between 0-255 sent to RasPi.
    pinMode(pinOut2, OUTPUT); //Output pin 2 to RasPi. Will output 0-255 from analogWrite of EMF sensitivity.
    sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
}
 
void loop() {
   
    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
        microseconds = micros();    //Overflows after around 70 minutes!
     
        vReal[i] = analogRead(pinInput1); //Reads the frequency values from the first sensor
        vImag[i] = 0;

        vReal2[i] = analogRead(pinInput2); //Reads the frequency values from the second sensor
        vImag2[i] = 0;
     
        while(micros() < (microseconds + sampling_period_us)){
        }
    }
 
    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT2.Windowing(vReal2, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT2.Compute(vReal2, vImag2, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    FFT2.ComplexToMagnitude(vReal2, vImag2, SAMPLES);
    
    double peak1 = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
    double peak2 = FFT.MajorPeak(vReal2, SAMPLES, SAMPLING_FREQUENCY);
 
    /*PRINT RESULTS*/
    HWSERIAL.print("Peak Frequency Sensor 1: ");
    HWSERIAL.println(peak1); //Prints out the peak frequency of sensor 1

    HWSERIAL.print("Peak Frequency Sensor 2: ");
    HWSERIAL.println(peak2); //Prints out the peak frequency of sensor 2
    
    double sensitivity1 = 100*(areaUnderCurve(5,7,vReal)/areaUnderCurve(2,11, vReal)); //Calculates the ratio of the magnitude around 10.3Hz/magnitude between 3-14 Hz
    double sensitivity2 = 100*(areaUnderCurve(5,7,vReal2)/areaUnderCurve(2,11, vReal2)); //Calculates the ratio of the magnitude around 10.3Hz/magnitude between 3-14 Hz

    //Maps out the areaUnderCurve ratio from 0 to 255 
    int senseVal1 = map(sensitivity1, 0, 85, 0, 255);
    int senseVal2 = map(sensitivity2, 0, 85, 0, 255);

    // Prints out the sensitivity value at current sensor placement
    HWSERIAL.print("Sensitivity 1: ");
    HWSERIAL.println(senseVal1);
    
    HWSERIAL.print("Sensitivity 2: ");
    HWSERIAL.println(senseVal2);
    

    // SImple logic to send output to LED based off of threshold 
    if(senseVal > 0){
      analogWrite(pinOut1, senseVal1);
      analogWrite(pinOut2, senseVal2);
    }
    
    delay(20);  //Repeat the process every second OR:
    //while(1);       //Run code once
}

double areaUnderCurve(int firstBin, int lastBin, double val[]){
    double totalArea = 0.0; //Initializes the area for calculation
    for(int i=firstBin; i<lastBin; i++){
      totalArea += val[i]; //Add each dx value of the denoted bin location
    }
    return totalArea; //Return the total area of the desired bin range
}
