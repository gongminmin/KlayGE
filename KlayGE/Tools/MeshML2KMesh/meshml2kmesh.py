#!/usr/bin/env python
#-*- coding: mbcs -*-

VEU_Position = 0
VEU_Normal = 1
VEU_Diffuse = 2
VEU_Specular = 3
VEU_BlendWeight = 4
VEU_BlendIndex = 5
VEU_TextureCoord = 6
VEU_Tangent = 7
VEU_Binormal = 8

class point1:
	def __init__(self, x):
		self.x = float(x)

class point2:
	def __init__(self, x, y):
		self.x = float(x)
		self.y = float(y)

class point3:
	def __init__(self, x, y, z):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)

class point4:
	def __init__(self, x, y, z, w):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)
		self.w = float(w)

class vertex:
	def __init__(self):
		self.pos = 0
		self.normal = 0
		self.diffuse = 0
		self.specular = 0
		self.blend_indices = 0
		self.blend_weights = 0
		self.texs = 0
		self.tangent = 0
		self.binormal = 0

class triangle:
	def __init__(self, a, b, c):
		self.a = int(a)
		self.b = int(b)
		self.c = int(c)

class vertex_element:
	def __init__(self, usage, usage_index, num_components):
		self.usage = int(usage)
		self.usage_index = int(usage_index)
		self.num_components = int(num_components)

class texture:
	def __init__(self, type, name):
		self.type = type
		self.name = name

class mesh:
	def __init__(self, name, vertex_elems, textures, vertices, triangles):
		self.name = name

		self.vertex_elems = vertex_elems
		self.textures = textures
		self.vertices = vertices
		self.triangles = triangles

class bone:
	def __init__(self, name, parent, bind_pos, bind_quat):
		self.name = str(name)
		self.parent = int(parent)

		self.bind_pos = bind_pos
		self.bind_quat = bind_quat

class key:
	def __init__(self, pos, quat):
		self.pos = pos
		self.quat = quat

class model:
	def __init__(self):
		self.version = 0

		self.bones = []
		self.meshes = []

		self.start_frame = 0
		self.end_frame = 0
		self.frame_rate = 0
		self.key_frames = []

