#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import struct
import sys

shade_mode_enum = {
	"flat" : 0,
	"gouraud" : 1,
}

compare_function_enum = {
	"always_fail" : 0,
	"always_pass" : 1,
	"less" : 2,
	"less_equal" : 3,
	"equal" : 4,
	"not_equal" : 5,
	"greater_equal" : 6,
	"greater" : 7,
}

cull_mode_enum = {
	"none" : 0,
	"front" : 1,
	"back" : 2,
};

polygon_mode_enum = {
	"point" : 0,
	"line" : 1,
	"fill" : 2
}

alpha_blend_factor_enum = {
	"zero" : 0,
	"one" : 1,
	"src_alpha" : 2,
	"dst_alpha" : 3,
	"inv_src_alpha" : 4,
	"inv_dst_alpha" : 5,
	"src_color" : 6,
	"dst_color" : 7,
	"inv_src_color" : 8,
	"inv_dst_color" : 9,
	"src_alpha_sat" : 10,
}

blend_operation_enum = {
	"add" : 1,
	"sub" : 2,
	"rev_sub" : 3,
	"min" : 4,
	"max" : 5,
}

stencil_operation_enum = {
	"keep" : 0,
	"zero" : 1,
	"replace" : 2,
	"increment" : 3,
	"decrement" : 4,
	"invert" : 5,
}

color_mask_enum = {
	"red" : 0x08,
	"green" : 0x04,
	"blue" : 0x02,
	"alpha" : 0x01,
	"all" : 0x0F,
}

texture_filter_enum = {
	"min_mag_mip_point" : 0x00,
	"min_mag_point_mip_linear" : 0x01,
	"min_point_mag_linear_mip_point" : 0x02,
	"min_point_mag_mip_linear" : 0x03,
	"min_linear_mag_mip_point" : 0x04,
	"min_linear_mag_point_mip_linear" : 0x05,
	"min_mag_linear_mip_point" : 0x06,
	"min_mag_mip_linear" : 0x07,
	"anisotropic" : 0x0F,
	"cmp_min_mag_mip_point" : 0x10,
	"cmp_min_mag_point_mip_linear" : 0x11,
	"cmp_min_point_mag_linear_mip_point" : 0x12,
	"cmp_min_point_mag_mip_linear" : 0x13,
	"cmp_min_linear_mag_mip_point" : 0x14,
	"cmp_min_linear_mag_point_mip_linear" : 0x15,
	"cmp_min_mag_linear_mip_point" : 0x16,
	"cmp_min_mag_mip_linear" : 0x17,
	"cmp_anisotropic" : 0x1F,
}

texture_addressing_mode_enum = {
	"wrap" : 0,
	"mirror" : 1,
	"clamp" : 2,
	"border" : 3,
}

types_define = [
	"bool",
	"dword",
	"string",
	"texture1D",
	"texture2D",
	"texture3D",
	"textureCUBE",
	"sampler",
	"shader",
	"int",
	"int2",
	"int3",
	"int4",
	"float",
	"float2",
	"float2x2",
	"float2x3",
	"float2x4",
	"float3",
	"float3x2",
	"float3x3",
	"float3x4",
	"float4",
	"float4x2",
	"float4x3",
	"float4x4"
]

render_states_define = [
	("polygon_mode", "int"),
	("shade_mode", "int"),
	("cull_mode", "int"),
	("front_face_ccw", "bool"),
	("polygon_offset_factor", "float"),
	("polygon_offset_units", "float"),
	("scissor_enable", "bool"),
	("multisample_enable", "bool"),

	("alpha_to_coverage_enable", "bool"),
	("independent_blend_enable", "bool"),
	("blend_enable", "bool"),
	("blend_op", "int"),
	("src_blend", "int"),
	("dest_blend", "int"),
	("blend_op_alpha", "int"),
	("src_blend_alpha", "int"),
	("dest_blend_alpha", "int"),
	("color_write_mask", "int"),
	("blend_factor", "float4"),
	("sample_mask", "int"),

	("depth_enable", "bool"),
	("depth_write_mask", "bool"),
	("depth_func", "int"),

	("front_stencil_enable", "bool"),
	("front_stencil_func", "int"),
	("front_stencil_ref", "int"),
	("front_stencil_read_mask", "int"),
	("front_stencil_write_mask", "int"),
	("front_stencil_fail", "int"),
	("front_stencil_depth_fail", "int"),
	("front_stencil_pass", "int"),
	("back_stencil_enable", "bool"),
	("back_stencil_func", "int"),
	("back_stencil_ref", "int"),
	("back_stencil_read_mask", "int"),
	("back_stencil_write_mask", "int"),
	("back_stencil_fail", "int"),
	("back_stencil_depth_fail", "int"),
	("back_stencil_pass", "int"),

	("pixel_shader", "shader"),
	("vertex_shader", "shader"),
	("geometry_shader", "shader"),
]

