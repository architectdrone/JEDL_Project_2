//ARDUINO
const float ERROR_RESOLUTION = 0.0049; //(Theoretical) Resolution of Error ADC.
const float PWM_MAX_VOLTAGE = 5.44; //The maximum voltage of the PWM. Note that this is the maximum output FROM THE PWM, not the maximum output of whatever circuit it is connected to.
const float PWM_MIN_VOLTAGE = 0.0; //The minimum voltage of the PWM. See above.
const int PWM_MAX = 255; //The maximum duty cycle int. Dependent on the pin chosen!
const int PWM_MIN = 0; //I can't think of any reason this wouldn't be 0.
const float ERROR_VOLTAGE_DIVIDER_R1 = 3000; 
const float ERROR_VOLTAGE_DIVIDER_R2 = 3000;

//CONTROL
const float DESIRED_VOLTAGE = 8.0; //Voltage we want to hit.
const float ACCEPTABLE_ERROR = 0.5; //The maximum acceptable error, to prevent unnecesary oscillations.


//CALIBRATION
const float ERROR_COR_COEFF_LIN = 0.0403346282255192; //Linear Term of Error Corrective Factor
const float ERROR_COR_COEFF_CON = 0.00305211074581631; //Constant Term of Error Corrective Factor

//PINS
const int PWM_PIN = 6;
const int ERROR_READ_PIN = A7; //Pin that reads voltage.

//State
int current_duty = PWM_MAX/2; //We start at 1/2. Change if needed.

void setup() {
  pinMode(PWM_PIN, OUTPUT);
  Serial.begin(9600);
}

float getActualVoltage()
{
  /*
   * Get actual voltage across the load.
   * Based on a voltage divider.
   */
   return getErrorVoltage()/(ERROR_VOLTAGE_DIVIDER_R1/(ERROR_VOLTAGE_DIVIDER_R1+ERROR_VOLTAGE_DIVIDER_R2));
}

float getErrorVoltage()
{
  /*
   * Returns the voltage, as measured from ERROR_READ_PIN. 
   * 
   * Rationale:
   * According to https://www.arduino.cc/reference/en/language/functions/analog-io/analogread/, the onboard ADC can read between 0 and 5V.
   * For a 10-bit result, this amounts to a resolution of 4.9mV/bit. 
   * Due to irregulatiies with this number, we performed some error correction.
   * It turns out that the onboard ADC can only go up to 4.81V. Anything past that, it just cuts off.
   * We also experimentally determined an error correction factor. See above.
   */
   float measured = analogRead(ERROR_READ_PIN)*ERROR_RESOLUTION;
   float corrective_factor = (ERROR_COR_COEFF_LIN*measured)+ERROR_COR_COEFF_CON;
   float corrected = measured-corrective_factor;
   return corrected;
}

bool isAcceptable(float voltage)
{
  //return false;
  return (voltage > DESIRED_VOLTAGE-ACCEPTABLE_ERROR) && (voltage < DESIRED_VOLTAGE+ACCEPTABLE_ERROR);
}

void loop() {

  analogWrite(PWM_PIN, current_duty);
  
  float voltage = getActualVoltage();
  if (!isAcceptable(voltage))
  {
    if ((voltage < DESIRED_VOLTAGE) && (current_duty+1 <= PWM_MAX))
    {
      current_duty++;
    }
    else if ((voltage > DESIRED_VOLTAGE) && (current_duty-1 >= PWM_MIN))
    {
      current_duty--;
    }
    else
    {
      Serial.println("ERROR in loop(): Voltage is outside of reachable bounds.");
    }
  }

  //Serial.println(voltage, 2);
  //Serial.println(getErrorVoltage());
  Serial.println(current_duty);
}
