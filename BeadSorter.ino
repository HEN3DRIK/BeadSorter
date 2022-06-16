#include <Servo.h>
#include <AccelStepper.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

#define dirPin 2 //Stepper
#define stepPin 3 //Stepper
#define motorInterfaceType 1 //Stepper
#define stepperMulti 100 //Stepper

#define motorSpeed 255 //Container Motor
#define GSM2 5 // Container Motor
#define in3 7 //Container Motor
#define in4 6 //Container Motor

#define setupPin 11 //Setup
#define photoSensorPin A0 //Photo Sensor
#define nullScanOffset 200 

#define angle 15 //Servo

// Parameters: https://learn.adafruit.com/adafruit-color-sensors/program-it
//Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_1X);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);
//Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_101MS, TCS34725_GAIN_1X);

//Max allowed color difference = thresholdFactor ∗ colorValue + offset (0,08/40)
//dark colors 0.02 + 10
float thresholdFactor = 0.04;
int offset = 40;

Servo servo;
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, dirPin);

boolean autoSort = true;
int errorCounter = 0;
int maxErrorCounter = 2;
int resultColor[4] = {0, 0, 0, 0};
int medianColors[4][4];
int storedColors[16][4];
int tempStoredColors[16][4];

int photoSensor = 0;

boolean calibrateNullScan = true;
int nullScanValues[4] = {6618, 1860, 2282, 2116}; //adjust these if no calibration

int autoColorCounter = 0;
int beadCounter = 0;

const int dynamicContainerArraySize = 16;
int dynamicContainerArray[dynamicContainerArraySize] = { -1, 666, -1, -1, -1, 666, -1, -1, -1, 666, -1, -1, -1, 666 , -1 , -1};