sampler_states_define = [
	("filtering", "int"),
	("address_u", "int"),
	("address_v", "int"),
	("address_w", "int"),
	("anisotropy", "int"),
	("max_mip_level", "int"),
	("mip_map_lod_bias", "float"),
	("border_clr", "float4")
]


def type_code(type_name):
	for i in range(0, len(types_define)):
		if (types_define[i] == type_name):
			return i
	else:
		print("Wrong type name: " + type_name)
		assert False

def render_state_code(state_name):
	for i in range(0, len(render_states_define)):
		if (render_states_define[i][0] == state_name):
			return i
	else:
		print("Wrong state name: " + state_name)
		assert False

def get_matrix(result, col, row, node):
	for y in range(row):
		for x in range(col):
			attr_name = '_' + str(y) + str(x)
			if node.getAttributeNode(attr_name) != None:
				setattr(result, attr_name, float(node.getAttribute(attr_name)))
			else:
				setattr(result, attr_name, 0.0)

def get_value(result, type, node):
	if ('float' == type):
		if node.getAttributeNode('value') != None:
			result.value = float(node.getAttribute('value'))
		else:
			result.value = 0.0
	elif (('int' == type) or ('dword' == type)):
		if node.getAttributeNode('value') != None:
			s = node.getAttribute('value').lower()
			if (('0' == s[0]) and ('x' == s[1])):
				result.value = int(s, 16)
			elif ('0' == s[0]):
				result.value = int(s, 8)
			else:
				result.value = int(s, 10)
		else:
			result.value = 0
	elif ('int2' == type):
		if ((node.getAttributeNode('x') != None)
				and (node.getAttributeNode('y') != None)):
			result.x = int(node.getAttribute('x'))
			result.y = int(node.getAttribute('y'))
		else:
			result.x = 0
			result.y = 0
	elif ('int3' == type):
		if ((node.getAttributeNode('x') != None)
				and (node.getAttributeNode('y') != None)
				and (node.getAttributeNode('z') != None)):
			result.x = int(node.getAttribute('x'))
			result.y = int(node.getAttribute('y'))
			result.z = int(node.getAttribute('z'))
		else:
			result.x = 0
			result.y = 0
			result.z = 0
	elif ('int4' == type):
		if ((node.getAttributeNode('x') != None)
				and (node.getAttributeNode('y') != None)
				and (node.getAttributeNode('z') != None)
				and (node.getAttributeNode('w') != None)):
			result.x = int(node.getAttribute('x'))
			result.y = int(node.getAttribute('y'))
			result.z = int(node.getAttribute('z'))
			result.w = int(node.getAttribute('w'))
		else:
			result.x = 0
			result.y = 0
			result.z = 0
			result.w = 0
	elif ('bool' == type):
		if node.getAttributeNode('value') != None:
			result.value = int('true' == node.getAttribute('value').lower())
		else:
			result.value = False
	elif ('string' == type):
		if node.getAttributeNode('value') != None:
			result.value = node.getAttribute('value')
		else:
			result.value = ''
	elif ('float2' == type):
		if ((node.getAttributeNode('x') != None)
				and (node.getAttributeNode('y') != None)):
			result.x = float(node.getAttribute('x'))
			result.y = float(node.getAttribute('y'))
		else:
			result.x = 0.0
			result.y = 0.0
	elif ('float2x2' == type):
		get_matrix(result, 2, 2, node)
	elif ('float2x3' == type):
		get_matrix(result, 2, 3, node)
	elif ('float2x4' == type):
		get_matrix(result, 2, 4, node)
	elif ('float3' == type):
		if ((node.getAttributeNode('x') != None)
				and (node.getAttributeNode('y') != None)
				and (node.getAttributeNode('z') != None)):
			result.x = float(node.getAttribute('x'))
			result.y = float(node.getAttribute('y'))
			result.z = float(node.getAttribute('z'))
		else:
			result.x = 0.0
			result.y = 0.0
			result.z = 0.0
	elif ('float3x2' == type):
		get_matrix(result, 3, 2, node)
	elif ('float3x3' == type):
		get_matrix(result, 3, 3, node)
	elif ('float3x4' == type):
		get_matrix(result, 3, 4, node)
	elif ('float4' == type):
		if ((node.getAttributeNode('x') != None)
				and (node.getAttributeNode('y') != None)
				and (node.getAttributeNode('z') != None)
				and (node.getAttributeNode('w') != None)):
			result.x = float(node.getAttribute('x'))
			result.y = float(node.getAttribute('y'))
			result.z = float(node.getAttribute('z'))
			result.w = float(node.getAttribute('w'))
		else:
			result.x = 0.0
			result.y = 0.0
			result.z = 0.0
			result.w = 0.0
	elif ('float4x2' == type):
		get_matrix(result, 4, 2, node)
	elif ('float4x3' == type):
		get_matrix(result, 4, 3, node)
	elif ('float4x4' == type):
		get_matrix(result, 4, 4, node)
	elif ('shader' == type):
		if (node.getAttributeNode('profile') != None):
			result.profile = str(node.getAttribute('profile'))
		else:
			result.profile = 'auto'

		result.value = str(node.getAttribute('value'))

