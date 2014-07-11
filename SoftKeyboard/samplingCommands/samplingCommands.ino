/*
    Code Developed by the 2014 UC Davis iGEM team (with the help of many examples)
 */


// Sine Function 
float phase = 0.0;
float twopi = 3.14159 * 2;

// Cyclic Voltametry
//---------------------------------------------------------------------------------Cyclic Voltammetry
// Example Instruction "cycVolt0.101.001002"
//      Start Volt (0.10 Volts)    CVstartVolt  float          <<Notes are incorrect, need rewrit
//      Peak Volt  (1.00 Volts)    CVpeakVolt   float
//      Scan Rate  (100 mV/S)      CVscanRate   float
//      Wave Type (  0 - constant  )            int
//                   1 - sin wave
//                   2 - triangle wave
//  
void cycVolt() {
  float CVsampTime = 2000*(CVpeakVolt - CVstartVolt)/CVscanRate;
  sample(CVsampTime, CVwaveType, CVstartVolt, CVpeakVolt, CVscanRate);

  zeroInstructions();
}

//---------------------------------------------------------------------------------Potentiostatic Amperometry
// Example Instruction "potAmpero1.001.00"
//      Sampling Time     (1.00 seconds) AsampTime  float
//      Potential Voltage (1.00 Volts)   PApotVolt    float 
//
void potAmpero() {
  sample(PAsampTime, 0, 0, PApotVolt, 0);
  zeroInstructions();
}

//---------------------------------------------------------------------------------Anodic Stripping
// Example Instruction "anoStrip0.101.005002"
//      Start Volt (0.10 Volts)    ASstartVolt  float
//      Peak Volt  (1.00 Volts)    ASpeakVolt   float
//      Scan Rate (100 mV/S)       ASscanRate   float
//      Wave Type (  0 - constant  )            int
//                   1 - sin wave
//                   2 - triangle wave
//
void anoStrip() {
  if (ASwaveType == 2) {
    ASsampTime = 2000*(ASpeakVolt - ASstartVolt)/ ASscanRate;
  }
  else {
    ASsampTime = 1000*(ASpeakVolt - ASstartVolt)/ ASscanRate;
  }  

  sample(ASsampTime, ASwaveType, ASstartVolt, ASpeakVolt, ASscanRate);
  zeroInstructions();
}
//---------------------------------------------------------------------------------Sampling Loop
//  Inputs:
//      float sampTime
//      int   waveType
//      float startVolt
//      float endVolt (or peakVolt)
//

void sample(float sampTime, int waveType, float startVolt, float endVolt, float scanRate) {
  int samples = round(sampTime * sampleRateFloat); // With delay of 0.5 ms, 2000 samples per second

  Serial.println(samples);                                            //samples
  double voltDiv = scanRate/(1000.0*sampleRateFloat);
  Serial.println(voltDiv,6);   //voltDiv
  Serial.println(startVolt,6);
  while (usec < 20); // wait
  usec = usec - 20;


  //elapsedMicros usec = 0;
  switch (waveType) {
    //---------------------------------------------------------------------------------Constant Potential
    float val, val2, val3;

    case (0):
    {
      val = DACaRefMid + (endVolt) * 4095.0 / DACaRef;
      analogWrite(A14, (int)val);

      while (usec < samplingDelay/2); // wait
      usec = usec - samplingDelay/2;

      for (int16_t i = 0; i < samples; i++) {
        value = analogRead(readPin);                  // analog read == # out of 2^16
        Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRef)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;
      }

      analogWrite(A14, DACaRefMid);
    }

    break;
    //---------------------------------------------------------------------------------Sine Wave
    case (1):
    {
      for (int16_t i = 0; i < samples; i++) {

        val2 = DACaRefMid + sin(phase) * endVolt*4095.0/DACaRef;
        analogWrite(A14, (int)val2);
        phase = phase + 100/sampleRateFloat;

        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;

        value = analogRead(readPin);                  // analog read == # out of 2^16
        //Serial.println(value * aRef / 65535.0, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
        Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRef)*gain, 6);    // 
        if (phase >= twopi) phase = 0;
        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;
      }
      phase = 0.0;
      analogWrite(A14, DACaRefMid);
    }

    break;
    //---------------------------------------------------------------------------------Triangle Wave
    // Consider the range of the DAC >> 2.048/4096 = 500 uV (range of DAC = 4096 values (0-4095))
    //    500 uV == val3 == 1
    //
    //
    case (2): // triangle wave
    {

      val3 = DACaRefMid + (startVolt)/DACaRef*4095.0;
      for (int16_t i = 0;  i < round(samples/2); i++) {

        analogWrite(A14, (int)val3);
        val3 += 4095.0*scanRate/(1000.0*sampleRateFloat*DACaRef);

        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;

        value = analogRead(readPin);                  // analog read == # out of 2^16
        Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRef)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;
      }
      for (int16_t i = 0; i < round(samples/2); i++) {

        val3 -= 4095.0*scanRate/(1000.0*sampleRateFloat*DACaRef);
        analogWrite(A14, (int)val3);

        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;

        value = analogRead(readPin);                  // analog read == # out of 2^16
        Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRef)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
        while (usec < samplingDelay/2); // wait
        usec = usec - samplingDelay/2;
      }
      analogWrite(A14, DACaRefMid);
    }
    break;
  }

}












