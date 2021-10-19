#Converts binary file to Differential Manchester encoded audio
# outputs 32kHz, 8bit, mono WAV. 8N1 format at 3200 baud
# includes calibration tone, and checksum. Zack Nelson 2021
import struct, os
from sys import argv

smplrate = 32000 #Hz
baud = 3200 #needs integer ratio between baud and sample rate 

#functions-------------------------------------------------
#each bit starts by inverting the output
#zeros will invert again in the middle
def out_bit(bit):
    global bit_status
    bit_status = not bit_status #toggle
    for x in range(2): #2 half-cycles
        for y in range(int(smplrate/baud/2)): #samples
            if bit_status: buf.append(0xD8) # hi
            else: buf.append(0x28) # lo
        #toggle if bit 0
        if x == 0 and not bit: bit_status = not bit_status

def out_byte(byte):
    out_bit(0) #start bit
    for i in range(8):
        out_bit(bool(byte & (1<<7)))
        byte <<= 1
    out_bit(1) #stop bit
#---------------------------------------------------------
    
try: len(argv[1]) #load arguments
except IndexError:
    print("Input file needed")
    exit(2)

fi = open(argv[1],'rb') #open input
fo = open(os.path.splitext(argv[1])[0]+".wav", 'wb+') #open output

file = bytearray(fi.read())
fi.close()
buf = []

bit_status = False
checksum = 0

for i in range(smplrate): buf.append(0x80) #silence
for i in range(256): out_bit(1) #calibration bits

for byte in file: #add all bytes
    checksum += byte
    out_byte(byte)

out_byte(checksum) #add checksum

for i in range(smplrate): buf.append(0x80)#silence

#write wave header to file
fo.write(str.encode("RIFF"))
fo.write((len(buf) + 36).to_bytes(4, byteorder='little')) #length in bytes
fo.write(str.encode("WAVEfmt "))
fo.write((16).to_bytes(4, byteorder='little')) #Length of format data
fo.write((1).to_bytes(2, byteorder='little')) #PCM
fo.write((1).to_bytes(2, byteorder='little')) #Number of chans
fo.write((smplrate).to_bytes(4, byteorder='little')) #Sample Rate
fo.write((smplrate).to_bytes(4, byteorder='little')) #Sample Rate * bits * chans / 8
fo.write((1).to_bytes(2, byteorder='little')) #8bit mono
fo.write((8).to_bytes(2, byteorder='little')) #Bits per sample
fo.write(str.encode("data"))
fo.write(len(buf).to_bytes(4, byteorder='little')) #length in bytes

fo.write(struct.pack('B'*len(buf), *buf)) #write audio to file
fo.close()