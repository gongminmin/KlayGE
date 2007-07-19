from math import pi, sin, cos

def get_pos(i, j, num):
	return (sin(2 * pi * j / int(num / 10)), i / 10.0, cos(2 * pi * j / int(num / 10)))

def get_clr(i, j, num):
	return (sin(2 * pi * j / int(num / 10)), cos(2 * pi * j / int(num / 10)), 0.0, 1.0)
