import serial
import time
import numpy as np
import glob
import threading

def find_usb_modem():
    # For macOS
    devices = glob.glob('/dev/tty.usbmodem*')
    # For Linux
    devices += glob.glob('/dev/ttyACM*')
    return devices[0] if devices else None

# Constants
SERIAL_PORT = find_usb_modem()
if not SERIAL_PORT:
    raise Exception("USB modem device not found!")
BAUD_RATE = 115200  # Change this to your servo drive's baud rate
TIMEOUT = 1  # Serial timeout in seconds
motor_data = []  # This list will act as our data frame

# Establish serial connection
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=TIMEOUT)

def send_command(command):
    ser.write((command + '\n').encode())  # Append \n to the command

def read_from_serial():
    while True:
        line = ser.readline().decode().strip()
        if line:  # Check if line is not empty
            motor_data.append(line)

# Start the thread to read from serial
read_thread = threading.Thread(target=read_from_serial)
read_thread.start()

def parse_recorded_data(data):
    parsed_data = []
    current_target = None

    for line in data:
        if "unknown cmd err" in line:
            # Handle or log the error
            continue
        elif "Target:" in line:
            current_target = float(line.split(":")[1].strip())
        else:
            values = [float(val) for val in line.split('\t')]
            parsed_data.append({
                "Target": current_target,
                "_MON_TARGET": values[0],
                "_MON_VOLT_Q": values[1],
                "_MON_VEL": values[2],
                "_MON_ANGLE": values[3]
            })

    return parsed_data

def main():
    send_command("Motion:angle")
    send_command("M1")
    time.sleep(1)
    send_command("M0")
    time.sleep(1)
    send_command("M1")
    time.sleep(1)
    send_command("M0")

    print(parse_recorded_data(motor_data))
    # while True:
        # response = get_step_response()
        # analysis = analyze_response(response)
        # new_params = tune_parameters(analysis)
        # apply_parameters(new_params)
        # Add a condition to break the loop when the system is tuned to your satisfaction

if __name__ == "__main__":
    main()
