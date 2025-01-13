import serial
from tqdm import tqdm

from PIL import Image
import numpy as np

img = Image.open("poop2.jpg").resize((1024, 768))
img_gray = img.convert('L') 

# Convert the image to a NumPy array
img_array = np.array(img)

print(np.min(img_array), np.max(img_array), np.mean(img_array))

# Configure the serial port
ser = serial.Serial(
    port='COM4',  # Replace with your serial port
    baudrate=9600,
    timeout=1
)

print('%04x\n' % 5)

for y in tqdm(range(806)):
    for x in range(512*2):
        if y >= 768:
            n = 0
        else:
            r = img_array[y][x][0] >> 6
            g = img_array[y][x][1] >> 5
            b = img_array[y][x][2] >> 6

            n = (r << 3) | (g << 0) | (b << 5)
        
        #if img_array[x][y] > 90:
        #    n = 0xffff
        # ser.write(('%02x\n' % (n >> 8,)).encode())
        ser.write(('%02x\n' % (n & 0xff,)).encode())

ser.close();