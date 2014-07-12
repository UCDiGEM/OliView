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

elapsedMicros usec = 0;

//---------------------------------------------------------------------------------Instructions

String inStruct;  // Main instructions from USB
String twoStruct;
String threeStruct;
String fourStruct;
String fiveStruct;
String sixStruct;

double value = 0; // ADC reading value
float aRef = 2.048; // Analog Reference
float aRefMid = aRef/2;
float DACaRef = 3.3;
float DACaRefMid = aRef*2047.5/DACaRef;
float DCoffset = -0.00425;    //measured analytically

//---------------------------------------------------------------------------------Setup

void setup() {

  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(readPin, INPUT);
  pinMode(outPin, OUTPUT);
  pinMode(sp4tOne, OUTPUT);
  pinMode(sp4tTwo, OUTPUT);

  analogWriteResolution(12);
  analogReadAveraging(32);
  analogReadRes(16);

  while (usec < 5000);
  usec = usec - 5000;
}

//---------------------------------------------------------------------------------Main Loop

void loop() {

  analogWrite(A14, DACaRefMid);                            //Maintain virtual zero at electrode.

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
  // Example Instruction "anoStrip0.101.002001,"
  //     Parsed into 
  //        float  ASstartVolt  
  //        float  ASpeakVolt
  //        float  ASscanRate 
  //        int    ASwaveType
  //

  if (inStruct.startsWith("anoStrip")) {
    const char * ASSVarray = twoStruct.c_str();
    ASstartVolt = atof(ASSVarray);
    const char * ASPVarray = threeStruct.c_str();
    ASpeakVolt = atof(ASPVarray);
    const char * ASSRarray = fourStruct.c_str();
    ASscanRate = atof(ASSRarray);
    ASwaveType = fiveStruct.toInt();
    ASiterations = sixStruct.toInt();

    anoStrip();
  }

  //---------------------------------------------------------------------------------Anodic Stripping
  // Example Instruction "cycVolt0.101.002001,"
  //     Parsed into 
  //        float  CVstartVolt  
  //        float  CVpeakVolt
  //        float  CVscanRate
  //        int    CVwaveType 
  //

  if (inStruct.startsWith("cycVolt")) {

    const char * CVSTarray = twoStruct.c_str();
    CVstartVolt = atof(CVSTarray);
    const char * CVPVarray = threeStruct.c_str();
    CVpeakVolt = atof(CVPVarray);
    const char * CVSRarray = fourStruct.c_str();
    CVscanRate = atof(CVSRarray);
    CVwaveType = fiveStruct.toInt();

    cycVolt();
  }

  //---------------------------------------------------------------------------------Parsing Potentiostatic Amperometry
  // Example Instruction "potAmpero1.001.00,"
  //      Sampling Time (1.00 seconds)    PAsampTime  float
  //      Potential Voltage (1.00 Volts)  PApotVolt   float 
  //
  if (inStruct.startsWith("potAmpero")) {
    PAsampTime = twoStruct.toFloat();
    PApotVolt = threeStruct.toFloat();
    potAmpero();
  }

  //---------------------------------------------------------------------------------Parsing Resolution Adjustment
  // Exmaple Instruction "resolutionA" or "resolutionB"
  //      A:  +/- 10 uA
  //      B:  +/- 1000 nA  
  //      C:  +/- 100 nA
  //      D:  +/- 10 nA
  //

  if (inStruct.startsWith("resolution")) {
    gain = twoStruct.toFloat();
    zeroInstructions();

  }

  if (inStruct.startsWith("changeSampleRate")) {
    sampleRateFloat = twoStruct.toFloat();
    samplingDelay = (int)(1000000.0/sampleRateFloat);             //(value in µs) >> 1/samplingDelay = Sampling Rate
    samplingDelayFloat = 1000000.0/sampleRateFloat;    //(value in µs) >> 1/samplingDelay = Sampling Rate
    zeroInstructions();
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
  int samples = round(sampTime * sampleRateFloat); // With delay of 0.5 ms, 2000 samples per second

  Serial.println(samples);                                            //samples
  double voltDiv = scanRate/(1000.0*sampleRateFloat);
  int flipSample = round(samples/2+0.5);
  Serial.println(voltDiv,6);   //voltDiv
  Serial.println(flipSample);
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
      for (int j = 0; j < iterations; j++) {      
        val3 = DACaRefMid + (startVolt)/DACaRef*4095.0;
        for (int16_t i = 0;  i < flipSample; i++) {

          analogWrite(A14, (int)val3);
          val3 += 4095.0*scanRate/(1000.0*sampleRateFloat*DACaRef);

          while (usec < samplingDelay/2); // wait
          usec = usec - samplingDelay/2;

          value = analogRead(readPin);                  // analog read == # out of 2^16
          Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRef)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
          while (usec < samplingDelay/2); // wait
          usec = usec - samplingDelay/2;
        }
        for (int16_t i = 0; i < flipSample; i++) {

          val3 -= 4095.0*scanRate/(1000.0*sampleRateFloat*DACaRef);
          analogWrite(A14, (int)val3);

          while (usec < samplingDelay/2); // wait
          usec = usec - samplingDelay/2;

          value = analogRead(readPin);                  // analog read == # out of 2^16
          Serial.println(((value * aRef / 65535.0-aRef/2+ DCoffset)/aRef)*gain, 6);    // ratio, value/2^16, is the percent of ADC reference... * aRef (ADC Reference Voltage) == Voltage measured
          while (usec < samplingDelay/2); // wait
          usec = usec - samplingDelay/2;
        }
      }
      analogWrite(A14, DACaRefMid);
    }
    break;
  }

}