void setup() {
  pinMode(GSM2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  pinMode(setupPin, INPUT);
  //  pinMode(stepperTunerPin, INPUT);

  Serial.begin(9600);
  Serial.println("");
  Serial.println("BeadSorter start");

  //Serial.println("Analyzer start");
  servo.attach(8);
  servo.write(angle);

  //stepper.setMaxSpeed(4000);
  //stepper.setAcceleration(5000);
  stepper.setMaxSpeed(6000);
  stepper.setAcceleration(9000);
  stepper.setCurrentPosition(0);

  if (tcs.begin()) {
    Serial.println("Sensor found");
  } else {
    Serial.println("TCS34725 not found ... exiting!");
    while (1); // Stop program
  }

  if (!autoSort) {
    Serial.println("Import default Colors");
    importDefaultColorSet();
  }

  if (calibrateNullScan) {
    Serial.print("Calibrating: ");
    for (int i = 0; i < 6; i++) {
      Serial.print(i);
      servoFeedIn();
      servoFeedOut();
    }
    Serial.println("");
    servoFeedIn();
    readColorSensor();
    setNullScanValues();
  }

  Serial.println("Starting continous color scan.");
  Serial.println("");
}

void loop() {
  photoSensor = analogRead(photoSensorPin);
  //Serial.println("");Serial.println(photoSensor);

  if (photoSensor > 750) {
    if (!digitalRead(in3) && !digitalRead(in4)) {
      digitalWrite(in3, LOW);  // start
      digitalWrite(in4, HIGH);
      analogWrite(GSM2, motorSpeed);
    }
    //Serial.println("Photosensor OK");
  } else {
    digitalWrite(in3, LOW);  // stop
    digitalWrite(in4, LOW);
    Serial.println("Photosensor Fail");
    //analogWrite(GSM2, 0);
  }

  if (digitalRead(setupPin) == HIGH) {
    while (digitalRead(setupPin) == HIGH) {
    }
    // for manual colors, there are more restricted thresholds
    autoSort = false;
    thresholdFactor = 0.03;
    offset = 20;
    addColor();
  }

  //  if (digitalRead(stepperTunerPin) == HIGH) {
  //    while (digitalRead(stepperTunerPin) == HIGH) {
  //    }
  //    moveStepperForward();
  //  }

  servoFeedIn();
  readColorSensor();

  if (!nullScan()) {
    Serial.println("");
    Serial.print("Beads analyzed: "); Serial.println(beadCounter++);
    Serial.print("\tClear:"); Serial.print(resultColor[0]);
    Serial.print("\tRed:"); Serial.print(resultColor[1]);
    Serial.print("\tGreen:"); Serial.print(resultColor[2]);
    Serial.print("\tBlue:"); Serial.print(resultColor[3]);
    Serial.println("");
    sortBeadToDynamicArray();
    errorCounter = 0;
  } else {
    Serial.print(".");
    errorCounter++;
  }

  servoFeedOut();

  //Serial.print("errorcounter: ");Serial.print(errorCounter);Serial.print(" maxErrorCounter:"); Serial.println(maxErrorCounter);
  if (errorCounter >= maxErrorCounter) {
    reverseContainerMotor();
    errorCounter = 0;
  }

}

void addColor() {
  int colorIndex;

  clearMedianColors();
  stopMotor();

  Serial.print("Insert Color to register. Press button when done...");

  while (digitalRead(setupPin) == LOW) {
  }

  startMotor();
  delay(10000);
  stopMotor();
  for (int i = 0; i < 4; i++) {
    Serial.print(i + 1); Serial.print("/4: ");
    servoFeedIn();
    readColorSensor();

    addColorToMedianColors(i);

    servoFeedOut();
  }

  calcMedianAndStore();
  startMotor();
}

void servoFeedIn() {
  delay(200);
  servo.write(16);
  delay(200);
  servo.write(18);
  delay(200);
  servo.write(17);
  delay(200);
}

void servoFeedOut() {
  servo.write(41);
  delay(200);
  servo.write(44);
  delay(200);
  servo.write(41);
  delay(200);
  servo.write(44);
  delay(200);
  servo.write(41);
  delay(200);
  servo.write(44);
  delay(200);
  servo.write(41);
  delay(200);
  servo.write(44);
  delay(500);
}

void readColorSensor() {
  
  // Sensor returns R G B and Clear value
  uint16_t clearcol, red, green, blue;
  delay(200); // Farbmessung dauert c. 50ms
  tcs.getRawData(&red, &green, &blue, &clearcol);

  resultColor[0] = clearcol;
  resultColor[1] = red;
  resultColor[2] = green;
  resultColor[3] = blue;
}

int addColorToMedianColors(int noOfTest) {
  medianColors[ noOfTest ][ 0 ] += resultColor[0];
  medianColors[ noOfTest ][ 1 ] += resultColor[1];
  medianColors[ noOfTest ][ 2 ] += resultColor[2];
  medianColors[ noOfTest ][ 3 ] += resultColor[3];
}

int getNextFreeArrayPlace() {
  int arrayCounter = 0;
  while (
    (storedColors[ arrayCounter ][ 0 ] > 0) and
    (storedColors[ arrayCounter ][ 1 ] > 0) and
    (storedColors[ arrayCounter ][ 2 ] > 0) and
    (storedColors[ arrayCounter ][ 3 ] > 0)) {
    arrayCounter++;
  }
  return arrayCounter;
}

void clearMedianColors() {
  memset(medianColors, 0, sizeof(medianColors));
}

void calcMedianAndStore() {
  memset(resultColor, 0, sizeof(resultColor));
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      resultColor[i] += medianColors[ j ][ i ];
    }
    resultColor[i] = resultColor[i] / 4;
  }

  int nextColorNo = getNextFreeArrayPlace();

  for (int i = 0; i < 4; i++) {
    storedColors[ nextColorNo ][ i ] = resultColor[i];
  }

  Serial.print("I stored color to bank #"); Serial.print(nextColorNo);
  Serial.println("");
}

void importDefaultColorSet() {
  storeColor(0, 1032, 854, 766); //Orange
  storeColor(1, 1331, 1321, 918); //Bright Yellow
  storeColor(2, 532, 914, 797); //Green
  storeColor(3, 512, 845, 967); //Dark Blue
  storeColor(4, 1257, 1151, 1129); //Rose
  storeColor(5, 1265, 1215, 1100); //Skin
  storeColor(6, 756, 757, 721); // Red
  storeColor(7, 1525, 1858, 1735); //White
  storeColor(8, 512, 742, 700); //Black
  storeColor(9, 939, 1462, 1101); //Lime Green
  storeColor(10, 712, 1298, 1121); //Mint Green
  storeColor(11, 1213, 975, 914); //Coral
  storeColor(12, 1185, 1115, 876); //Dark Yellow
}

