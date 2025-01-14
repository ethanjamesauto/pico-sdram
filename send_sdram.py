import serial
from tqdm import tqdm

from PIL import Image
import numpy as np

#img_gray = img.convert('L') 

# Convert the image to a NumPy array

# Configure the serial port
ser = serial.Serial(
    port='COM4',  # Replace with your serial port
    baudrate=9600,
    timeout=1
)

num_frames = 3

for f in range(num_frames):
    img = Image.open("poop" + str(f+1) + ".jpg").resize((1024, 768))
    img_array = np.array(img)

    for y in tqdm(range(768)):
        row = np.zeros(512*2, dtype=np.uint16)
        for x in range(512*2):
            r = img_array[y][x][0] >> 6
            g = img_array[y][x][1] >> 5
            b = img_array[y][x][2] >> 6
            n = (r << 3) | (g << 0) | (b << 5)
            
            row[x] = n
            #if img_array[x][y] > 90:
            #    n = 0xffff
            # ser.write(('%02x\n' % (n >> 8,)).encode())
        ser.write(row.tobytes())

ser.close();