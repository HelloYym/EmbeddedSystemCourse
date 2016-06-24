import threading
import RPi.GPIO as GPIO
from time import sleep
from gpiozero import LED, Button
from signal import pause
from gpiozero import LightSensor, Buzzer

ldr = LightSensor(pin=4, queue_len=2, charge_time_limit=22, threshold=0.966)

led = LED(17)
button = Button(2)

p_exit = 0;

def led_exit():
	global p_exit
	p_exit = 1

button.when_pressed = led_exit


GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN)
GPIO.setup(15, GPIO.IN)

infrared_on = False
def infrared_detect():
	global infrared_on
	global p_exit
	while True:
		if(p_exit):
			break
		infrared_on = False
		if (GPIO.input(15) == True):
			print "in thread...closing..."
			infrared_on = True 
			sleep(1.5)	

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
			
#infrared_thread = threading.Thread(target = infrared_detect, name='InfraredThread')
#infrared_thread.start()
acoustic_thread = threading.Thread(target = acoustic_detect, name='AcousticThread')
acoustic_thread.start()
while True:
	if (p_exit):
		exit()
	
	if ((infrared_on or acoustic_on or not ldr.light_detected)):
		led.on()
		sleep(2)
	else:
		led.off()		
		sleep(0.1)

pause()