String getColorNameFromNo(int colorNo) {
  switch (colorNo) {
    case 0: return "Orange";
    case 1: return "Bright Yellow";
    case 2: return "Green";
    case 3: return "Dark Blue";
    case 4: return "Rose";
    case 5: return "Skin";
    case 6: return "Red";
    case 7: return "White";
    case 8: return "Black";
    case 9: return "Lime Green";
    case 10: return "Mint Green";
    case 11: return "Coral";
    case 12: return "Dark Yellow";
    default: return "Unknown";
  }
}

void storeColor(int index, int red, int green, int blue) {
  storedColors[ index ][0] = index;
  storedColors[ index ][1] = red;
  storedColors[ index ][2] = green;
  storedColors[ index ][3] = blue;
  //  storedColors[ index ][4] = 1;
}

boolean nullScan() {
  if ((resultColor[0] > (nullScanValues[0] - nullScanOffset) and resultColor[0] < (nullScanValues[0] + nullScanOffset)) and
      (resultColor[1] > (nullScanValues[1] - nullScanOffset) and resultColor[1] < (nullScanValues[1] + nullScanOffset)) and
      (resultColor[2] > (nullScanValues[2] - nullScanOffset) and resultColor[2] < (nullScanValues[2] + nullScanOffset)) and
      (resultColor[3] > (nullScanValues[3] - nullScanOffset) and resultColor[3] < (nullScanValues[3] + nullScanOffset))) {
    return true;
  }
  return false;
}

void setNullScanValues() {
  nullScanValues[0] = resultColor[0];
  nullScanValues[1] = resultColor[1];
  nullScanValues[2] = resultColor[2];
  nullScanValues[3] = resultColor[3];

  Serial.print("Calibration results: "); Serial.print(nullScanValues[0]); Serial.print(" "); Serial.print(nullScanValues[1]); Serial.print(" "); Serial.print(nullScanValues[2]); Serial.print(" "); Serial.print(nullScanValues[3]); Serial.println("");
}

void sortBeadToDynamicArray() {
  int threshold;
  int upperLimit;
  int lowerLimit;
  boolean found =  false;

  copyColorsToTemp();

  Serial.println("Analyzing Results:");

  for (int i = 0; i < 16; i++) {
    threshold = thresholdFactor * resultColor[1] + offset;
    upperLimit = resultColor[1] + threshold;
    lowerLimit = resultColor[1] - threshold;
    //        Serial.print(i);Serial.print(":R:"); Serial.print(threshold);Serial.println("");
    //        Serial.print(i);Serial.print(":R:"); Serial.print(upperLimit);Serial.println("");
    //        Serial.print(i);Serial.print(":R:"); Serial.print(lowerLimit);Serial.println("");
    //        Serial.println(tempStoredColors[ i][1]);
    if (tempStoredColors[ i ][1] >= lowerLimit and tempStoredColors[ i ][1] <= upperLimit) {
      threshold = thresholdFactor * resultColor[2] + offset;
      upperLimit = resultColor[2] + threshold;
      lowerLimit = resultColor[2] - threshold;
      //
      //            Serial.print(i);Serial.print(":G:"); Serial.print(threshold);Serial.println("");
      //            Serial.print(i);Serial.print(":G:"); Serial.print(upperLimit);Serial.println("");
      //            Serial.print(i);Serial.print(":G:"); Serial.print(lowerLimit);Serial.println("");

      if (tempStoredColors[ i ][2] >= lowerLimit and tempStoredColors[ i ][2] <= upperLimit) {
        threshold = thresholdFactor * resultColor[3] + offset;
        upperLimit = resultColor[3] + threshold;
        lowerLimit = resultColor[3] - threshold;
        //
        //                Serial.print(i);Serial.print(":B:"); Serial.print(threshold);Serial.println("");
        //                Serial.print(i);Serial.print(":B:"); Serial.print(upperLimit);Serial.println("");
        //                Serial.print(i);Serial.print(":B:"); Serial.print(lowerLimit);Serial.println("");

        if (tempStoredColors[ i ][3] >= lowerLimit and tempStoredColors[ i ][3] <= upperLimit) {
          //          Serial.print("Color is #"); Serial.print(tempStoredColors[ i ][0]); Serial.println("");
          if (autoSort) {
            Serial.print("Color is R:"); Serial.print(tempStoredColors[ i ][1]); Serial.print(" G:"); Serial.print(tempStoredColors[ i ][2]); Serial.print(" B:"); Serial.print(tempStoredColors[ i ][3]);
            //            updateStoredColorCount(i);
            //            updateDetectedColorFromTempStoredColor(i);
          } else {
            Serial.print("Color is #"); Serial.print(getColorNameFromNo(tempStoredColors[ i ][0])); Serial.print(". ");
          }
          //Serial.println(tempStoredColors[ i ][0]);

          int containerNo = getContainerNo(tempStoredColors[ i ][0]);
          //Serial.print("move stepper to container No:"); Serial.println(containerNo);
          moveSorterToPosition(containerNo);
          found = true;
          break;
        }
      }
    }
  }

  if (!found) {
    Serial.println("not found");
    if (autoSort) {
      Serial.println("autosort!");
      if (!allContainerFull()) {
        Serial.println("not allContainerFull. StoreColor");
        storeColor(autoColorCounter, resultColor[1], resultColor[2], resultColor[3]);
        autoColorCounter++;
        moveSorterToPosition(dynamicContainerArraySize - 1);
      } else {
        moveSorterToPosition(dynamicContainerArraySize - 1);
      }
    } else {
      moveSorterToPosition(dynamicContainerArraySize - 1);
    }
  }
}

