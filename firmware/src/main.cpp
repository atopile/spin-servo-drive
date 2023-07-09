#include <Arduino.h>
#include <SimpleFOC.h>

// BLDC motor & driver instance
// BLDCMotor motor = BLDCMotor(pole pair number, phase resistance (optional) );
BLDCMotor motor = BLDCMotor(4);

// BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
BLDCDriver6PWM driver = BLDCDriver6PWM(0,1,2,3,6,7);

//Position Sensor
MagneticSensorI2C sensor = MagneticSensorI2C(0x36, 12, 0x0E, 4);

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&motor.target, cmd); }

InlineCurrentSense current_sense = InlineCurrentSense(62.0, A2, A1, A0);


void setup() {
  Serial.begin(115200);
  while (!Serial);
  motor.useMonitoring(Serial);

  // Wire.setSCL(21);
  // Wire.setSDA(20);

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 10;

  driver.init();
  sensor.init();

  // link driver
  motor.linkDriver(&driver);
  motor.linkSensor(&sensor);

  // aligning voltage
  motor.voltage_sensor_align = 1;
  motor.voltage_limit = 2;   // [V] - if phase resistance not defined
  motor.velocity_limit = 5; // [rad/s] cca 50rpm

  // current sense
  current_sense.init();
  // motor.linkCurrentSense(&current_sense);

  // set motion control loop to be used
  motor.torque_controller = TorqueControlType::voltage;
  motor.controller = MotionControlType::torque;
  // motor.controller = MotionControlType::velocity_openloop;

  // add current limit
  // motor.phase_resistance = 3.52 // [Ohm]
  // motor.current_limit = 2;   // [Amps] - if phase resistance defined

  // monitoring
  motor.monitor_downsample = 100; // set downsampling can be even more > 100
  motor.monitor_variables = _MON_CURR_Q | _MON_CURR_D; // set monitoring of d and q current

  // initialize motor
  motor.init();

  // align sensor and start FOC
  // motor.initFOC(0, Direction::CW);
  // Serial.print("Aligning...");
  motor.initFOC();

  // set the initial motor target
  // motor.target = 0.2; // Amps - if phase resistance defined
  // motor.target = 1; // Volts

  // add target command T
  // command.add('T', doTarget, "target current"); // - if phase resistance defined
  command.add('T', doTarget, "target voltage");
  // command.add('T', doTarget, "target velocity");

  Serial.println(F("Motor ready."));
  Serial.println(F("Set the target using serial terminal:"));
  _delay(1000);
}

void loop() {
  // main FOC algorithm function
  motor.loopFOC();

  // Motion control function
  motor.move();

  motor.monitor();

  // user communication
  command.run();
}
