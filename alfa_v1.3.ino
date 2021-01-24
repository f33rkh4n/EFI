int inj_Pin = 3;    // Injector to digital pin 9
int pump_Pin = 8;    // Injector to digital pin 9
int tps_Pin = A1;   // TPS to analog pin 3
int map_Pin = A2;   // TPS to analog pin 3
int tps_val = 0;    // variable to store the read value
int map_val = 0;
//int val_pwm = 0;
float  val_inj_tune = 0;
unsigned long  lst_inj = 0;
unsigned long  delay_inj = 0;
unsigned long  inj_value = 0;
int inj_state = 0;
int inj_fuel_map = 0;
unsigned long  lst_pump = 0;
unsigned long  delay_pump = 0;
int pump_state = 0;
unsigned long ms;        //time from millis()

const int ignitionPin = 2;
const int ignitionInterrupt = 0;
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;
int rpm_int;
int rpm_to_disp;


//timing map=====================================================
int adv_curve [11][11] = {         //---tune later---
//rpm===0.5k==1k==1.5k==2k==2.5k==3k==3.5k==4k==5k==6k=====load%
  {15,   7,   8,   8,   9,   9,  10,  11,  13,  17,  20}, //100%
  {15,   8,   8,   9,  10,  11,  12,  13,  15,  18,  21}, //90%
  {15,   9,   9,  11,  12,  13,  14,  15,  16,  20,  22}, //80%
  {15,  10,  11,  13,  15,  15,  16,  17,  18,  22,  24}, //70%
  {15,  11,  13,  15,  17,  17,  18,  19,  20,  24,  26}, //60%
  {15,  11,  15,  17,  19,  19,  20,  20,  22,  26,  29}, //50%
  {15,  12,  16,  19,  21,  22,  23,  23,  25,  29,  31}, //40%
  {15,  12,  18,  21,  24,  25,  25,  25,  28,  31,  33}, //30%
  {15,  13,  19,  23,  26,  27,  28,  28,  31,  34,  35}, //20%
  {15,  14,  20,  24,  28,  30,  30,  31,  33,  36,  36}, //10%
  {15,  15,  21,  26,  30,  32,  33,  34,  35,  36,  37}, //0%
};
//=====================================================timing map

//fuel map======================================================
int fuel_curve [11][11] = {         //---tune later---
//rpm==0.5k==1k===2k===3k===4k===5k===6k===7k===8k===9k===10k==load%
        {5,  14,  21,  26,  30,  32,  33,  34,  35,  36,  37}, //100%
        {5,  14,  20,  24,  28,  30,  30,  31,  33,  36,  36}, //90%
        {5,  13,  19,  23,  26,  27,  28,  28,  31,  34,  35}, //80%
        {5,  12,  18,  21,  24,  25,  25,  25,  28,  31,  33}, //70%
        {5,  12,  16,  19,  21,  22,  23,  23,  25,  29,  31}, //60%
        {5,  11,  15,  17,  19,  19,  20,  20,  22,  26,  29}, //50%
        {5,  11,  13,  15,  17,  17,  18,  19,  20,  24,  26}, //40%
        {5,  10,  11,  13,  15,  15,  16,  17,  18,  22,  24}, //30%
        {5,   9,   9,  11,  12,  13,  14,  15,  16,  20,  22}, //20%
        {5,   8,   8,   9,  10,  11,  12,  13,  15,  18,  21}, //10%
        {5,   7,   8,   8,   9,   9,  10,  11,  13,  17,  20}, //0%
      };
//======================================================fuel map


void ignitionIsr()
{
  unsigned long now = micros();
  unsigned long interval = now - lastPulseTime;
  if (interval > 5000)
  {
    rpm = 60000000UL/(interval * pulsesPerRev);
    lastPulseTime = now;
    //rpm_int=int(rpm);
  }  

}

void setup() {
  pinMode(inj_Pin, OUTPUT);  // sets the pin as output
  pinMode(pump_Pin, OUTPUT);  // sets the pin as output
  pinMode(map_Pin, INPUT);  // sets the pin as output
  pinMode(tps_Pin, INPUT);  // sets the pin as output
  pinMode(ignitionPin, INPUT);
  attachInterrupt(ignitionInterrupt, ignitionIsr, RISING);
  Serial.begin(9600);
}



void loop() {
  
  ms = millis();
  tps_val = analogRead(tps_Pin);  // read tps input pin
  map_val = analogRead(map_Pin);  // read map input pin
  noInterrupts();
  rpm_to_disp=int(rpm);
  interrupts();
  if(rpm_to_disp<1000){inj_fuel_map=0;}
  else if(rpm_to_disp>1000){inj_fuel_map=rpm_to_disp/1000;}
  else if(rpm_to_disp>10000){inj_fuel_map=10;}
  inj_value = fuel_curve[0][inj_fuel_map];
  
  //val_inj_tune = tps_val/4*((0.0004888*map_val)+0.75);
  //val_inj_tune = tps_val/4*((0.0002933*map_val)+0.85);
  val_inj_tune = (-0.8956*tps_val)+398;
  

// Fuel pump State start
  if(pump_state==0 && (ms-delay_pump)>(val_inj_tune*2.5)){
    digitalWrite(pump_Pin, 1);
    lst_pump = ms;
    pump_state=1;
  }else if(pump_state==1 && (ms-lst_pump)>500){
    digitalWrite(pump_Pin, 0);
    delay_pump = ms;
    pump_state=0;
  }
// Fuel pump  State end

// Injector State start
  if(inj_state==0 && (ms-delay_inj)>(val_inj_tune)){
    digitalWrite(inj_Pin, 1);
    lst_inj = ms;
    inj_state=1;
  }else if(inj_state==1 && (ms-lst_inj)>inj_value){
    digitalWrite(inj_Pin, 0);
    delay_inj = ms;
    inj_state=0;
  }
// Injector State end

  Serial.print("rpm:");
  Serial.print(rpm_to_disp);

  Serial.print("   tps:");
  Serial.print(tps_val);

  Serial.print("   inj val:");
  Serial.println(inj_value);
  
  
}
