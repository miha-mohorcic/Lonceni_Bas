import time
import os
import RPi.GPIO as GPIO
from threading import Thread
import pygame.mixer

GPIO.setmode(GPIO.BCM) #BCM pin numbering

GPIO.setup(17, GPIO.IN, pull_up_down=GPIO.PUD_DOWN) #zvok 1
GPIO.setup(22, GPIO.IN, pull_up_down=GPIO.PUD_DOWN) #zvok 2
GPIO.setup(23, GPIO.IN, pull_up_down=GPIO.PUD_DOWN) #zvok 3

pygame.mixer.init(48000,-16,1,1024)

zvok1=pygame.mixer.Sound("C.wav")
zvok2=pygame.mixer.Sound("D.wav")
zvok3=pygame.mixer.Sound("E.wav")

chan1=pygame.mixer.Channel(1)
chan2=pygame.mixer.Channel(2)
chan3=pygame.mixer.Channel(3)


def funkcija1(channel):
    print "zaznan pritisk 1"
    chan1.play(zvok1)

def funkcija2(channel):
    print "zaznan pritisk 2"
    chan2.play(zvok2)

def funkcija3(channel):
    print "zaznan pritisk 3"
    chan3.play(zvok3)

    
GPIO.add_event_detect(17, GPIO.RISING, callback=funkcija1, bouncetime=120)
GPIO.add_event_detect(22, GPIO.RISING, callback=funkcija2, bouncetime=120)
GPIO.add_event_detect(23, GPIO.RISING, callback=funkcija3, bouncetime=120)


try:
    while True:
        pass
except KeyboardInterrupt:
	GPIO.cleanup() #reset status of GPIO pins
