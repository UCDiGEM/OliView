/*
          Code Developed by the 2014 UC Davis iGEM team (with the help of many examples)
 */
//---------------------------------------------------------------------------------Function Specific Variables
// Anodic Stripping
float ASstartVolt;    // Value delivered from QT instructions
float ASpeakVolt;     // Value delivered from QT instructions
float ASscanRate;     // Value delivered from QT instructions
float ASsampTime;     // Calculated from instructions (if/else requires global variable)
int ASwaveType;       // Value delivered from QT instructions
int ASiterations;

// Cyclic Voltammetry
float CVstartVolt;    // Value delivered from QT instructions
float CVpeakVolt;     // Value delivered from QT instructions
float CVscanRate;     // Value delivered from QT instructions
int CVwaveType;

// Potentiostatic Amperometry 
float PAsampTime;     // Value delivered from QT instructions
float PApotVolt;      // Value delivered from QT instructions

// Changing Resolution
float gain;

// Changing Sampling Speed
float sampleRateFloat;
int sampleRate;
int samplingDelay;             //(value in µs) >> 1/samplingDelay = Sampling Rate
float samplingDelayFloat;    //(value in µs) >> 1/samplingDelay = Sampling Rate

//---------------------------------------------------------------------------------Pin Assignments

const int readPin = A2;  // Main Analog Input
const int outPin = A14;
const int sp4tOne = 11;  // Resolution Switch 1
const int sp4tTwo = 10;  // Resolution Switch 2
const int ledPin = 13;  //LED pin
const int refCounterShortSwitch = 9;      //in2
const int workingElectrodeSwitch = 8;     //in4
const int counterElectrodeSwitch = 7;     //in1


elapsedMicros usec = 0;

//---------------------------------------------------------------------------------Instructions

String inStruct;  // Main instructions from USB
String twoStruct;
String threeStruct;
String fourStruct;
String fiveStruct;
String sixStruct;

double value = 0; // ADC reading value
double ref = 0; //Ref reading value
float aRef = 2.0505; // Analog Reference
float aRefMid = 1.025530;
float DACaRef = 3.3;
float DACaRefMid = (aRefMid+0.012)*4095.0/DACaRef;
float DACoff = aRef * 4095.0 / DACaRef;
float DCoffset = 0.0034;    //measured analytically

//---------------------------------------------------------------------------------Setup

//#include <ADC.h>
//ADC *adc = new ADC(); // adc object

void setup() {

  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(readPin, INPUT);
  pinMode(outPin, OUTPUT);
  pinMode(sp4tOne, OUTPUT);
  pinMode(sp4tTwo, OUTPUT);
  pinMode(refCounterShortSwitch, OUTPUT);      
  pinMode(workingElectrodeSwitch, OUTPUT);
  pinMode(counterElectrodeSwitch, OUTPUT);

  analogWriteResolution(12);
  analogReadAveraging(32);
  analogReadRes(16);

  while (usec < 5000);
  usec = usec - 5000;
}

//---------------------------------------------------------------------------------Main Loop

