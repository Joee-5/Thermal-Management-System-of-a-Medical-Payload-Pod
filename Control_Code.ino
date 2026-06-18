/**************************************************************
 * Project: Battery-Powered Plant Chamber Thermal Managment
 * Team   : 
            Ereny Gerges   - 2300036
            Haneen Riad    - 2300971
            Maha sherif    - 2301157
            Youssef Yasser - 2300666 

 * Course : MEP3112 - Heat Transfer
 * Date   : 16/6/2026
 
 * Description:
   This program monitors temperatures using three DHT11 sensors,
   controls a power resistor through a MOSFET, monitors battery
   voltage and current draw, drives a cooling fan when temperatures become high,
   and provides visual status indication using an LED.
 
 * Features:
  - Three-point temperature monitoring
  - Battery voltage monitoring
  - Automatic cooling fan control
  - Low-voltage protection
  - Sensor fault protection
  - Status LED indication
 *************************************************************/

//Defining DHT11 sensors pins, accuracy = +- 2 deg celcius
#include <DHT.h>
#define DHT_CONTROL_PIN 6
#define DHT_POWER_PIN   8
#define DHT_PLANT_PIN   7

#define DHTTYPE DHT11
DHT dhtControl(DHT_CONTROL_PIN, DHTTYPE);
DHT dhtPower(DHT_POWER_PIN, DHTTYPE);
DHT dhtPlant(DHT_PLANT_PIN, DHTTYPE);

//Defining other components pins
#define LED 4
#define MOSFET_GATE 3
#define VB A3
#define FAN_MOSFET_GATE 2

//Resistors values [power resistors, voltage divider] in ohms
#define R_PWR 4 
#define R_DIV 2000

//resistors MOSFET duty cycle [100% = 255]
#define RES_PWM 255
int PWM = RES_PWM;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(MOSFET_GATE, OUTPUT);
  pinMode(FAN_MOSFET_GATE, OUTPUT);
  digitalWrite(FAN_MOSFET_GATE,  LOW); // Fan OFF initially

  dhtControl.begin();
  dhtPower.begin();
  dhtPlant.begin();

  Serial.begin(9600);
}

void loop()
{
  
  //Temperatures readings in celcius
float TC  = dhtControl.readTemperature();
float TP  = dhtPower.readTemperature();
float TPL = dhtPlant.readTemperature();

  //Checking if one of the sensors misses readings
if (isnan(TP) || isnan(TC) || isnan(TPL))
{
    PWM = 0;
    digitalWrite(LED,LOW);
    digitalWrite(FAN_MOSFET_GATE, HIGH);
    Serial.println("DHT read error!");
    delay(1000);
    return;
}
else{
  PWM = RES_PWM;
}
  
  //Battery terminal voltage divider
  float adc = analogRead(VB);
  float vb = adc * (5.0 / 1023.0) * 2.0;
  
  // calculating total current draw
  float I_divider = vb / R_DIV;
  float I_pwr = vb / R_PWR;
  float current = I_divider + I_pwr;

  //LED stops when PWM is cut
  if (PWM == 0)
  {
  digitalWrite(LED, LOW);
  }

  //Fast blinking if temperature rises
  else if (TC > 35 || TP > 35 || TPL > 35)
  {
    digitalWrite(LED,LOW);
    delay(100);
    digitalWrite(LED,HIGH);
    delay(100);
  }

  //Slow blinking if voltage drops & cutoff
  else if(vb < 5)
  {
    PWM = 0;
    digitalWrite(LED,LOW);
    delay(1000);
    digitalWrite(LED,HIGH);
    delay(1000);
  }

  //LED on during steady operation
  else{
  	digitalWrite(LED, HIGH);
  }
  
  //fan starts running at high temperature 
  if (TC >= 38 || TP >= 38 || TPL >= 38)
  {
    digitalWrite(FAN_MOSFET_GATE, HIGH); 
  }

  else if (TC<32 && TP<32 && TPL<32)
  {
  digitalWrite(FAN_MOSFET_GATE, LOW); 
  }

  //PWM input to MOSFETs
  analogWrite(MOSFET_GATE, PWM);

  //Resistors power in Watt
  float power = I_pwr*I_pwr*R_PWR;
  
  //Displaying results
  Serial.print("TC= ");
  Serial.print(TC);
  Serial.print(" C| TP= ");
  Serial.print(TP);
  Serial.print(" C| TPL= ");
  Serial.print(TPL);  
  Serial.print(" C| PWR =");
  Serial.print(power);
  Serial.print(" Watt| I_tot= ");
  Serial.print(current);
  Serial.print(" A| Vb= ");
  Serial.print(vb);
  Serial.println(" V");
  delay(1000);
}