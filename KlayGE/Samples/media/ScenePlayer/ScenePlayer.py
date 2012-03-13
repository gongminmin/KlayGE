#!/usr/bin/env python
#-*- coding: ascii -*-

from math import *

class float4:
	def __init__(self, x = 0.0, y = 0.0, z = 0.0, w = 0.0):
		self.vec = [ x, y, z, w ];

class float4x4:
	def __init__(self, f11 = 0.0, f12 = 0.0, f13 = 0.0, f14 = 0.0,
				f21 = 0.0, f22 = 0.0, f23 = 0.0, f24 = 0.0,
				f31 = 0.0, f32 = 0.0, f33 = 0.0, f34 = 0.0,
				f41 = 0.0, f42 = 0.0, f43 = 0.0, f44 = 0.0):
		self.vec = [ f11, f12, f13, f14,
			f21, f22, f23, f24,
			f31, f32, f33, f34,
			f41, f42, f43, f44 ];


def rotation_x(x):
	sx = sin(x);
	cx = cos(x);
	return float4x4(
		1,	0,		0,		0,
		0,	cx,		sx,		0,
		0,	-sx,	cx,		0,
		0,	0,		0,		1);

def rotation_y(y):
	sy = sin(y);
	cy = cos(y);
	return float4x4(
		cy,		0,		-sy,	0,
		0,		1,		0,		0,
		sy,		0,		cy,		0,
		0,		0,		0,		1);

def rotation_z(z):
	sz = sin(z);
	cz = cos(z);
	return float4x4(
		cz,		sz,		0,		0,
		-sz,	cz,		0,		0,
		0,		0,		1,		0,
		0,		0,		0,		1);

def scaling(x, y, z):
	return float4x4(
		x,	0,	0,	0,
		0,	y,	0,	0,
		0,	0,	z,	0,
		0,	0,	0,	1);

def translation(x, y, z):
	return float4x4(
		1,	0,	0,	0,
		0,	1,	0,	0,
		0,	0,	1,	0,
		x,	y,	z,	1);
