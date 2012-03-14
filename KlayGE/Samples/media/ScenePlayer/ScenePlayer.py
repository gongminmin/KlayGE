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

	def get(self, row, col):
		return self.vec[row * 4 + col];


def mul(lhs, rhs):
	tmp = transpose(rhs);

	return float4x4(
		lhs.get(0, 0) * tmp.get(0, 0) + lhs.get(0, 1) * tmp.get(0, 1) + lhs.get(0, 2) * tmp.get(0, 2) + lhs.get(0, 3) * tmp.get(0, 3),
		lhs.get(0, 0) * tmp.get(1, 0) + lhs.get(0, 1) * tmp.get(1, 1) + lhs.get(0, 2) * tmp.get(1, 2) + lhs.get(0, 3) * tmp.get(1, 3),
		lhs.get(0, 0) * tmp.get(2, 0) + lhs.get(0, 1) * tmp.get(2, 1) + lhs.get(0, 2) * tmp.get(2, 2) + lhs.get(0, 3) * tmp.get(2, 3),
		lhs.get(0, 0) * tmp.get(3, 0) + lhs.get(0, 1) * tmp.get(3, 1) + lhs.get(0, 2) * tmp.get(3, 2) + lhs.get(0, 3) * tmp.get(3, 3),

		lhs.get(1, 0) * tmp.get(0, 0) + lhs.get(1, 1) * tmp.get(0, 1) + lhs.get(1, 2) * tmp.get(0, 2) + lhs.get(1, 3) * tmp.get(0, 3),
		lhs.get(1, 0) * tmp.get(1, 0) + lhs.get(1, 1) * tmp.get(1, 1) + lhs.get(1, 2) * tmp.get(1, 2) + lhs.get(1, 3) * tmp.get(1, 3),
		lhs.get(1, 0) * tmp.get(2, 0) + lhs.get(1, 1) * tmp.get(2, 1) + lhs.get(1, 2) * tmp.get(2, 2) + lhs.get(1, 3) * tmp.get(2, 3),
		lhs.get(1, 0) * tmp.get(3, 0) + lhs.get(1, 1) * tmp.get(3, 1) + lhs.get(1, 2) * tmp.get(3, 2) + lhs.get(1, 3) * tmp.get(3, 3),

		lhs.get(2, 0) * tmp.get(0, 0) + lhs.get(2, 1) * tmp.get(0, 1) + lhs.get(2, 2) * tmp.get(0, 2) + lhs.get(2, 3) * tmp.get(0, 3),
		lhs.get(2, 0) * tmp.get(1, 0) + lhs.get(2, 1) * tmp.get(1, 1) + lhs.get(2, 2) * tmp.get(1, 2) + lhs.get(2, 3) * tmp.get(1, 3),
		lhs.get(2, 0) * tmp.get(2, 0) + lhs.get(2, 1) * tmp.get(2, 1) + lhs.get(2, 2) * tmp.get(2, 2) + lhs.get(2, 3) * tmp.get(2, 3),
		lhs.get(2, 0) * tmp.get(3, 0) + lhs.get(2, 1) * tmp.get(3, 1) + lhs.get(2, 2) * tmp.get(3, 2) + lhs.get(2, 3) * tmp.get(3, 3),

		lhs.get(3, 0) * tmp.get(0, 0) + lhs.get(3, 1) * tmp.get(0, 1) + lhs.get(3, 2) * tmp.get(0, 2) + lhs.get(3, 3) * tmp.get(0, 3),
		lhs.get(3, 0) * tmp.get(1, 0) + lhs.get(3, 1) * tmp.get(1, 1) + lhs.get(3, 2) * tmp.get(1, 2) + lhs.get(3, 3) * tmp.get(1, 3),
		lhs.get(3, 0) * tmp.get(2, 0) + lhs.get(3, 1) * tmp.get(2, 1) + lhs.get(3, 2) * tmp.get(2, 2) + lhs.get(3, 3) * tmp.get(2, 3),
		lhs.get(3, 0) * tmp.get(3, 0) + lhs.get(3, 1) * tmp.get(3, 1) + lhs.get(3, 2) * tmp.get(3, 2) + lhs.get(3, 3) * tmp.get(3, 3));

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

def transpose(rhs):
	return float4x4(
		rhs.get(0, 0), rhs.get(1, 0), rhs.get(2, 0), rhs.get(3, 0),
		rhs.get(0, 1), rhs.get(1, 1), rhs.get(2, 1), rhs.get(3, 1),
		rhs.get(0, 2), rhs.get(1, 2), rhs.get(2, 2), rhs.get(3, 2),
		rhs.get(0, 3), rhs.get(1, 3), rhs.get(2, 3), rhs.get(3, 3));
