#!/usr/bin/env python
#-*- coding: utf-8 -*-

from __future__ import print_function
from struct import pack
from sys import	getfilesystemencoding
encoding = getfilesystemencoding()

VEU_Position = 0
VEU_Normal = 1
VEU_Diffuse = 2
VEU_Specular = 3
VEU_BlendWeight = 4
VEU_BlendIndex = 5
VEU_TextureCoord = 6
VEU_Tangent = 7
VEU_Binormal = 8

class vertex_element:
	def __init__(self, usage, usage_index, num_components):
		self.usage = int(usage)
		self.usage_index = int(usage_index)
		self.num_components = int(num_components)

class model:
	def __init__(self, dom):
		self.dom = dom

		self.version = int(dom.documentElement.getAttribute('version'))
		if self.version != 3:
			print("model version must be 3")
			raise

		self.num_joints = 0
		if len(dom.documentElement.getElementsByTagName('bones_chunk')) > 0:
			bones_chunk_tag = dom.documentElement.getElementsByTagName('bones_chunk')[0]
			bone_tags = bones_chunk_tag.getElementsByTagName('bone')
			self.num_joints = len(bone_tags)

		meshes_chunk_tag = dom.documentElement.getElementsByTagName('meshes_chunk')[0]
		mesh_tags = meshes_chunk_tag.getElementsByTagName('mesh')
		self.num_meshes = len(mesh_tags)

		self.start_frame = 0
		self.end_frame = 0
		self.frame_rate = 0
		self.num_key_frames = 0
		if len(dom.documentElement.getElementsByTagName('key_frames_chunk')) > 0:
			key_frames_chunk_tag = dom.documentElement.getElementsByTagName('key_frames_chunk')[0]
			self.start_frame = int(key_frames_chunk_tag.getAttribute('start_frame'))
			self.end_frame = int(key_frames_chunk_tag.getAttribute('end_frame'))
			self.frame_rate = int(key_frames_chunk_tag.getAttribute('frame_rate'))

			key_frame_tags = key_frames_chunk_tag.getElementsByTagName('key_frame')
			self.num_key_frames = len(key_frame_tags)

	def compile_meshes(self, stream):
		meshes_chunk_tag = self.dom.documentElement.getElementsByTagName('meshes_chunk')[0]
		mesh_tags = meshes_chunk_tag.getElementsByTagName('mesh')
		for mesh_tag in mesh_tags:
			name = mesh_tag.getAttribute('name').encode(encoding)
			
			print("Compiling mesh:", name)

			stream.write(pack('B', len(name)))
			stream.write(name)

			vertex_elems = []
			vertex_elems_chunk_tag = mesh_tag.getElementsByTagName('vertex_elements_chunk')[0]
			vertex_elem_tags = vertex_elems_chunk_tag.getElementsByTagName('vertex_element')
			stream.write(pack('B', len(vertex_elem_tags)))
			for vertex_elem_tag in vertex_elem_tags:
				ve = vertex_element(vertex_elem_tag.getAttribute('usage'), vertex_elem_tag.getAttribute('usage_index'), vertex_elem_tag.getAttribute('num_components'))
				vertex_elems.append(ve)
				stream.write(pack('BBB', ve.usage, ve.usage_index, ve.num_components))

			textures_chunk_tag = mesh_tag.getElementsByTagName('textures_chunk')[0]
			texture_tags = textures_chunk_tag.getElementsByTagName('texture')
			stream.write(pack('B', len(texture_tags)))
			for texture_tag in texture_tags:
				texture_type = texture_tag.getAttribute('type').encode(encoding)
				texture_name = texture_tag.getAttribute('name').encode(encoding)
				stream.write(pack('B', len(texture_type)))
				stream.write(texture_type)
				stream.write(pack('B', len(texture_name)))
				stream.write(texture_name)

			vertices_chunk_tag = mesh_tag.getElementsByTagName('vertices_chunk')[0]
			vertex_tags = vertices_chunk_tag.getElementsByTagName('vertex')
			stream.write(pack('L', len(vertex_tags)))
			for vertex_elem in vertex_elems:
				if (VEU_Position == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						stream.write(pack('fff', float(vertex_tag.getAttribute('x')), float(vertex_tag.getAttribute('y')), float(vertex_tag.getAttribute('z'))))
				elif (VEU_Normal == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						normal_tag = vertex_tag.getElementsByTagName('normal')[0]
						stream.write(pack('fff', float(normal_tag.getAttribute('x')), float(normal_tag.getAttribute('y')), float(normal_tag.getAttribute('z'))))
				elif (VEU_Diffuse == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						diffuse_tag = vertex_tag.getElementsByTagName('diffuse')[0]
						stream.write(pack('ffff', float(diffuse_tag.getAttribute('r')), float(diffuse_tag.getAttribute('g')), float(diffuse_tag.getAttribute('b')), float(diffuse_tag.getAttribute('a'))))
				elif (VEU_Specular == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						specular_tag = vertex_tag.getElementsByTagName('specular')[0]
						stream.write(pack('ffff', float(specular_tag.getAttribute('r')), float(specular_tag.getAttribute('g')), float(specular_tag.getAttribute('b')), float(specular_tag.getAttribute('a'))))
				elif (VEU_BlendIndex == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						weight_tags = vertex_tag.getElementsByTagName('weight')
						for weight_tag in weight_tags:
							stream.write(pack('B', int(weight_tag.getAttribute('bone_index'))))
				elif (VEU_BlendWeight == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						weight_tags = vertex_tag.getElementsByTagName('weight')
						for weight_tag in weight_tags:
							stream.write(pack('f', float(weight_tag.getAttribute('weight'))))
				elif VEU_TextureCoord == vertex_elem.usage:
					for vertex_tag in vertex_tags:
						tex_coord_tag = vertex_tag.getElementsByTagName('tex_coord')[vertex_elem.usage_index]
						if 1 == vertex_elem.num_components:
							stream.write(pack('f', float(tex_coord_tag.getAttribute('u'))))
						elif 2 == vertex_elem.num_components:
							stream.write(pack('ff', float(tex_coord_tag.getAttribute('u')), float(tex_coord_tag.getAttribute('v'))))
						elif 3 == vertex_elem.num_components:
							stream.write(pack('fff', float(tex_coord_tag.getAttribute('u')), float(tex_coord_tag.getAttribute('v')), float(tex_coord_tag.getAttribute('w'))))
				elif (VEU_Tangent == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						tangent_tag = vertex_tag.getElementsByTagName('tangent')[0]
						stream.write(pack('fff', float(tangent_tag.getAttribute('x')), float(tangent_tag.getAttribute('y')), float(tangent_tag.getAttribute('z'))))
				elif (VEU_Binormal == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						binormal_tag = vertex_tag.getElementsByTagName('binormal')[0]
						stream.write(pack('fff', float(binormal_tag.getAttribute('x')), float(binormal_tag.getAttribute('y')), float(binormal_tag.getAttribute('z'))))

			triangles_chunk_tag = mesh_tag.getElementsByTagName('triangles_chunk')[0]
			triangle_tags = triangles_chunk_tag.getElementsByTagName('triangle')
			stream.write(pack('L', len(triangle_tags)))
			for triangle_tag in triangle_tags:
				stream.write(pack('HHH', int(triangle_tag.getAttribute('a')), int(triangle_tag.getAttribute('b')), int(triangle_tag.getAttribute('c'))))

	def compile_joints(self, stream):
		if len(self.dom.documentElement.getElementsByTagName('bones_chunk')) > 0:
			bones_chunk_tag = self.dom.documentElement.getElementsByTagName('bones_chunk')[0]
			bone_tags = bones_chunk_tag.getElementsByTagName('bone')
			for bone_tag in bone_tags:
				joint_name = bone_tag.getAttribute('name').encode(encoding)

				print("Compiling joint:", joint_name)

				stream.write(pack('B', len(joint_name)))
				stream.write(joint_name)

				stream.write(pack('h', int(bone_tag.getAttribute('parent'))))
				bind_pos_tag = bone_tag.getElementsByTagName('bind_pos')[0]
				stream.write(pack('fff', float(bind_pos_tag.getAttribute('x')), float(bind_pos_tag.getAttribute('y')), float(bind_pos_tag.getAttribute('z'))))
				bind_quat_tag = bone_tag.getElementsByTagName('bind_quat')[0]
				stream.write(pack('ffff', float(bind_quat_tag.getAttribute('x')), float(bind_quat_tag.getAttribute('y')), float(bind_quat_tag.getAttribute('z')), float(bind_quat_tag.getAttribute('w'))))

	def compile_key_frames(self, stream):
		if len(self.dom.documentElement.getElementsByTagName('key_frames_chunk')) > 0:
			key_frames_chunk_tag = self.dom.documentElement.getElementsByTagName('key_frames_chunk')[0]

			key_frame_tags = key_frames_chunk_tag.getElementsByTagName('key_frame')
			for key_frame_tag in key_frame_tags:
				joint_name = key_frame_tag.getAttribute('joint').encode(encoding)

				print("Compiling key frame:", joint_name)

				stream.write(pack('B', len(joint_name)))
				stream.write(joint_name)

				key_tags = key_frame_tag.getElementsByTagName('key')
				for key_tag in key_tags:
					pos_tag = key_tag.getElementsByTagName('pos')[0]
					stream.write(pack('fff', float(pos_tag.getAttribute('x')), float(pos_tag.getAttribute('y')), float(pos_tag.getAttribute('z'))))
				for key_tag in key_tags:
					quat_tag = key_tag.getElementsByTagName('quat')[0]
					stream.write(pack('ffff', float(quat_tag.getAttribute('x')), float(quat_tag.getAttribute('y')), float(quat_tag.getAttribute('z')), float(quat_tag.getAttribute('w'))))

	def compile(self, stream):
		stream.write('MESH')

		header_size = 4 + 1 + 1 + 1 + 4 + 4 + 4
		stream.write(pack('L', header_size))

		stream.write(pack('L', self.version))

		print("Model version:", self.version)

		stream.write(pack('B', self.num_meshes))
		stream.write(pack('B', self.num_joints))
		stream.write(pack('B', self.num_key_frames))
		stream.write(pack('L', self.start_frame))
		stream.write(pack('L', self.end_frame))
		stream.write(pack('L', self.frame_rate))

		self.compile_meshes(stream)
		self.compile_joints(stream)
		self.compile_key_frames(stream)

if __name__ == '__main__':
	import sys

	if len(sys.argv) < 2:
		print("Usage: meshml2kmesh.py src.meshml [dst.kmodel]")
		sys.exit()

	in_file_name = sys.argv[1]	
	if len(sys.argv) < 3:
		index = in_file_name.rfind('.')
		if index != -1:
			out_file_name = in_file_name[0 : index] + ".kmodel"
		else:
			out_file_name = in_file_name + ".kmodel"
	else:
		out_file_name = sys.argv[2]

	print("Parsing:", in_file_name)

	from xml.dom.minidom import parse
	kmodel = model(parse(in_file_name))

	kmodel.compile(open(out_file_name, 'wb'))

	print("DONE!!")

