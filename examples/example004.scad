from openscad import *

def example004():
	return difference([
		cube(30, True),
		sphere(20)
	])

openscad.result = example004()

