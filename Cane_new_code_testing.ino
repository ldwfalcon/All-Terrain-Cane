
// This sketch shows how to configure the outp

#include <basicMPU6050.h>
#include "HX711.h"
#include <millisDelay.h>

//-- Input parameters:

// Gyro settings:
#define LP_FILTER 3  // Low pass filter.           Value from 0 to 6
#define GYRO_SENS 0  // Gyro sensitivity.          Value from 0 to 3
#define ACCEL_SENS 3 // Accelerometer sensitivity. Value from 0 to 3

// Accelerometer offset:
constexpr int AX_OFFSET = 552;  // Use these values to calibrate the accelerometer. The sensor should output 1.0g if held level.
constexpr int AY_OFFSET = -241; // These values are unlikely to be zero.
constexpr int AZ_OFFSET = -3185;

// Output scale:
constexpr float AX_SCALE = 1; // Multiplier for accelerometer outputs. Use this to calibrate the sensor. If unknown set to 1.
constexpr float AY_SCALE = 1;
constexpr float AZ_SCALE = 1;

constexpr float GX_SCALE = 0.99764; // Multiplier to gyro outputs. Use this to calibrate the sensor. If unknown set to 1.
constexpr float GY_SCALE = 1.0;
constexpr float GZ_SCALE = 1.01037;

// Bias estimate:
#define GYRO_BAND 35    // Standard deviation of the gyro signal. Gyro signals within this band (relative to the mean) are suppresed.
#define BIAS_COUNT 5000 // Samples of the mean of the gyro signal. Larger values provide better calibration but delay suppression response.

//-- Set the template parameters:

basicMPU6050<LP_FILTER, GYRO_SENS, ACCEL_SENS,
             AX_OFFSET, AY_OFFSET, AZ_OFFSET,
             &AX_SCALE, &AY_SCALE, &AZ_SCALE,
             &GX_SCALE, &GY_SCALE, &GZ_SCALE,
             GYRO_BAND, BIAS_COUNT>
             imu;

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN_RED = 13;
const int LOADCELL_SCK_PIN_RED = 6;
const int LOADCELL_DOUT_PIN_green = 5;
const int LOADCELL_SCK_PIN_green = 4;
const int LOADCELL_DOUT_PIN_yellow = 3;
const int LOADCELL_SCK_PIN_yellow = 2;
int red_zero2 = 0;
int green_zero2 = 0;
int yellow_zero2 = 0;

HX711 redscale;
HX711 greenscale;
HX711 yellowscale;

int gyroData[6];


float Rcalibration_factor = 2230;
float Gcalibration_factor = 2230;
float Ycalibration_factor = 2230;
int r = 1; //rounding number
int i = 1;
const int red = 12; // the number of the LED pin
const int redLow = 11;
const int green = 10;
const int greenLow = 9;
const int yellow = 8;
const int yellowLow = 7;
millisDelay realDelay;
int loopcountkeeper = 0;
int haslifted = 0; // var that keeps track of whether or not the cane has been lifted
int floorLegs = 0; // to keep count of legs in floor
int totalLegs = 0; //to keep count of counter for legs
int redLeg = 0;    //vars to keep track of status of each leg's contact with the ground
int greenLeg = 0;
int yellowLeg = 0;

int redDFG = 0; //distance of leg from ground
int greenDFG = 0;
int yellowDFG = 0;

const int offGround = 2; //the reading of the HX711 that will define if a leg is on or off the ground

class solenoidControl
{
  public:
    void redHigh()
    {
      digitalWrite(red, HIGH);
      digitalWrite(redLow, LOW);
    }
    void greenHigh() {
      digitalWrite(green, HIGH);
      digitalWrite(greenLow, LOW);
    } void yellowHigh()
    {
      digitalWrite(yellow, HIGH);
      digitalWrite(yellowLow, LOW);
    }
    void redLowRun()
    {
      digitalWrite(redLow, HIGH);
      digitalWrite(red, LOW);
    }
    void greenLowRun()
    {
      digitalWrite(greenLow, HIGH);
      digitalWrite(green, LOW);
    }
    void yellowLowRun()
    {
      digitalWrite(yellowLow, HIGH);
      digitalWrite(yellow, LOW);
    }
    void redOff()
    {
      digitalWrite(red, LOW);
      digitalWrite(redLow, LOW);
    }
    void greenOff()
    {
      digitalWrite(green, LOW);
      digitalWrite(greenLow, LOW);
    }
    void yellowOff()
    {
      digitalWrite(yellow, LOW);
      digitalWrite(yellowLow, LOW);
    }
    void high()
    { //activate solenoids
      digitalWrite(red, HIGH);
      digitalWrite(green, HIGH);
      digitalWrite(yellow, HIGH);
      digitalWrite(redLow, LOW);
      digitalWrite(greenLow, LOW);
      digitalWrite(yellowLow, LOW);
    }
    void low()
    { //turn on "low power mode"
      digitalWrite(redLow, HIGH);
      digitalWrite(greenLow, HIGH);
      digitalWrite(yellowLow, HIGH);
      digitalWrite(red, LOW);
      digitalWrite(green, LOW);
      digitalWrite(yellow, LOW);
    }
    void off()
    { //turn off
      digitalWrite(red, LOW);
      digitalWrite(green, LOW);
      digitalWrite(yellow, LOW);
      digitalWrite(redLow, LOW);
      digitalWrite(greenLow, LOW);
      digitalWrite(yellowLow, LOW);
    }
};
solenoidControl solCont;