int getContainerNo(int colorIndex) {
  int containerNo = -1;
  int i = 0;

  for (i; i < dynamicContainerArraySize; i++) {
    //Serial.print("looking for ");Serial.print(colorIndex);Serial.print(" at position ");Serial.println(i);
    if (colorIndex == dynamicContainerArray[i]) {
      Serial.print("Color found in container array: "); Serial.println(dynamicContainerArray[i]);
      containerNo = i;
      break;
    }
  }

  if (containerNo == -1) {
    if (allContainerFull()) {
      containerNo = 15;
    } else {
      containerNo = getNextContainerNo(colorIndex);
    }
  }

  return containerNo;
}

int getNextContainerNo(int colorIndex) {
  Serial.println("finding container for new color");
  int arrayCounter = 0;

  while ((dynamicContainerArray[ arrayCounter ] > -1)) {
    arrayCounter++;
  }
  dynamicContainerArray[arrayCounter] = colorIndex;

  Serial.print("Return Color Index: "); Serial.print(colorIndex); Serial.println("");
  return arrayCounter;
}

bool allContainerFull() {
  int arrayCounter = 0;
  //Serial.println("testing all container full");

  while ((dynamicContainerArray[ arrayCounter ] > -1)) {
    arrayCounter++;
  }

  return arrayCounter >= 15;
}

void copyColorsToTemp() {
  memcpy(tempStoredColors, storedColors, sizeof(tempStoredColors));
}

void moveSorterToPosition(int position) {
  int currentPos = stepper.currentPosition() / stepperMulti;
  int diffToPosition = abs(position - currentPos);

  // TODO: Es wird nur sequentiell angesteuert. 
  //  if ((diffToPosition)>8){
  //    int diff = 16 - diffToPosition ;
  //    position = -diff;
  //  }

  position *= stepperMulti;

  stepper.moveTo(position);
  stepper.runToPosition();
}

void stopMotor() {
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(GSM2, motorSpeed);
}

void startMotor() {
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(GSM2, motorSpeed);
}

void reverseContainerMotor() {

  if (digitalRead(in3)) {
    digitalWrite(in3, LOW);
  } else {
    digitalWrite(in3, HIGH);
  }

  if (!digitalRead(in4)) {
    digitalWrite(in4, HIGH);
  } else {
    digitalWrite(in4, LOW);
  }

  analogWrite(GSM2, motorSpeed);
}

//void updateDetectedColorFromTempStoredColor(int i) {
//  Serial.println(""); Serial.println("Updating color.");
//  Serial.println(""); Serial.print("Red before: "); Serial.print(storedColors[ i ][1]); Serial.print(" Color Counter: "); Serial.print(storedColors[ i ][4]);
//  storedColors[ i ][1] = ((storedColors[ i ][1] * (storedColors[ i ][4] - 1)) + (resultColor[1])) / storedColors[ i ][4];
//  storedColors[ i ][2] = ((storedColors[ i ][2] * (storedColors[ i ][4] - 1)) + (resultColor[2])) / storedColors[ i ][4];
//  storedColors[ i ][3] = ((storedColors[ i ][3] * (storedColors[ i ][4] - 1)) + (resultColor[3])) / storedColors[ i ][4];
//  Serial.println(""); Serial.print("Red after: "); Serial.print(storedColors[ i ][1]);
//  Serial.println("");
//
//}
//
//void updateStoredColorCount(int i){
//  storedColors[ i ][4] += 1;
//}
