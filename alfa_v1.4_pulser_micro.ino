int inj_Pin = 3;    // Injector to digital         pin 3 OUT
int pump_Pin = 8;   // Pump to digital        pin 8 OUT
int tps_Pin = A1;   // TPS to analog          pin 3 IN
int map_Pin = A2;   // MAP to analog          pin 3 IN
const int cdi_pulse_Pin = 2;    // RPM/Pulse to digital pin 2 IN

int tps_val = 0;    // variable to store the read value
int map_val = 0;
//int val_pwm = 0;
float  val_inj_tune = 0;
unsigned long  lst_inj = 0;
unsigned long  delay_inj = 0;
unsigned long  inj_value = 0;
int inj_state = 0;
int inj_fuel_map = 0;
int inj_value_map = 0;

unsigned long  lst_pump = 0;
unsigned long  delay_pump = 0;
int pump_state = 0;
unsigned long ms;        //time from millis()
unsigned long mcs;        //time from micros()

const int ignitionInterrupt = 0;
const unsigned int pulsesPerRev = 1;
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;
int rpm_int;
int rpm_to_disp;

//fuel map======================================================
int fuel_curve [11][11] = {         //---tune later---
//rpm==0.5k==1k===2k===3k===4k===5k===6k===7k===8k===9k===10k==load%
        {11, 14,  23, 26,  30,  32,  33,  34,  35,  36,  37}, //100%
        {10, 14,  20,  24,  28,  30,  30,  31,  33,  36,  36}, //90%
        {9,  13,  19,  23,  26,  27,  28,  28,  31,  34,  35}, //80%
        {8,  12,  18,  21,  24,  25,  25,  25,  28,  31,  33}, //70%
        {7,  12,  16,  19,  21,  22,  23,  23,  25,  29,  31}, //60%
        {6,  11,  15,  17,  19,  19,  20,  20,  22,  26,  29}, //50%
        {5,  11,  13,  15,  17,  17,  18,  19,  20,  24,  26}, //40%
        {4,  10,  11,  13,  15,  15,  16,  17,  18,  22,  24}, //30%
        {3,   9,   9,  11,  12,  13,  14,  15,  16,  20,  22}, //20%
        {2,   8,   8,   9,  10,  11,  12,  13,  15,  18,  21}, //10%
        {1,   7,   8,   8,   9,   9,  10,  11,  13,  17,  20}, //0%
      };
//======================================================fuel map

void ignitionIsr()
{
	unsigned long now = millis();
	unsigned long interval = now - lastPulseTime;
	if (interval > 5)
	{
		rpm = 60000UL/(interval * pulsesPerRev);
		lastPulseTime = now;
		//rpm_int=int(rpm);
	}  

}

void setup() {
	pinMode(inj_Pin, OUTPUT);     // sets the pin as output
	pinMode(pump_Pin, OUTPUT);    // sets the pin as output
	pinMode(map_Pin, INPUT);      // sets the pin as input
	pinMode(tps_Pin, INPUT);      // sets the pin as input
	pinMode(cdi_pulse_Pin, INPUT);  // sets the pin as input
	attachInterrupt(ignitionInterrupt, ignitionIsr, RISING);
	rpm = 550;
	Serial.begin(9600);
}

void loop() {

	ms = millis();
	mcs = micros();
	tps_val = analogRead(tps_Pin);  // read tps input pin
	map_val = analogRead(map_Pin);  // read map input pin
	noInterrupts();
	rpm_to_disp=int(rpm);
	interrupts();

	//tps 0% = 110
	//tps 100% = 850
	inj_fuel_map = ((-0.117*tps_val)+110)/10; //============= Calculation utk TPS percentage

	if(inj_fuel_map<0){	inj_value=0;}
	else if(inj_fuel_map>=0){	inj_value = inj_value_map;}
	inj_value = fuel_curve[inj_value][0];

	val_inj_tune = (-0.06956*tps_val)+118;

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
	if((inj_state==0 && digitalRead(cdi_pulse_Pin)==HIGH) || ms<3000){
		digitalWrite(inj_Pin, 1);
		lst_inj = mcs;
		inj_state=1;
	}else if(inj_state==1 && (mcs-lst_inj)>(inj_value*100)){
		digitalWrite(inj_Pin, 0);
		delay_inj = mcs;
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