def parse_model(dom):
	ret = model()

	ret.version = int(dom.documentElement.getAttribute('version'))
	if ret.version != 3:
		print "model version must be 3"
		raise

	if len(dom.documentElement.getElementsByTagName('bones_chunk')) > 0:
		bones_chunk_tag = dom.documentElement.getElementsByTagName('bones_chunk')[0]
		bone_tags = bones_chunk_tag.getElementsByTagName('bone')
		for bone_tag in bone_tags:
			name = bone_tag.getAttribute('name')
			parent = bone_tag.getAttribute('parent')

			bind_pos_tag = bone_tag.getElementsByTagName('bind_pos')[0]
			bind_pos = point3(bind_pos_tag.getAttribute('x'), bind_pos_tag.getAttribute('y'), bind_pos_tag.getAttribute('z'))
			bind_quat_tag = bone_tag.getElementsByTagName('bind_quat')[0]
			bind_quat = point4(bind_quat_tag.getAttribute('x'), bind_quat_tag.getAttribute('y'), bind_quat_tag.getAttribute('z'), bind_quat_tag.getAttribute('w'))

			ret.bones.append(bone(name, parent, bind_pos, bind_quat))

	meshes_chunk_tag = dom.documentElement.getElementsByTagName('meshes_chunk')[0]
	mesh_tags = meshes_chunk_tag.getElementsByTagName('mesh')
	for mesh_tag in mesh_tags:
		name = mesh_tag.getAttribute('name')

		vertex_elems = []
		vertex_elems_chunk_tag = mesh_tag.getElementsByTagName('vertex_elements_chunk')[0]
		vertex_elem_tags = vertex_elems_chunk_tag.getElementsByTagName('vertex_element')
		for vertex_elem_tag in vertex_elem_tags:
			vertex_elems.append(vertex_element(vertex_elem_tag.getAttribute('usage'), vertex_elem_tag.getAttribute('usage_index'), vertex_elem_tag.getAttribute('num_components')))

		textures = []
		textures_chunk_tag = mesh_tag.getElementsByTagName('textures_chunk')[0]
		texture_tags = textures_chunk_tag.getElementsByTagName('texture')
		for texture_tag in texture_tags:
			textures.append(texture(texture_tag.getAttribute('type'), texture_tag.getAttribute('name')))

		vertices = []
		vertices_chunk_tag = mesh_tag.getElementsByTagName('vertices_chunk')[0]
		vertex_tags = vertices_chunk_tag.getElementsByTagName('vertex')
		for vertex_tag in vertex_tags:
			v = vertex()

			for vertex_elem in vertex_elems:
				if (VEU_Position == vertex_elem.usage):
					v.pos = point3(vertex_tag.getAttribute('x'), vertex_tag.getAttribute('y'), vertex_tag.getAttribute('z'))
				elif (VEU_Normal == vertex_elem.usage):
					normal_tags = vertex_tag.getElementsByTagName('normal')
					v.normal = point3(normal_tags[0].getAttribute('x'), normal_tags[0].getAttribute('y'), normal_tags[0].getAttribute('z'))
				elif (VEU_Diffuse == vertex_elem.usage):
					diffuse_tags = vertex_tag.getElementsByTagName('diffuse')
					v.diffuse = point4(diffuse_tags[0].getAttribute('r'), diffuse_tags[0].getAttribute('g'), diffuse_tags[0].getAttribute('b'), diffuse_tags[0].getAttribute('a'))
				elif (VEU_Specular == vertex_elem.usage):
					specular_tags = vertex_tag.getElementsByTagName('specular')
					v.specular = point4(specular_tags[0].getAttribute('r'), specular_tags[0].getAttribute('g'), specular_tags[0].getAttribute('b'), specular_tags[0].getAttribute('a'))
				elif (VEU_BlendWeight == vertex_elem.usage):
					v.blend_indices = []
					v.blend_weights = []
					weight_tags = vertex_tag.getElementsByTagName('weight')
					for weight_tag in weight_tags:
						v.blend_indices.append(int(weight_tag.getAttribute('bone_index')))
						v.blend_weights.append(float(weight_tag.getAttribute('weight')))
				elif VEU_TextureCoord == vertex_elem.usage:
					v.texs = []
					tex_coord_tags = vertex_tag.getElementsByTagName('tex_coord')
					for tex_coord_tag in tex_coord_tags:
						if 1 == vertex_elem.num_components:
							v.texs.append(point1(tex_coord_tag.getAttribute('u')))
						elif 2 == vertex_elem.num_components:
							v.texs.append(point2(tex_coord_tag.getAttribute('u'), tex_coord_tag.getAttribute('v')))
						elif 3 == vertex_elem.num_components:
							v.texs.append(point3(tex_coord_tag.getAttribute('u'), tex_coord_tag.getAttribute('v'), tex_coord_tag.getAttribute('w')))
				elif (VEU_Tangent == vertex_elem.usage):
					tangent_tags = vertex_tag.getElementsByTagName('tangent')
					v.tangent = point3(tangent_tags[0].getAttribute('x'), tangent_tags[0].getAttribute('y'), tangent_tags[0].getAttribute('z'))
				elif (VEU_Binormal == vertex_elem.usage):
					binormal_tags = vertex_tag.getElementsByTagName('binormal')
					v.binormal = point3(binormal_tags[0].getAttribute('x'), binormal_tags[0].getAttribute('y'), binormal_tags[0].getAttribute('z'))

			vertices.append(v)

		triangles = []
		triangles_chunk_tag = mesh_tag.getElementsByTagName('triangles_chunk')[0]
		triangle_tags = triangles_chunk_tag.getElementsByTagName('triangle')
		for triangle_tag in triangle_tags:
			triangles.append(triangle(triangle_tag.getAttribute('a'), triangle_tag.getAttribute('b'), triangle_tag.getAttribute('c')))

		ret.meshes.append(mesh(name, vertex_elems, textures, vertices, triangles))

	if len(dom.documentElement.getElementsByTagName('key_frames_chunk')) > 0:
		key_frames_chunk_tag = dom.documentElement.getElementsByTagName('key_frames_chunk')[0]
		ret.start_frame = int(key_frames_chunk_tag.getAttribute('start_frame'))
		ret.end_frame = int(key_frames_chunk_tag.getAttribute('end_frame'))
		ret.frame_rate = int(key_frames_chunk_tag.getAttribute('frame_rate'))

		key_frame_tags = key_frames_chunk_tag.getElementsByTagName('key_frame')
		for key_frame_tag in key_frame_tags:
			joint = key_frame_tag.getAttribute('joint')

			keys = []
			key_tags = key_frame_tag.getElementsByTagName('key')
			for key_tag in key_tags:
				pos_tag = key_tag.getElementsByTagName('pos')[0]
				pos = point3(pos_tag.getAttribute('x'), pos_tag.getAttribute('y'), pos_tag.getAttribute('z'))
				quat_tag = key_tag.getElementsByTagName('quat')[0]
				quat = point4(quat_tag.getAttribute('x'), quat_tag.getAttribute('y'), quat_tag.getAttribute('z'), quat_tag.getAttribute('w'))

				keys.append(key(pos, quat))

			ret.key_frames.append([joint, keys])

	return ret


