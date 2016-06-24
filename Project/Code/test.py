import RPi.GPIO as GPIO
from time import sleep
GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN)

while True:
	print GPIO.input(18)
	sleep(1)



