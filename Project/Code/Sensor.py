import threading
import RPi.GPIO as GPIO
import PLC
from time import sleep
from gpiozero import LED, Button
from signal import pause
from gpiozero import LightSensor, Buzzer

ldr = LightSensor(pin=4, queue_len=2, charge_time_limit=22, threshold=0.955)
led = LED(17)
buzzer = Buzzer(22)
button = Button(2)

p_exit = 0;
def led_exit():
	global p_exit
	p_exit = 1
button.when_pressed = led_exit

lock = threading.Lock()


GPIO.setmode(GPIO.BCM)
GPIO.setup(18, GPIO.IN)
GPIO.setup(27, GPIO.IN)




infrared_on = False
def infrared_detect():
	global infrared_on
	global p_exit
	while True:
		if(p_exit):
			break	
		if (GPIO.input(27) == True):
			infrared_on = True
			sleep(0.3)	
		else:
			infrared_on = False


acoustic_on = False
def acoustic_detect():
	global acoustic_on
	global p_exit 
	while True:
		if (p_exit):	
			break
		

		acoustic_on = False

		if (GPIO.input(18) == 0):
			acoustic_on = True 
			sleep(0.5)

			# flag = False
			# sleep(0.4)
			# for i in range(1000):
			# 	if (GPIO.input(18) == 0):
			# 		led.off()
			# 		print "led off"
			# 		flag = True
			# 		break
			# 	else:
			# 		sleep(0.001)
			# if (not flag):
			# 	acoustic_on = True 
			# 	sleep(0.5)

light_on = True
def ldr_detect():
	global p_exit 
	global ldr
	global light_on
	global led

	while True:
		if (p_exit):	
			break
		if (light_on):
			if (not ldr.light_detected):
				print "turn dark"
				led.on()
				light_on = False
		else:
			if (ldr.light_detected):
				print "turn light"
				led.off()
				light_on = True
		sleep(0.3)



def sendmsg():
	global acoustic_on
	global infrared_on
	global light_on
	global led
	while True:
		if (p_exit):	
			break
		PLC.send2plc(acoustic_on, light_on, infrared_on, led.is_lit)
		sleep(20)


sendmsg_thread = threading.Thread(target = sendmsg, name='SendMsgThread')
sendmsg_thread.start()



infrared_thread = threading.Thread(target = infrared_detect, name='InfraredThread')
infrared_thread.start()
acoustic_thread = threading.Thread(target = acoustic_detect, name='AcousticThread')
acoustic_thread.start()
ldr_thread = threading.Thread(target = ldr_detect, name='LdrThread')
ldr_thread.start()



while True:
	if (p_exit):
		exit()


	# if (ldr.light_detected):
	# 	led.off()
	# else:
	# 	led.on()

	if (acoustic_on):
		buzzer.off()
		print "voice detected"
		if (not ldr.light_detected):
			led.on()
			sleep(2)
		else:
			led.on()
			sleep(1)
			led.off()

	if (infrared_on):
		print "body detected"
		led.blink(0.2, 0.2)
		buzzer.beep(0.3, 0.3)
		# sleep(2)

	
	sleep(0.3)

pause()