def write_short_string(stream, str):
	stream.write(struct.pack('B', len(str)))
	if 3 == sys.version_info[0]:
		stream.write(bytes(str, encoding = 'ascii'))
	else:
		stream.write(str)

def write_matrix(stream, col, row, var):
	for y in range(row):
		for x in range(col):
			attr_name = '_' + str(y) + str(x)
			stream.write(struct.pack('f', getattr(var, attr_name)))

def write_var(stream, type, name, var):
	stream.write(struct.pack('I', type_code(type)))
	write_short_string(stream, name)

	if hasattr(var, 'array_size') and (var.array_size != 0):
		return

	if ('string' == type):
		write_short_string(stream, var.value)
	elif ('dword' == type):
		stream.write(struct.pack('I', var.value))
	elif ('bool' == type):
		stream.write(struct.pack('B', var.value))
	elif ('shader' == type):
		write_short_string(stream, var.profile)
		write_short_string(stream, var.value)
	elif ('int' == type):
		stream.write(struct.pack('i', var.value))
	elif ('int2' == type):
		stream.write(struct.pack('i', var.x))
		stream.write(struct.pack('i', var.y))
	elif ('int3' == type):
		stream.write(struct.pack('i', var.x))
		stream.write(struct.pack('i', var.y))
		stream.write(struct.pack('i', var.z))
	elif ('int4' == type):
		stream.write(struct.pack('i', var.x))
		stream.write(struct.pack('i', var.y))
		stream.write(struct.pack('i', var.z))
		stream.write(struct.pack('i', var.w))
	elif ('float' == type):
		stream.write(struct.pack('f', var.value))
	elif ('float2' == type):
		stream.write(struct.pack('f', var.x))
		stream.write(struct.pack('f', var.y))
	elif ('float2x2' == type):
		write_matrix(stream, 2, 2, var)
	elif ('float2x3' == type):
		write_matrix(stream, 2, 3, var)
	elif ('float2x4' == type):
		write_matrix(stream, 2, 4, var)
	elif ('float3' == type):
		stream.write(struct.pack('f', var.x))
		stream.write(struct.pack('f', var.y))
		stream.write(struct.pack('f', var.z))
	elif ('float3x2' == type):
		write_matrix(stream, 3, 2, var)
	elif ('float3x3' == type):
		write_matrix(stream, 3, 3, var)
	elif ('float3x4' == type):
		write_matrix(stream, 3, 4, var)
	elif ('float4' == type):
		stream.write(struct.pack('f', var.x))
		stream.write(struct.pack('f', var.y))
		stream.write(struct.pack('f', var.z))
		stream.write(struct.pack('f', var.w))
	elif ('float4x2' == type):
		write_matrix(stream, 4, 2, var)
	elif ('float4x3' == type):
		write_matrix(stream, 4, 3, var)
	elif ('float4x4' == type):
		write_matrix(stream, 4, 4, var)
	elif ('sampler' == type):
		stream.write(struct.pack('i', var.filtering))
		stream.write(struct.pack('i', var.address_u))
		stream.write(struct.pack('i', var.address_v))
		stream.write(struct.pack('i', var.address_w))
		stream.write(struct.pack('i', var.anisotropy))
		stream.write(struct.pack('i', var.max_mip_level))
		stream.write(struct.pack('f', var.mip_map_lod_bias))
		stream.write(struct.pack('f', var.border_clr[0]))
		stream.write(struct.pack('f', var.border_clr[1]))
		stream.write(struct.pack('f', var.border_clr[2]))
		stream.write(struct.pack('f', var.border_clr[3]))

