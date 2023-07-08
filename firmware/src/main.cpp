#include <Arduino.h>
#include <SimpleFOC.h>

// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(4);
BLDCDriver6PWM driver = BLDCDriver6PWM(5, 6, 9, 10, 11, 12);

MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);

// instantiate the commander
// Commander command = Commander(Serial);
// void doTarget(char* cmd) { command.scalar(&motor.target, cmd); }

void setup() {

  // initialize encoder sensor hardware
  sensor.init();

  // link the motor to the sensor
//   motor.linkSensor(&sensor);

//   // driver config
//   // power supply voltage [V]
//   driver.voltage_power_supply = 10;
//   driver.init();

//   // link driver
//   motor.linkDriver(&driver);

//   // aligning voltage
//   motor.voltage_sensor_align = 1;

//   // set motion control loop to be used
//   motor.torque_controller = TorqueControlType::voltage;
//   motor.controller = MotionControlType::torque;

  // add current limit
  // motor.phase_resistance = 3.52 // [Ohm]
  // motor.current_limit = 2;   // [Amps] - if phase resistance defined

  // use monitoring with serial
  Serial.begin(115200);
//   // comment out if not needed
//   motor.useMonitoring(Serial);

//   // initialize motor
//   motor.init();

//   // align sensor and start FOC
//   motor.initFOC();

//   // set the initial motor target
//   // motor.target = 0.2; // Amps - if phase resistance defined
//   motor.target = 1; // Volts

//   // add target command T
//   // command.add('T', doTarget, "target current"); // - if phase resistance defined
//   command.add('T', doTarget, "target voltage");

//   Serial.println(F("Motor ready."));
//   Serial.println(F("Set the target using serial terminal:"));
  _delay(1000);
}

void loop() {
  // main FOC algorithm function
//   motor.loopFOC();

//   // Motion control function
//   motor.move();

//   // user communication
//   command.run();

  //
  _delay(100);
  sensor.update();

  // display the angle and the angular velocity to the terminal
  Serial.println(sensor.getAngle());
}
