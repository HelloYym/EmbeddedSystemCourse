import threading
import RPi.GPIO as GPIO
from time import sleep
from gpiozero import LED, Button
from signal import pause

p_exit = 0;

def led_exit():
	global p_exit
	p_exit = 1

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(12, GPIO.IN)
GPIO.setup(21, GPIO.OUT)

infrared_on = False
def ledon():
	GPIO.output(21,GPIO.LOW)

def ledoff():
	GPIO.output(21,GPIO.HIGH)
	
def infrared_detect():
	global infrared_on
	global p_exit
	while True:
		if(p_exit):
			break
		infrared_on = False
		print GPIO.input(12)
		if (GPIO.input(12) == True):
			infrared_on = True 
			sleep(1.5)	

infrared_thread = threading.Thread(target = infrared_detect, name='InfraredThread')
infrared_thread.start()
while True:
	if (p_exit):
		exit()
	
	if (infrared_on ):
		ledon()
		sleep(2)
	else:
		ledoff()		
		sleep(0.1)

pause()