class annotation:
	def __init__(self, tag):
		self.type = tag.getAttribute('type')
		self.name = tag.getAttribute('name')
		get_value(self, self.type, tag)

	def write(self, stream):
		write_var(stream, self.type, self.name, self)

	def __str__(self):
		ret = self.type + ' ' + self.name + ' = '

		if hasattr(self, 'value'):
			if "string" == self.type:
				ret += '"' + str(self.value) + '"'
			else:
				ret += str(self.value)
		else:
			ret += '{ ' + str(self.x)
			if hasattr(self, 'y'):
				ret += ', ' + str(self.y)
			if hasattr(self, 'z'):
				ret += ', ' + str(self.z)
			if hasattr(self, 'w'):
				ret += ', ' + str(self.w)
			ret += ' }'

		ret += ';'

		return ret

class parameter:
	def __init__(self, tag):
		self.semantic = tag.getAttribute('semantic')
		self.type = tag.getAttribute('type')
		self.name = tag.getAttribute('name')
		if 0 == len(tag.getAttribute('array_size')):
			get_value(self, self.type, tag)
			self.array_size = 0
		else:
			self.array_size = int(tag.getAttribute('array_size'))

		if "cbuffer" == tag.parentNode.tagName:
			self.cbuff = tag.parentNode.getAttribute('name')
		else:
			self.cbuff = "global_cb"

		self.annotations = []
		anns = tag.getElementsByTagName('annotation')			
		for ann in anns:
			self.annotations.append(annotation(ann))

		if 'sampler' == self.type:
			self.filtering = texture_filter_enum['min_mag_mip_point']
			self.address_u = texture_addressing_mode_enum['wrap']
			self.address_v = texture_addressing_mode_enum['wrap']
			self.address_w = texture_addressing_mode_enum['wrap']
			self.anisotropy = 1
			self.max_mip_level = 0
			self.mip_map_lod_bias = 0
			self.border_clr = [0.0, 0.0, 0.0, 0.0]

			states = tag.getElementsByTagName('state')
			for state in states:
				name = state.getAttribute('name')
				value_str = state.getAttribute('value')
				if ('filtering' == name):
					try:
						self.filtering = int(value_str)
					except:
						self.filtering = texture_filter_enum[value_str]
				elif ('address_u' == name):
					try:
						self.address_u = int(value_str)
					except:
						self.address_u = texture_addressing_mode_enum[value_str]

				elif ('address_v' == name):
					try:
						self.address_v = int(value_str)
					except:
						self.address_v = texture_addressing_mode_enum[value_str]
				elif ('address_w' == name):
					try:
						self.address_w = int(value_str)
					except:
						self.address_w = texture_addressing_mode_enum[value_str]
				elif ('anisotropy' == name):
					self.anisotropy = int(state.getAttribute('value'))
				elif ('max_mip_level' == name):
					self.max_mip_level = int(state.getAttribute('value'))
				elif ('mip_map_lod_bias' == name):
					self.mip_map_lod_bias = float(state.getAttribute('value'))
				elif ('border_clr' == name):
					self.border_clr[0] = float(state.getAttribute('r'))
					self.border_clr[1] = float(state.getAttribute('g'))
					self.border_clr[2] = float(state.getAttribute('b'))
					self.border_clr[3] = float(state.getAttribute('a'))
				else:
					print("Wrong sampler state name:" + self.name)

	def write(self, stream):
		stream.write(struct.pack('I', self.array_size))
		write_var(stream, self.type, self.name, self)

		stream.write(struct.pack('I', len(self.annotations)))
		if len(self.annotations) != 0:
			for ann in self.annotations:
				ann.write(stream)

		write_short_string(stream, self.semantic)

	def __str__(self):
		ret = self.type + ' ' + self.name
		if len(self.annotations) != 0:
			ret += '<'
			for ann in self.annotations:
				ret += str(ann)
			ret += '>'

		if hasattr(self, 'value'):
			if "string" == self.type:
				ret += ' = "' + str(self.value) + '"'
			else:
				ret += ' = ' + str(self.value)
		elif hasattr(self, 'x'):
			ret += ' = {' + str(self.x)
			if hasattr(self, 'y'):
				ret += ',' + str(self.y)
			if hasattr(self, 'z'):
				ret += ',' + str(self.z)
			if hasattr(self, 'w'):
				ret += ',' + str(self.w)
			ret += '}'

		if len(self.semantic) != 0:
			ret += ':' + self.semantic

		ret += ';'

		return ret