class dataRead
{
  public:
    void weight()
    {
      // Update gyro calibration
      if (redscale.is_ready())
      {
        int a = i++;
        redscale.set_scale(Rcalibration_factor);
        greenscale.set_scale(Gcalibration_factor);
        yellowscale.set_scale(Ycalibration_factor);
        //int redreading = (((round((redscale.read()/r)))*r - red_zero2)); //for being dymb and calibrating HX711 without using the HX711 calibration from the HX711 library
        //int greenreading = (((round((greenscale.read()/r)))*r - green_zero2)*1);
        //int yellowreading = (((round((yellowscale.read()/r)))*r - yellow_zero2)*1);
        float redreading = redscale.get_units();
        float greenreading = greenscale.get_units();
        float yellowreading = yellowscale.get_units();
        //Serial.print(a);
        //Serial.print(" ");
        Serial.print(redreading / 2);
        Serial.print(" ");
        Serial.print(greenreading / 2);
        Serial.print(" ");
        Serial.println(yellowreading / 2);
      }
    }
    void gyro()
    {

      imu.updateBias();
      int gyroData[6];
      //-- Scaled and calibrated output:
      // Accel
      gyroData[0] = imu.ax();
      gyroData[1] = imu.ay();
      gyroData[2] = imu.az();
      gyroData[3] = imu.gx();
      gyroData[4] = imu.gy();
      gyroData[5] = imu.gz();
      Serial.print(gyroData[3]);
      return gyroData;
    }
};

