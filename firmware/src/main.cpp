#include <Arduino.h>
#include <SimpleFOC.h>

// BLDC motor & driver instance
// BLDCMotor motor = BLDCMotor(pole pair number, phase resistance (optional) );
BLDCMotor motor = BLDCMotor(4);

// BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
BLDCDriver6PWM driver = BLDCDriver6PWM(0,1,2,3,4,5);

//Position Sensor
MagneticSensorI2C sensor = MagneticSensorI2C(0x36, 12, 0x0E, 4);

// instantiate the commander
Commander command = Commander(Serial);
void doTarget(char* cmd) { command.scalar(&motor.target, cmd); }

void setup() {
  Serial.begin(115200);
  while (!Serial);
  motor.useMonitoring(Serial);

  Wire.setSCL(21);
  Wire.setSDA(20);

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

  // set motion control loop to be used
  motor.torque_controller = TorqueControlType::voltage;
  motor.controller = MotionControlType::velocity_openloop;

  // add current limit
  // motor.phase_resistance = 3.52 // [Ohm]
  // motor.current_limit = 2;   // [Amps] - if phase resistance defined

  // initialize motor
  motor.init();

  // align sensor and start FOC
  // motor.initFOC(0, Direction::CW);
  motor.initFOC();

  // set the initial motor target
  // motor.target = 0.2; // Amps - if phase resistance defined
  motor.target = 1; // Volts

  // add target command T
  // command.add('T', doTarget, "target current"); // - if phase resistance defined
  command.add('T', doTarget, "target voltage");

  Serial.println(F("Motor ready."));
  Serial.println(F("Set the target using serial terminal:"));
  _delay(1000);
}

void loop() {
  // main FOC algorithm function
  motor.loopFOC();

  // Motion control function
  motor.move();

  // user communication
  command.run();

  sensor.update();
  // Serial.println(sensor.getAngle());
}
