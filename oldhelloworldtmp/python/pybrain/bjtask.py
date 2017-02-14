#!/usr/bin/python
from scipy import clip,asarray
from pybrain.rl.environments.task import Task
from numpy import *


class BlackjackTask(Task):
	def __init__(self,environment):
		self.env = environment
		self.lastreward = 0

	def performAction(self,action):
		self.env.performAction(action)

	def getObservation(self):
		sensors = self.env.getSensors()
		return sensors

	def getReward(self):
		reward = raw_input("Enter reward: ")
		cur_reward = self.lastreward
		self.lastreward = reward

		return cur_reward

	@property
	def indim(self):
		return self.env.indim
	
	@property
	def outdim(self):
		return self.env.outdim
