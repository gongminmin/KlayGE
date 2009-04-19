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
	def __init__(self, root):
		self.root = root

		self.version = int(root.attrib['version'])
		if self.version != 4:
			print("model version must be 4")
			raise

		self.num_joints = 0
		if len(root.findall('bones_chunk')) > 0:
			bone_tags = root.findall('bones_chunk/bone')
			self.num_joints = len(bone_tags)

		material_tags = root.findall('materials_chunk/material')
		self.num_materials = len(material_tags)

		mesh_tags = root.findall('meshes_chunk/mesh')
		self.num_meshes = len(mesh_tags)

		self.start_frame = 0
		self.end_frame = 0
		self.frame_rate = 0
		self.num_key_frames = 0
		if len(root.findall('key_frames_chunk')) > 0:
			key_frames_chunk_tag = root.find('key_frames_chunk')
			self.start_frame = int(key_frames_chunk_tag.attrib['start_frame'])
			self.end_frame = int(key_frames_chunk_tag.attrib['end_frame'])
			self.frame_rate = int(key_frames_chunk_tag.attrib['frame_rate'])

			key_frame_tags = key_frames_chunk_tag.findall('key_frame')
			self.num_key_frames = len(key_frame_tags)

	def compile_materials(self, stream):
		material_tags = self.root.findall('materials_chunk/material')
		for material_tag in material_tags:
			stream.write(pack('fff', float(material_tag.attrib['ambient_r']), float(material_tag.attrib['ambient_g']), float(material_tag.attrib['ambient_b'])))
			stream.write(pack('fff', float(material_tag.attrib['diffuse_r']), float(material_tag.attrib['diffuse_g']), float(material_tag.attrib['diffuse_b'])))
			stream.write(pack('fff', float(material_tag.attrib['specular_r']), float(material_tag.attrib['specular_g']), float(material_tag.attrib['specular_b'])))
			stream.write(pack('fff', float(material_tag.attrib['emit_r']), float(material_tag.attrib['emit_g']), float(material_tag.attrib['emit_b'])))
			stream.write(pack('fff', float(material_tag.attrib['opacity']), float(material_tag.attrib['specular_level']), float(material_tag.attrib['shininess'])))

			texture_tags = material_tag.findall('textures_chunk/texture')
			stream.write(pack('B', len(texture_tags)))
			for texture_tag in texture_tags:
				texture_type = texture_tag.attrib['type'].encode(encoding)
				texture_name = texture_tag.attrib['name'].encode(encoding)
				stream.write(pack('B', len(texture_type)))
				stream.write(texture_type)
				stream.write(pack('B', len(texture_name)))
				stream.write(texture_name)

	def compile_meshes(self, stream):
		mesh_tags = self.root.findall('meshes_chunk/mesh')
		for mesh_tag in mesh_tags:
			name = mesh_tag.attrib['name'].encode(encoding)

			print("Compiling mesh:", name)

			stream.write(pack('B', len(name)))
			stream.write(name)

			stream.write(pack('L', int(mesh_tag.attrib['mtl_id'])))

			vertex_elems = []
			vertex_elem_tags = mesh_tag.findall('vertex_elements_chunk/vertex_element')
			stream.write(pack('B', len(vertex_elem_tags)))
			for vertex_elem_tag in vertex_elem_tags:
				ve = vertex_element(vertex_elem_tag.attrib['usage'], vertex_elem_tag.attrib['usage_index'], vertex_elem_tag.attrib['num_components'])
				vertex_elems.append(ve)
				stream.write(pack('BBB', ve.usage, ve.usage_index, ve.num_components))

			vertex_tags = mesh_tag.findall('vertices_chunk/vertex')
			stream.write(pack('L', len(vertex_tags)))
			for vertex_elem in vertex_elems:
				if (VEU_Position == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						stream.write(pack('fff', float(vertex_tag.attrib['x']), float(vertex_tag.attrib['y']), float(vertex_tag.attrib['z'])))
				elif (VEU_Normal == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						normal_tag = vertex_tag.find('normal')
						stream.write(pack('fff', float(normal_tag.attrib['x']), float(normal_tag.attrib['y']), float(normal_tag.attrib['z'])))
				elif (VEU_Diffuse == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						diffuse_tag = vertex_tag.find('diffuse')
						stream.write(pack('ffff', float(diffuse_tag.attrib['r']), float(diffuse_tag.attrib['g']), float(diffuse_tag.attrib['b']), float(diffuse_tag.attrib['a'])))
				elif (VEU_Specular == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						specular_tag = vertex_tag.find('specular')
						stream.write(pack('ffff', float(specular_tag.attrib['r']), float(specular_tag.attrib['g']), float(specular_tag.attrib['b']), float(specular_tag.attrib['a'])))
				elif (VEU_BlendIndex == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						weight_tags = vertex_tag.findall('weight')
						for weight_tag in weight_tags:
							stream.write(pack('B', int(weight_tag.attrib['bone_index'])))
				elif (VEU_BlendWeight == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						weight_tags = vertex_tag.findall('weight')
						for weight_tag in weight_tags:
							stream.write(pack('f', float(weight_tag.attrib['weight'])))
				elif VEU_TextureCoord == vertex_elem.usage:
					for vertex_tag in vertex_tags:
						tex_coord_tag = vertex_tag.findall('tex_coord')[vertex_elem.usage_index]
						if 1 == vertex_elem.num_components:
							stream.write(pack('f', float(tex_coord_tag.attrib['u'])))
						elif 2 == vertex_elem.num_components:
							stream.write(pack('ff', float(tex_coord_tag.attrib['u']), float(tex_coord_tag.attrib['v'])))
						elif 3 == vertex_elem.num_components:
							stream.write(pack('fff', float(tex_coord_tag.attrib['u']), float(tex_coord_tag.attrib['v']), float(tex_coord_tag.attrib['w'])))
				elif (VEU_Tangent == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						tangent_tag = vertex_tag.find('tangent')
						stream.write(pack('fff', float(tangent_tag.attrib['x']), float(tangent_tag.attrib['y']), float(tangent_tag.attrib['z'])))
				elif (VEU_Binormal == vertex_elem.usage):
					for vertex_tag in vertex_tags:
						binormal_tag = vertex_tag.find('binormal')
						stream.write(pack('fff', float(binormal_tag.attrib['x']), float(binormal_tag.attrib['y']), float(binormal_tag.attrib['z'])))

			triangle_tags = mesh_tag.findall('triangles_chunk/triangle')
			stream.write(pack('L', len(triangle_tags)))
			for triangle_tag in triangle_tags:
				stream.write(pack('HHH', int(triangle_tag.attrib['a']), int(triangle_tag.attrib['b']), int(triangle_tag.attrib['c'])))

	def compile_joints(self, stream):
		if len(self.root.findall('bones_chunk')) > 0:
			bone_tags = self.root.findall('bones_chunk/bone')
			for bone_tag in bone_tags:
				joint_name = bone_tag.attrib['name'].encode(encoding)

				print("Compiling joint:", joint_name)

				stream.write(pack('B', len(joint_name)))
				stream.write(joint_name)

				stream.write(pack('h', int(bone_tag.attrib['parent'])))
				bind_pos_tag = bone_tag.find('bind_pos')
				stream.write(pack('fff', float(bind_pos_tag.attrib['x']), float(bind_pos_tag.attrib['y']), float(bind_pos_tag.attrib['z'])))
				bind_quat_tag = bone_tag.find('bind_quat')
				stream.write(pack('ffff', float(bind_quat_tag.attrib['x']), float(bind_quat_tag.attrib['y']), float(bind_quat_tag.attrib['z']), float(bind_quat_tag.attrib['w'])))

	def compile_key_frames(self, stream):
		if len(self.root.findall('key_frames_chunk')) > 0:
			key_frame_tags = self.root.findall('key_frames_chunk/key_frame')
			for key_frame_tag in key_frame_tags:
				joint_name = key_frame_tag.attrib['joint'].encode(encoding)

				print("Compiling key frame:", joint_name)

				stream.write(pack('B', len(joint_name)))
				stream.write(joint_name)

				key_tags = key_frame_tag.findall('key')
				for key_tag in key_tags:
					pos_tag = key_tag.find('pos')
					stream.write(pack('fff', float(pos_tag.attrib['x']), float(pos_tag.attrib['y']), float(pos_tag.attrib['z'])))
				for key_tag in key_tags:
					quat_tag = key_tag.find('quat')
					stream.write(pack('ffff', float(quat_tag.attrib['x']), float(quat_tag.attrib['y']), float(quat_tag.attrib['z']), float(quat_tag.attrib['w'])))

	def compile(self, stream):
		if 3 == sys.version_info[0]:
			stream.write(b'MESH')
		else:
			stream.write('MESH')

		header_size = 4 + 1 + 1 + 1 + 4 + 4 + 4
		stream.write(pack('L', header_size))

		stream.write(pack('L', self.version))

		print("Model version:", self.version)

		stream.write(pack('B', self.num_materials))
		stream.write(pack('B', self.num_meshes))
		stream.write(pack('B', self.num_joints))
		stream.write(pack('B', self.num_key_frames))
		stream.write(pack('L', self.start_frame))
		stream.write(pack('L', self.end_frame))
		stream.write(pack('L', self.frame_rate))

		self.compile_materials(stream)
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

	from xml.etree.cElementTree import ElementTree

	import time
	start = time.time()

	kmodel = model(ElementTree().parse(in_file_name))
	kmodel.compile(open(out_file_name, 'wb'))

	print("Timing: %.2fs" % (time.time() - start))

	print("DONE!!")

