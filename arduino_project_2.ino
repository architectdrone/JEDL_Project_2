//ARDUINO
const float ERROR_RESOLUTION = 0.0049; //(Theoretical) Resolution of Error ADC.
const float PWM_MAX_VOLTAGE = 5.44; //The maximum voltage of the PWM. Note that this is the maximum output FROM THE PWM, not the maximum output of whatever circuit it is connected to.
const float PWM_MIN_VOLTAGE = 0.0; //The minimum voltage of the PWM. See above.
const int PWM_MAX = 255; //The maximum duty cycle int. Dependent on the pin chosen!
const int PWM_MIN = 0; //I can't think of any reason this wouldn't be 0.
const float ERROR_VOLTAGE_DIVIDER_R1 = 987; //Connected to Voltage 987
const float ERROR_VOLTAGE_DIVIDER_R2 = 5030; //Connected to Ground 5030

//CONTROL
const float STARTING_DESIRED_VOLTAGE = 10.0; //Voltage we want to hit.
const float ACCEPTABLE_ERROR = 0.1; //The maximum acceptable error, to prevent unnecesary oscillations.


//CALIBRATION
const float ERROR_COR_COEFF_LIN = 0.0403346282255192; //Linear Term of Error Corrective Factor
const float ERROR_COR_COEFF_CON = 0.00305211074581631; //Constant Term of Error Corrective Factor

//PINS
const int PWM_PIN = 7;
const int ERROR_READ_PIN = A0; //Pin that reads voltage.

//State
int current_duty = PWM_MAX/2; //We start at 1/2. Change if needed.
float current_desired_voltage = STARTING_DESIRED_VOLTAGE;

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
  return (voltage > current_desired_voltage-ACCEPTABLE_ERROR) && (voltage < current_desired_voltage+ACCEPTABLE_ERROR);
}

float getAverageActualVoltage()
{
  float sum = 0;
  for (int i = 0; i < 1000; i++)
  {
    //samples[i] = getActualVoltage();
    sum+=getActualVoltage();
  }
  return sum/1000;
}

void handleInput()
{
  String readStr;
  if(Serial.available()){
    //printStatus();
    readStr = Serial.readString();
    while(readStr.length() > 0){
      if(readStr.startsWith("set")){
        readStr.remove(0, 3);
        readStr.trim();
        float v_set = readStr.toFloat();
        v_set = static_cast<float>(static_cast<int>(v_set*2))/2;
        current_desired_voltage = v_set;
      }
      else if (readStr.startsWith("pwm"))
      {
        readStr.remove(0, 3);
        readStr.trim();
        current_duty = readStr.toInt();
      }
      else{
        readStr.remove(0, 1);
      }
    }
  }
}

void loop() {

  analogWrite(PWM_PIN, current_duty);
  
  float voltage = getActualVoltage(); //getAverageActualVoltage();
  if (!isAcceptable(voltage))
  {
    if ((voltage < current_desired_voltage) && (current_duty+1 <= PWM_MAX))
    {
      current_duty++;
    }
    else if ((voltage > current_desired_voltage) && (current_duty-1 >= PWM_MIN))
    {
      current_duty--;
    }
    else
    {
      //Serial.println("ERROR in loop(): Voltage is outside of reachable bounds.");
    }
  }
  //handleInput();
  Serial.print("ACTUAL: ");
  Serial.print(voltage, 2);
  Serial.print("V ");
  Serial.print("MEASURED: ");
  Serial.print(getErrorVoltage(), 2);
  Serial.print(" DESIRED: ");
  Serial.print(current_desired_voltage, 2);
  Serial.print("V ");
  Serial.println(current_duty);
  //delay(500);
}
