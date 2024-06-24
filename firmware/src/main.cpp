#include <Arduino.h>
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include <Wire.h>
#include <settings/stm32/STM32FlashSettingsStorage.h>
#include <encoders/ma730/MagneticSensorMA730SSI.h>
#include <encoders/ma730/MagneticSensorMA730.h>
#include <encoders/hysteresis/HysteresisSensor.h>
#include <comms/telemetry/SimpleTelemetry.h>
#include <comms/telemetry/TeleplotTelemetry.h>
#include <utilities/stm32math/STM32G4CORDICTrigFunctions.h>


#define SERIAL_SPEED 115200

#define MUX_SELECT PA6
#define BUTTON PB5
#define BATT_VOLTAGE_SENSE PA4
#define TEMP_SENSE PA5
#define LED PC6

// Define CAN bus pins
#define CAN_TX_PIN PB9
#define CAN_RX_PIN PB8


// BLDC motor & driver instance
BLDCMotor motor = BLDCMotor(5, 5.2f, 120.0f, 0.000831f); // int pp,  float R = NOT_SET, float KV = NOT_SET, float L = NOT_SET
BLDCDriver3PWM driver = BLDCDriver3PWM(PA0, PA1, PA2);

//Position Sensor
MagneticSensorMA730 sensor = MagneticSensorMA730(PB12);
HysteresisSensor hysteresisSensor = HysteresisSensor(sensor, 0.001f);

LowsideCurrentSense current_sense = LowsideCurrentSense(0.003, 46, PB0, PB1, PA3);

// settings
STM32FlashSettingsStorage settings = STM32FlashSettingsStorage(); // use 1 page at top of flash

// instantiate the commander
Commander command = Commander(Serial);
void doMotor(char* cmd) { command.motor(&motor, cmd); };
void doSave(char* cmd) { settings.saveSettings(); };
void doLoad(char* cmd) { settings.loadSettings(); };
void doReinit(char* cmd) { motor.sensor_direction = Direction::UNKNOWN; motor.zero_electric_angle = NOT_SET; motor.initFOC(); };
void doHysteresis(char* cmd) { hysteresisSensor._amount = atof(cmd); Serial.print("Hysteresis: "); Serial.println(hysteresisSensor._amount, 4); };
void doSetSensor(char* cmd) {
  if (cmd[0]=='1') {
    motor.linkSensor(&hysteresisSensor);
    Serial.println("Using hysteresis");
  }
  else {
    motor.linkSensor(&sensor);
    Serial.println("Using MA730");
  }
};

// telemetry
TextIO textIO = TextIO(Serial);
//SimpleTelemetry telemetry = SimpleTelemetry();
TeleplotTelemetry telemetry = TeleplotTelemetry();
void doDownsample(char* cmd) { telemetry.downsample = atoi(cmd); };

void setup() {
  Serial.begin(SERIAL_SPEED);
  while (!Serial);
  SimpleFOCDebug::enable(&Serial);

  Serial.print("spin servo - firmware version ");
  Serial.println(SPIN_SERVO_FIRMWARE_VERSION);

  // Wire.setSCL(PB8);
  // Wire.setSDA(PB9);
  // Wire.begin();
  // Wire.setClock(400000);

  SPI.setMISO(PB14);
  SPI.setMOSI(PB15);
  SPI.setSCLK(PB13);


  SimpleFOC_CORDIC_Config();

  // driver config
  // power supply voltage [V]
  driver.voltage_power_supply = 10;
  driver.voltage_limit = driver.voltage_power_supply*0.95f;
  driver.pwm_frequency	= 20000;

  driver.init();
  sensor.init();
  hysteresisSensor.init();


  FieldStrength fs = sensor.getFieldStrength();
  Serial.print("Field strength: 0x");
  Serial.println(fs, HEX);

  // link driver
  motor.linkDriver(&driver);
  motor.linkSensor(&hysteresisSensor);

  // current sense
  current_sense.linkDriver(&driver);
  current_sense.init();
  motor.linkCurrentSense(&current_sense);

  // aligning voltage
  motor.voltage_sensor_align = 2;
  motor.current_limit = 3;
  motor.voltage_limit = driver.voltage_limit / 2.0f;
  motor.velocity_limit = 1000.0f; // 1000rad/s = aprox 9550rpm

  // some defaults
  motor.PID_velocity.P = 0.05f;
  motor.PID_velocity.I = 0.05f;
  motor.PID_velocity.D = 0.000f;
  motor.PID_velocity.output_ramp = 1000;
  motor.LPF_velocity.Tf = 0.01f;
  motor.P_angle.P = 20.0f;
  motor.P_angle.I = 0.0f;
  motor.P_angle.D = 0.0f;
  motor.P_angle.output_ramp = 1000;
  motor.LPF_angle.Tf = 0.005f;

  motor.torque_controller = TorqueControlType::foc_current;
  motor.controller = MotionControlType::angle;
  motor.motion_downsample = 10;

  // load settings
  settings.addMotor(&motor);
  SimpleFOCRegister registers[] = { REG_SENSOR_DIRECTION, REG_ZERO_ELECTRIC_ANGLE, REG_VEL_LPF_T, REG_VEL_PID_P, REG_VEL_PID_I, REG_VEL_PID_D, REG_VEL_PID_LIM, REG_VEL_PID_RAMP, REG_ANG_LPF_T, REG_ANG_PID_P, REG_ANG_PID_I, REG_ANG_PID_D, REG_ANG_PID_LIM, REG_ANG_PID_RAMP, REG_CURQ_LPF_T, REG_CURQ_PID_P, REG_CURQ_PID_I, REG_CURQ_PID_D, REG_CURQ_PID_LIM, REG_CURQ_PID_RAMP, REG_CURD_LPF_T, REG_CURD_PID_P, REG_CURD_PID_I, REG_CURD_PID_D, REG_CURD_PID_LIM, REG_CURD_PID_RAMP, REG_VOLTAGE_LIMIT, REG_CURRENT_LIMIT, REG_VELOCITY_LIMIT, REG_MOTION_DOWNSAMPLE, REG_TORQUE_MODE, REG_CONTROL_MODE };
  settings.setRegisters(registers, sizeof(registers)/sizeof(SimpleFOCRegister));
  settings.settings_version = 2;
  settings.init();
  //settings.loadSettings();

  // initialize motor
  motor.init();

  // align sensor and start FOC
  Serial.print("Aligning...");
  motor.initFOC();

  // add commands
  command.echo = true;
  command.add('M', doMotor, "motor commands");
  command.add('s', doSave, "save settings");
  command.add('l', doLoad, "load settings");
  command.add('d', doDownsample, "downsample telemetry");
  command.add('r', doReinit, "reinit motor");
  command.add('h', doHysteresis, "hysteresis amount");
  command.add('S', doSetSensor, "set sensor (0=MA730, 1=hysteresis)");
  // add telemetry
  telemetry.addMotor(&motor);
  telemetry.downsample = 0; // off by default, use register 28 to set
  uint8_t telemetry_registers[] = { REG_TARGET, REG_ANGLE, REG_VELOCITY, REG_SENSOR_MECHANICAL_ANGLE, REG_ITERATIONS_SEC };
  telemetry.setTelemetryRegisters(sizeof(telemetry_registers)/sizeof(SimpleFOCRegister), telemetry_registers);
  telemetry.init(textIO);

  Serial.println(F("Motor ready."));
  Serial.println(F("Set the target using serial terminal:"));
}


void loop() {
  // Motion control function
  motor.move();
  // main FOC algorithm function
  motor.loopFOC();
  // user communication
  command.run();
  // telemetry
  telemetry.run();
}
