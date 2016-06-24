
from gpiozero import LightSensor, Buzzer

ldr = LightSensor(pin=4, charge_time_limit=22, threshold=0.93)  # alter if using a different pin

while True:
	print(ldr.charge_time_limit)
	print(ldr.value)
