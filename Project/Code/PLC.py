import json
import time
import datetime


def send2plc(x,y,z,l):

	if (x):
		x = 1
	else:
		x = 0

	if (y):
		y = 1
	else:
		y = 0

	if (z):
		z = 1
	else:
		z = 0

	if (l):
		l = 1
	else
		l = 0

	packet = "1" + "G881" + str(l) + str(x) + str(y) + str(z);	


	ser = serial.Serial("/dev/ttyAMA0")
	ser.write(packet);

	print len(packet)
	print packet



send2plc(1,1,1,1)