if __name__ == '__main__':
	import sys

	if len(sys.argv) < 2:
		print "Usage: meshml2kmesh.py src.meshml [dst.kmodel]"
		sys.exit()

	in_file_name = sys.argv[1]	
	if len(sys.argv) == 2:
		index = in_file_name.rfind('.')
		if index != -1:
			out_file_name = in_file_name[0 : index] + ".kmodel"
		else:
			out_file_name = in_file_name + ".kmodel"
	else:
		out_file_name = sys.argv[2]

	print "Prasing:", in_file_name
	from xml.dom.minidom import parse
	model = parse_model(parse(in_file_name))

	ofs = open(out_file_name, 'wb')
	ofs.write('MESH')

	from struct import pack
	ofs.write(pack('L', model.version))

	print "Model version:", model.version

	ofs.write(pack('B', len(model.meshes)))
	for mesh in model.meshes:
		print "Compiling mesh:", mesh.name

		ofs.write(pack('B', len(mesh.name)))
		ofs.write(mesh.name)

		ofs.write(pack('B', len(mesh.vertex_elems)))
		for vertex_elem in mesh.vertex_elems:
			ofs.write(pack('BBB', vertex_elem.usage, vertex_elem.usage_index, vertex_elem.num_components))

		ofs.write(pack('B', len(mesh.textures)))
		for texture in mesh.textures:
			ofs.write(pack('B', len(texture.type)))
			ofs.write(texture.type)
			ofs.write(pack('B', len(texture.name)))
			ofs.write(texture.name)

		ofs.write(pack('L', len(mesh.vertices)))
		ofs.write(pack('L', len(mesh.vertices[0].texs)))
		for vertex_elem in mesh.vertex_elems:
			if (VEU_Position == vertex_elem.usage):
				for vertex in mesh.vertices:
					ofs.write(pack('fff', vertex.pos.x, vertex.pos.y, vertex.pos.z))
			elif (VEU_Normal == vertex_elem.usage):
				for vertex in mesh.vertices:
					ofs.write(pack('fff', vertex.normal.x, vertex.normal.y, vertex.normal.z))
			elif (VEU_Diffuse == vertex_elem.usage):
				for vertex in mesh.vertices:
					ofs.write(pack('ffff', vertex.diffuse.x, vertex.diffuse.y, vertex.diffuse.z, vertex.diffuse.w))
			elif (VEU_Specular == vertex_elem.usage):
				for vertex in mesh.vertices:
					ofs.write(pack('ffff', vertex.specular.x, vertex.specular.y, vertex.specular.z, vertex.specular.w))
			elif (VEU_BlendIndex == vertex_elem.usage):
				for vertex in mesh.vertices:
					for index in vertex.blend_indices:
						ofs.write(pack('B', index))
			elif (VEU_BlendWeight == vertex_elem.usage):
				for vertex in mesh.vertices:
					for weight in vertex.blend_weights:
						ofs.write(pack('f', weight))
			elif VEU_TextureCoord == vertex_elem.usage:
				for vertex in mesh.vertices:
					tex = vertex.texs[vertex_elem.usage_index]
					if 1 == vertex_elem.num_components:
						ofs.write(pack('f', tex.x))
					elif 2 == vertex_elem.num_components:
						ofs.write(pack('ff', tex.x, tex.y))
					elif 3 == vertex_elem.num_components:
						ofs.write(pack('fff', tex.x, tex.y, tex.z))
			elif (VEU_Tangent == vertex_elem.usage):
				for vertex in mesh.vertices:
					ofs.write(pack('fff', vertex.tangent.x, vertex.tangent.y, vertex.tangent.z))
			elif (VEU_Binormal == vertex_elem.usage):
				for vertex in mesh.vertices:
					ofs.write(pack('fff', vertex.binormal.x, vertex.binormal.y, vertex.binormal.z))

		ofs.write(pack('L', len(mesh.triangles)))
		for triangle in mesh.triangles:
			ofs.write(pack('HHH', triangle.a, triangle.b, triangle.c))

	ofs.write(pack('B', len(model.bones)))
	for bone in model.bones:
		print "Compiling bone:", bone.name

		ofs.write(pack('B', len(bone.name)))
		ofs.write(bone.name)

		ofs.write(pack('h', bone.parent))
		ofs.write(pack('fff', bone.bind_pos.x, bone.bind_pos.y, bone.bind_pos.z))
		ofs.write(pack('ffff', bone.bind_quat.x, bone.bind_quat.y, bone.bind_quat.z, bone.bind_quat.w))

	ofs.write(pack('B', len(model.key_frames)))
	ofs.write(pack('L', model.start_frame))
	ofs.write(pack('L', model.end_frame))
	ofs.write(pack('L', model.frame_rate))
	for key_frame in model.key_frames:
		print "Compiling key frame:", key_frame[0]

		ofs.write(pack('B', len(key_frame[0])))
		ofs.write(key_frame[0])

		for key in key_frame[1]:
			ofs.write(pack('fff', key.pos.x, key.pos.y, key.pos.z))
			ofs.write(pack('ffff', key.quat.x, key.quat.y, key.quat.z, key.quat.w))

	print "Done"

