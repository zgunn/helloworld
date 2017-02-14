#!/usr/bin/python
from pybrain.rl.environments.environment import Environment
from scipy import zeros
import random

class BlackjackEnv(Environment):
	indim = 2 # number of action values environment accepts
	outdim = 21 # number of sensor values the environment produces

	def getSensors(self):
		#hand_val = int(raw_input("Enter hand value: ")) -1
		hand_val = random.randint(2,20)
		print "Hand value: ",hand_val
		return [float(hand_val),]

	def performAction(self,action):
		if action == 1:
			print "Action performed: ",action," : Hit"
		if action == 0:
			print "Action performed: ",action," : Stand"	
	
	def reset(self):
		""" Most environments will implement this optional method that allows for reinitialization. """