class shader_func:
	def __init__(self, tag):
		self.str = ''
		for node in tag.childNodes:
			import xml.dom
			if (node.nodeType in [xml.dom.Node.TEXT_NODE, xml.dom.Node.CDATA_SECTION_NODE]):
				self.str += node.data

	def write(self, stream):
		stream.write(struct.pack('I', len(self.str)))
		if 3 == sys.version_info[0]:
			stream.write(bytes(self.str, encoding = 'ascii'))
		else:
			stream.write(self.str)

	def __str__(self):
		return self.str

class render_state:
	def __init__(self, tag):
		self.name = tag.getAttribute('name')
		for state_define in render_states_define:
			if state_define[0] == self.name:
				self.type = state_define[1]
				break
		else:
			print("Wrong render state name:" + self.name)

		if tag.getAttributeNode('index') != None:
			self.index = int(tag.getAttribute('index'))

		value_str = tag.getAttribute('value')
		if "bool" == self.type:
			self.value = bool('true' == value_str.lower())
		elif "shader" == self.type:
			if (tag.getAttributeNode('profile') != None):
				self.profile = str(tag.getAttribute('profile'))
			else:
				self.profile = 'auto'

			self.value = str(value_str)
		elif "polygon_mode" == self.name:
			try:
				self.value = int(value_str)
			except:
				self.value = polygon_mode_enum[value_str]
		elif "shade_mode" == self.name:
			try:
				self.value = int(value_str)
			except:
				self.value = shade_mode_enum[value_str]
		elif "cull_mode" == self.name:
			try:
				self.value = int(value_str)
			except:
				self.value = cull_mode_enum[value_str]
		elif self.name in ["slope_scale_depth_bias", "depth_bias"]:
			self.value = float(value_str)
		elif self.name in ["blend_op", "blend_op_alpha"]:
			try:
				self.value = int(value_str)
			except:
				self.value = blend_operation_enum[value_str]
		elif self.name in ["src_blend", "dest_blend", "src_blend_alpha", "dest_blend_alpha"]:
			try:
				self.value = int(value_str)
			except:
				self.value = alpha_blend_factor_enum[value_str]
		elif self.name in ["depth_func", "front_stencil_func", "back_stencil_func"]:
			try:
				self.value = int(value_str)
			except:
				self.value = compare_function_enum[value_str]
		elif self.name in ["front_stencil_ref", "front_stencil_read_mask", "front_stencil_write_mask",
							"back_stencil_ref", "back_stencil_read_mask", "back_stencil_write_mask"]:
			self.value = int(value_str)
		elif self.name in ["front_stencil_fail", "front_stencil_depth_fail", "front_stencil_pass",
							"back_stencil_fail", "back_stencil_depth_fail", "back_stencil_pass"]:
			try:
				self.value = int(value_str)
			except:
				self.value = stencil_operation_enum[value_str]
		elif "color_mask" == self.name:
			self.value = int(value_str)
		elif "blend_factor" == self.name:
			self.x = float(tag.getAttribute('r'))
			self.y = float(tag.getAttribute('g'))
			self.z = float(tag.getAttribute('b'))
			self.w = float(tag.getAttribute('a'))
		elif "sample_mask" == self.name:
			self.value = int(value_str)
		else:
			print("Wrong render state name:" + self.name)
			assert False

	def write(self, stream):
		n = self.name
		if hasattr(self, 'index'):
			n += '[' + str(self.index) + ']'
		write_var(stream, self.type, n, self)

	def __str__(self):
		ret = self.name

		if hasattr(self, 'index'):
			ret += '[' + str(self.index) + ']'

		if hasattr(self, 'value'):
			if "string" == self.type:
				ret += ' = "' + str(self.value) + '"'
			else:
				ret += ' = ' + str(self.value)
		elif hasattr(self, 'x'):
			ret += ' = {' + str(self.x)
			if hasattr(self, 'y'):
				ret += ',' + str(self.y)
			if hasattr(self, 'z'):
				ret += ',' + str(self.z)
			if hasattr(self, 'w'):
				ret += ',' + str(self.w)
			ret += '}'

		ret += ';'

		return ret

