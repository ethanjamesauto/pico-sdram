import serial
# Configure the serial port
ser = serial.Serial(
    port='COM4',  # Replace with your serial port
    baudrate=9600,
    timeout=1
)
ser.close()