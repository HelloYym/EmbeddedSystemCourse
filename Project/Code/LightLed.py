import threading
import RPi.GPIO as GPIO
from time import sleep
from gpiozero import LED, Button
from signal import pause

led = LED(17)
button = Button(2)

#button.when_pressed = led.on
#button.when_released = led.off


p_exit = 0;

def led_exit():
	global p_exit
	p_exit = 1

button.when_pressed = led_exit

GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN)

acoustic_on = 0

def acoustic_detect():
	global acoustic_on
	global p_exit 
	while True:
		if (p_exit):	
			break
		acoustic_on = 0
		if (GPIO.input(18) == 0):
			acoustic_on = 1 
			sleep(1.5)	
			

acoustic_thread = threading.Thread(target = acoustic_detect, name='AcousticThread')
acoustic_thread.start()
while True:
	if (p_exit):
		print "pexit"
		exit()
	print acoustic_on
	if (acoustic_on):
		led.on()
		sleep(2)
	else:
		led.off()		
		sleep(0.1)

pause()