class render_state_block:
	def __init__(self, tag):
		self.name = tag.getAttribute('name')

		self.render_states = []
		states = tag.getElementsByTagName('state')
		for state in states:
			self.render_states.append(render_state(state))

class render_pass:
	def __init__(self, tag, render_state_blocks):
		self.name = tag.getAttribute('name')

		self.annotations = []
		anns = tag.getElementsByTagName('annotation')			
		for ann in anns:
			self.annotations.append(annotation(ann))

		self.render_states = []
		
		sblocks = tag.getElementsByTagName('state_block')
		for sblock in sblocks:
			ref_name = sblock.getAttribute('name')
			for rsbs in render_state_blocks:
				if rsbs.name == ref_name:
					for rs in rsbs.render_states:
						self.render_states.append(rs)
					break

		states = tag.getElementsByTagName('state')
		for state in states:
			self.render_states.append(render_state(state))

	def write(self, stream):
		write_short_string(stream, self.name)

		stream.write(struct.pack('I', len(self.annotations)))
		if len(self.annotations) != 0:
			for ann in self.annotations:
				ann.write(stream)

		stream.write(struct.pack('I', len(self.render_states)))
		for rs in self.render_states:
			rs.write(stream)

	def __str__(self):
		ret = 'pass ' + self.name
		if len(self.annotations) != 0:
			ret += '<'
			for ann in self.annotations:
				ret += str(ann)
			ret += '>'

		ret += '{'
		for rs in self.render_states:
			ret += str(rs)
		ret += '}'

		return ret

class technique:
	def __init__(self, tag, render_state_blocks):
		self.weight = 1.0

		self.name = tag.getAttribute('name')

		self.annotations = []
		anns = tag.getElementsByTagName('annotation')			
		for ann in anns:
			self.annotations.append(annotation(ann))

		self.render_passes = []
		rps = tag.getElementsByTagName('pass')
		for rp in rps:
			self.render_passes.append(render_pass(rp, render_state_blocks))

		for p in self.render_passes:
			self.weight += len(p.render_states)

		blend = False
		for p in self.render_passes:
			for rs in p.render_states:
				if ('blend_enable' == rs.name) and rs.value:
					blend = True
					break
		if blend:
			self.weight += 10000

	def write(self, stream):
		write_short_string(stream, self.name)
		stream.write(struct.pack('f', self.weight))

		stream.write(struct.pack('I', len(self.annotations)))
		if len(self.annotations) != 0:
			for ann in self.annotations:
				ann.write(stream)

		stream.write(struct.pack('I', len(self.render_passes)))
		for rp in self.render_passes:
			rp.write(stream)

	def __str__(self):
		ret = 'technique ' + self.name
		if len(self.annotations) != 0:
			ret += '<'
			for ann in self.annotations:
				ret += str(ann)
			ret += '>'

		ret += '{'
		for rp in self.render_passes:
			ret += str(rp)
		ret += '}'

		return ret

