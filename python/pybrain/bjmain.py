#!/usr/bin/python
from bjtask import BlackjackTask
from bjenv import BlackjackEnv
from pybrain.rl.learners.valuebased import ActionValueTable
from pybrain.rl.agents import LearningAgent
from pybrain.rl.learners import Q
from pybrain.rl.experiments import Experiment
from pybrain.rl.explorers import EpsilonGreedyExplorer

# define action-value table
# number of states is:
#	current value: 1-21
# number of actions:
#	Stand=0, Hit=1
av_table = ActionValueTable(21,2)
av_table.initialize(0.)

# define Q-learning agent
learner = Q(0.5,0.0)
learner._setExplorer(EpsilonGreedyExplorer(0.0))
agent = LearningAgent(av_table,learner)

# define env
env = BlackjackEnv()
# define task
task = BlackjackTask(env)
# defin experiment
experiment = Experiment(task,agent)

while True:
	experiment.doInteractions(1)
	agent.learn()
	agent.reset()