dataRead dtr;
class dataAnalization
{
  public:
    void gyroPlane()
    {
      int gyToL = 0.15; //⚠️Chang according to actual distance from the center of the gyro to the leg in metters.
      int xTheta = gyroData[0];
      int yTheta = gyroData[1];
      if (xTheta == 0 && yTheta == 0)
      {
        redDFG = 0; //leg is on the ground
        greenDFG = 0;
        yellowDFG = 0;
      }
      else
      {
        redDFG = sin(gyToL * xTheta) * 10; //if red is || to the x axis of the gyro
        greenDFG = sqrt(sin(gyToL * xTheta) + sin(gyToL * yTheta)) * 10; //for the legs that are not || to any axis of the gyro
        yellowDFG = sqrt(sin(gyToL * xTheta) + sin(gyToL * yTheta)) * 10; //uses pythagorean theorem to find the distance based on change in y and x
      }
    }
}; 
dataAnalization dtAna;
class actions
{
  public:
    void sol()
    {
      //int gyro = dtr.gyro();
      if (redscale.is_ready() && greenscale.is_ready() && yellowscale.is_ready())
      {
        int a = i++;
        redscale.set_scale(Rcalibration_factor);
        greenscale.set_scale(Gcalibration_factor);
        yellowscale.set_scale(Ycalibration_factor);
        //int redreading = (((round((redscale.read()/r)))*r - red_zero2));
        //int greenreading = (((round((greenscale.read()/r)))*r - green_zero2)*1);
        //int yellowreading = (((round((yellowscale.read()/r)))*r - yellow_zero2)*1);
        float redreading = redscale.get_units();
        float greenreading = greenscale.get_units();
        float yellowreading = yellowscale.get_units();
        //^^if the rading above are greater than two, then they are off the ground but if they are less than 2, then it is on the ground
        if (redreading > offGround)
        {
          int redLeg = 1; //off the ground
        }
        else
        {
          int redLeg = 0; //on the ground
        }
        if (greenreading > offGround)
        {
          int greenLeg = 3; //these numbers are made so that their sum is special and we can know which legs
        }
        else
        {
          int greenLeg = 0;
        }
        if (yellowreading > offGround)
        {
          int yellowLeg = 5;
        }
        else
        {
          int yellowLeg = 0;
        }

        int floorLegs = redLeg + greenLeg + yellowLeg;

        switch (floorLegs) //to know which and how many legs are on the floor
        {
          case 0:
            totalLegs = 0; //none off, all on ground
            haslifted = 0;
            loopcountkeeper = 0;
            solCont.off();
            break;

          case 1:
            totalLegs = 1; //red off
            break;

          case 3:
            totalLegs = 3; //green off
            break;

          case 5:
            totalLegs = 5; //yellow off
            break;

          case 4:
            totalLegs = 4; //red + green off
            break;

          case 6:
            totalLegs = 6; //red + yellow off
            break;

          case 8:
            totalLegs = 8; //green + yellow off
            break;

          case 9:
            totalLegs = 9; //all off
            haslifted = 1;
            loopcountkeeper = 0;
            solCont.off();
            break;
        }

        if (haslifted = 1 && totalLegs < 9 && totalLegs > 0)
        {
          switch (totalLegs) //to know which and how many legs are on the floor
          {
            case 1: //green + yellow unlock
              if (realDelay.remaining() == 0 && loopcountkeeper == 0)
              {
                solCont.greenHigh();
                solCont.yellowHigh();
                realDelay.start(1000);
                int loopcountkeeer = 1;
              }
              else if (realDelay.remaining() == 0 && loopcountkeeper == 1)
              {
                solCont.greenLowRun();
                solCont.yellowLowRun();
              }
              break;

            case 3: //red + yellow unlock
              if (realDelay.remaining() == 0 && loopcountkeeper == 0)
              {
                solCont.redHigh();
                solCont.yellowHigh();
                realDelay.start(1000);
                loopcountkeeper = 1;
              }
              else if (realDelay.remaining() == 0 && loopcountkeeper == 1)
              {
                solCont.redLowRun();
                solCont.yellowLowRun();
              }
              break;

            case 5: //red + green unlock
              if (realDelay.remaining() == 0 && loopcountkeeper == 0)
              {
                solCont.greenHigh();
                solCont.redHigh();
                realDelay.start(1000);
                loopcountkeeper = 1;
              }
              else if (realDelay.remaining() == 0 && loopcountkeeper == 1)
              {
                solCont.greenLowRun();
                solCont.redLowRun();
              }
              break;

            case 4: //yellow unlock
              if (realDelay.remaining() == 0 && loopcountkeeper == 0)
              {
                solCont.yellowHigh();
                realDelay.start(1000);
                loopcountkeeper = 1;
              }
              else if (realDelay.remaining() == 0 && loopcountkeeper == 1)
              {
                solCont.yellowLowRun();
              }
              break;

            case 6: //green unlock
              if (realDelay.remaining() == 0 && loopcountkeeper == 0)
              {
                solCont.greenHigh();
                realDelay.start(1000);
                loopcountkeeper = 1;
              }
              else if (realDelay.remaining() == 0 && loopcountkeeper == 1)
              {
                solCont.greenLowRun();
              }
              break;

            case 8: //red unlock
              if (realDelay.remaining() == 0 && loopcountkeeper == 0)
              {
                solCont.redHigh();
                realDelay.start(1000);
                loopcountkeeper = 1;
              }
              else if (realDelay.remaining() == 0 && loopcountkeeper == 1)
              {
                solCont.redLowRun();
              }
              break;
          }
        }
      }
      else
      {
        Serial.print("Not ready");
      }
    }
};

void setup()
{
  // Set registers - Always required
  imu.setup();

  // Initial calibration of gyro
  imu.setBias();

  // Start console
  Serial.begin(57600);
  redscale.begin(LOADCELL_DOUT_PIN_RED, LOADCELL_SCK_PIN_RED);
  greenscale.begin(LOADCELL_DOUT_PIN_green, LOADCELL_SCK_PIN_green);
  yellowscale.begin(LOADCELL_DOUT_PIN_yellow, LOADCELL_SCK_PIN_yellow);
  redscale.set_scale();
  greenscale.set_scale();
  yellowscale.set_scale();
  redscale.tare();
  greenscale.tare();
  yellowscale.tare();
  long Rzero_factor = redscale.read_average();    //Get a baseline reading
  long Gzero_factor = greenscale.read_average();  //Get a baseline reading
  long Yzero_factor = yellowscale.read_average(); //Get a baseline reading
  pinMode(red, OUTPUT);
  pinMode(redLow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(greenLow, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(yellowLow, OUTPUT);
}
void loop()
{
  dtr.weight();
  dtr.sol();
}