class effect:
	def __init__(self, tag):
		self.cbuffers = []

		self.parameters = []
		vars = tag.getElementsByTagName('parameter')
		for var in vars:
			p = parameter(var)

			if p.type != "sampler":
				found = False
				for m in self.cbuffers:
					if m[0] == p.cbuff:
						m[1].append(len(self.parameters))
						found = True
						break
				if not found:
					self.cbuffers.append([p.cbuff, [len(self.parameters)]])

			self.parameters.append(p)

		render_state_blocks = []
		sblocks = tag.getElementsByTagName('state_block')
		for block in sblocks:
			render_state_blocks.append(render_state_block(block))

		self.shaders = []
		shaders = tag.getElementsByTagName('shader')
		for shader in shaders:
			self.shaders.append(shader_func(shader))

		self.techniques = []
		techs = tag.getElementsByTagName('technique')
		for tech in techs:
			self.techniques.append(technique(tech, render_state_blocks))

	def write(self, stream):
		if 3 == sys.version_info[0]:
			stream.write(b'FXML')
		else:
			stream.write('FXML')
		stream.write(struct.pack('I', 3))  # version
		stream.write(struct.pack('I', len(self.parameters)))
		stream.write(struct.pack('I', len(self.cbuffers)))
		stream.write(struct.pack('I', len(self.shaders)))
		stream.write(struct.pack('I', len(self.techniques)))

		for var in self.parameters:
			var.write(stream)

		for cbuff in self.cbuffers:
			write_short_string(stream, cbuff[0])
			stream.write(struct.pack('I', len(cbuff[1])))
			for p in cbuff[1]:
				stream.write(struct.pack('I', p))

		for shader in self.shaders:
			shader.write(stream)

		for tech in self.techniques:
			tech.write(stream)

	def __str__(self):
		ret = ''

		for var in self.parameters:
			ret += str(var)

		for shader in self.shaders:
			ret += str(shader)

		for tech in self.techniques:
			ret += str(tech)

		return ret


def preprocess(file_name):
	ret = ''

	dir = ''
	last_slash = file_name.rfind('/')
	if last_slash != -1:
		dir = file_name[0 : last_slash + 1]

	import xml.dom
	
	from xml.dom.minidom import parse
	dom = parse(file_name)
	
	from sys import	getfilesystemencoding
	encoding = getfilesystemencoding()

	include_files = dom.documentElement.getElementsByTagName('include')
	while len(include_files) != 0:
		for include_file in include_files:
			if 3 == sys.version_info[0]:
				include_name = include_file.getAttribute('name')
			else:
				include_name = include_file.getAttribute('name').encode(encoding)
			if len(include_name) != 0:
				try:
					include_dom = parse(include_name)
				except:
					include_dom = parse(dir + include_name)
				for child in include_dom.documentElement.childNodes:
					if xml.dom.Node.ELEMENT_NODE == child.nodeType:
						dom.documentElement.insertBefore(child, include_file)
			dom.documentElement.removeChild(include_file)

		include_files = dom.documentElement.getElementsByTagName('include')

	return dom
			

if __name__ == '__main__':
	input_name = 'sample.fxml'
	output_name = 'output.kfx'

	import sys
	if len(sys.argv) >= 2:
		input_name = sys.argv[1]
		if len(sys.argv) < 3:
			for i in range(0, len(input_name) - 1):
				if input_name[len(input_name) - 1 - i] == '.':
					output_name = input_name[0 : len(input_name) - 1 - i] + ".kfx"
		else:
			output_name = sys.argv[2]

	input_name = input_name.replace('\\', '/')
	output_name = output_name.replace('\\', '/')

	dom = preprocess(input_name);
	fx = effect(dom.documentElement)
	fx.write(open(output_name, 'wb'))
