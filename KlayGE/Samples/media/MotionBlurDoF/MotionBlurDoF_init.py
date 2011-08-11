#!/usr/bin/env python
#-*- coding: ascii -*-

from math import pi, sin, cos

def get_pos(i, j, num, line):
	return (sin(2 * pi * j / int(num / line)), i / float(line), cos(2 * pi * j / int(num / line)))

def get_clr(i, j, num, line):
	return (abs(sin(2 * pi * j / int(num / line))), abs(cos(2 * pi * j / int(num / line))), 0.0, 1.0)