void loop() {
  
  analogWrite(A14, DACoff);                            //Maintain almost (small voltage due to op-Amp offset) zero at electrode.
  digitalWrite(refCounterShortSwitch, HIGH);

  if (Serial.available()) {
    // Interprets commands from computer
    // All commands must be terminated with a comma ","

    inStruct = Serial.readStringUntil('!');
    twoStruct = Serial.readStringUntil('@');
    threeStruct = Serial.readStringUntil('#');
    fourStruct = Serial.readStringUntil('$');
    fiveStruct = Serial.readStringUntil('%');
    sixStruct = Serial.readStringUntil('^');
  }

  //---------------------------------------------------------------------------------Anodic Stripping

  if (inStruct.startsWith("anoStrip")) {
    ASstartVolt = twoStruct.toFloat();
    ASpeakVolt = threeStruct.toFloat();
    ASscanRate = fourStruct.toFloat();
    ASwaveType = fiveStruct.toInt();
    ASiterations = sixStruct.toInt();

    anoStrip();
  }

  //---------------------------------------------------------------------------------Anodic Stripping

  if (inStruct.startsWith("cycVolt")) {
    CVstartVolt = twoStruct.toFloat();
    CVpeakVolt = threeStruct.toFloat();
    CVscanRate = fourStruct.toFloat();
    CVwaveType = fiveStruct.toInt();

    cycVolt();
  }

  //---------------------------------------------------------------------------------Parsing Potentiostatic Amperometry
  if (inStruct.startsWith("potAmpero")) {
    PAsampTime = twoStruct.toFloat();
    PApotVolt = threeStruct.toFloat();
    potAmpero();
  }

  //---------------------------------------------------------------------------------Parsing Resolution Adjustment
  // Exmaple Instruction "resolution1" or "resolution2"
  //      A:  +/- 10 uA
  //      B:  +/- 1000 nA  
  //      C:  +/- 100 nA
  //      D:  +/- 10 nA
  //

  if (inStruct.startsWith("resolution")) {
    int gainBuf = twoStruct.toInt();
    switch (gainBuf){
      case (1): 
      {
        gain = 10.0;
        digitalWrite(sp4tOne, LOW);
        digitalWrite(sp4tTwo, LOW);
      }
      break;
      case (2): 
      {
        gain = 1000.0;
        digitalWrite(sp4tOne, HIGH);
        digitalWrite(sp4tTwo, LOW);
      }
      break;
      case (3): 
      {
        gain = 100.0;
        digitalWrite(sp4tOne, LOW);
        digitalWrite(sp4tTwo, HIGH);
      }
      break;
      case (4): 
      {
        gain = 10.0;
        digitalWrite(sp4tOne, HIGH);
        digitalWrite(sp4tTwo, HIGH);
      }
      break;
    }
    zeroInstructions();

  }

  if (inStruct.startsWith("changeSampleRate")) {
    sampleRateFloat = twoStruct.toFloat();
    samplingDelay = (int)(1000000.0/sampleRateFloat);             //(value in µs) >> 1/samplingDelay = Sampling Rate
    samplingDelayFloat = 1000000.0/sampleRateFloat;    //(value in µs) >> 1/samplingDelay = Sampling Rate
    zeroInstructions();
  }
  if (usec > 5000) {
    usec = 0;
  }
}

void zeroInstructions() {
  inStruct = "";
  twoStruct = "";
  threeStruct = "";
  fourStruct = "";
  fiveStruct = "";
  sixStruct = "";
}

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
  sample(CVsampTime, CVwaveType, CVstartVolt, CVpeakVolt, CVscanRate, 1);

  zeroInstructions();
}

//---------------------------------------------------------------------------------Potentiostatic Amperometry
// Example Instruction "potAmpero1.001.00"
//      Sampling Time     (1.00 seconds) AsampTime  float
//      Potential Voltage (1.00 Volts)   PApotVolt    float 
//
void potAmpero() {
  sample(PAsampTime, 0, 0, PApotVolt, 0, 1);
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

  sample(ASsampTime, ASwaveType, ASstartVolt, ASpeakVolt, ASscanRate, ASiterations);
  zeroInstructions();
}
//---------------------------------------------------------------------------------Sampling Loop
//  Inputs:
//      float sampTime
//      int   waveType
//      float startVolt
//      float endVolt (or peakVolt)
//

void sample(float sampTime, int waveType, float startVolt, float endVolt, float scanRate, int iterations) {
  int32_t samples = round(sampTime * sampleRateFloat); // With delay of 0.5 ms, 2000 samples per second

  digitalWrite(refCounterShortSwitch, LOW);      
  digitalWrite(workingElectrodeSwitch, HIGH);
  digitalWrite(counterElectrodeSwitch, HIGH);

  elapsedMicros timer = 0;

  DACaRefMid = aRefMid*4095.0/DACaRef;

  Serial.println(samples);                                            //samples
  double voltDiv = scanRate/(1000.0*sampleRateFloat);
  int32_t flipSample = round(samples/2);
  Serial.println(voltDiv,6);   //voltDiv
  Serial.println(flipSample);

  while (timer < 20); // wait
  timer = timer - 20;

  switch (waveType) {
    //---------------------------------------------------------------------------------Constant Potential
    float val, val2, val3;

    case (0):
    {

      digitalWrite(ledPin, HIGH);
      val = DACaRefMid + (endVolt) * 4095.0 / DACaRef;
      analogWrite(A14, (int)val);

      while (timer < samplingDelay/2); // wait
      timer = timer - samplingDelay/2;

      for (int32_t i = 0; i < samples; i++) {

        value = analogRead(readPin);                  // analog read == # out of 2^16


        Serial.println((((value) * aRef / 65535.0 - aRefMid + DCoffset)/aRefMid)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
        while (timer < samplingDelay); // wait
        timer = timer - samplingDelay;
      }

      digitalWrite(ledPin, LOW);
    }

    break;
    //---------------------------------------------------------------------------------Sine Wave
    case (1):
    {

      digitalWrite(ledPin, HIGH);
      for (int32_t i = 0; i < samples; i++) {

        val2 = DACaRefMid + sin(phase) * endVolt*4095.0/DACaRef;
        analogWrite(A14, (int)val2);
        phase = phase + 100/sampleRateFloat;

        while (timer < samplingDelay/2); // wait
        timer = timer - samplingDelay/2;

        value = analogRead(readPin);                  // analog read == # out of 2^16
        //Serial.println(value * aRef / 65535.0, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
        Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRefMid)*gain, 6);    // 
        if (phase >= twopi) phase = 0;
        while (timer < samplingDelay/2); // wait
        timer = timer - samplingDelay/2;
      }
      phase = 0.0;
      digitalWrite(ledPin, LOW);
    }

    break;
    //---------------------------------------------------------------------------------Triangle Wave
    // Consider the range of the DAC >> 2.048/4096 = 500 uV (range of DAC = 4096 values (0-4095))
    //    500 uV == val3 == 1
    //
    //
    case (2): // triangle wave
    {
      int j = 0;

      while (j < iterations) {
        digitalWrite(ledPin, HIGH);
        j++;
        double voltMark = startVolt;      
        val3 = DACaRefMid + (startVolt)/DACaRef*4095.0;
        for (int32_t i = 0;  i < flipSample; i++) {

          analogWrite(A14, (int)val3);
          val3 += 4095.0*scanRate/(1000.0*sampleRateFloat*DACaRef);

          while (timer < samplingDelay/2); // wait
          timer = timer - samplingDelay/2;

          value = analogRead(readPin);                  // analog read == # out of 2^16
          Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRefMid)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
          Serial.println(voltMark, 6);
          voltMark += scanRate/(1000.0*sampleRateFloat);

          while (timer < samplingDelay/2); // wait
          timer = timer - samplingDelay/2;


        }

        for (int32_t i = 0;  i < flipSample; i++) {

          analogWrite(A14, (int)val3);
          val3 -= 4095.0*scanRate/(1000.0*sampleRateFloat*DACaRef);

          while (timer < samplingDelay/2); // wait
          timer = timer - samplingDelay/2;

          value = analogRead(readPin);                  // analog read == # out of 2^16
          Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRefMid)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
          Serial.println(voltMark, 6);
          voltMark -= scanRate/(1000.0*sampleRateFloat);

          while (timer < samplingDelay/2); // wait
          timer = timer - samplingDelay/2;
        }

      }
      digitalWrite(ledPin, LOW);
    }

    break;
    //--------------------------------------------------------------------------------------------------Continuous Sampling
    case (3): 
    {
    }
    break;

  }
  analogWrite(A14, DACoff);
  digitalWrite(refCounterShortSwitch, HIGH);      
  digitalWrite(workingElectrodeSwitch, LOW);
  digitalWrite(counterElectrodeSwitch, LOW);
}









