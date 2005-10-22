#!/usr/bin/env python
#-*- coding: mbcs -*-

class point2:
	def __init__(self, x, y):
		self.x = float(x)
		self.y = float(y)

class point3:
	def __init__(self, x, y, z):
		self.x = float(x)
		self.y = float(y)
		self.z = float(z)

class vertex:
	def __init__(self, pos, normal, texs):
		self.pos = pos
		self.normal = normal
		self.texs = texs

class triangle:
	def __init__(self, a, b, c):
		self.a = int(a)
		self.b = int(b)
		self.c = int(c)

class texture:
	def __init__(self, type, name):
		self.type = type
		self.name = name

class mesh:
	def __init__(self, name, textures, vertices, triangles):
		self.name = name

		self.textures = textures
		self.vertices = vertices
		self.triangles = triangles

class bone:
	def __init__(self, name, parent, bind_pos, bind_quat):
		self.name = str(name)
		self.parent = int(parent)

		self.bind_pos = bind_pos
		self.bind_quat = bind_quat

class model:
	def __init__(self):
		self.version = 0

		self.bones = []
		self.meshes = []

def parse_model(dom):
	ret = model()

	ret.version = int(dom.documentElement.getAttribute('version'))

	meshes_chunk_tag = dom.documentElement.getElementsByTagName('meshes_chunk')[0]
	mesh_tags = meshes_chunk_tag.getElementsByTagName('mesh')
	for mesh_tag in mesh_tags:
		name = mesh_tag.getAttribute('name')

		textures = []
		textures_chunk_tag = mesh_tag.getElementsByTagName('textures_chunk')[0]
		texture_tags = textures_chunk_tag.getElementsByTagName('texture')
		for texture_tag in texture_tags:
			textures.append(texture(texture_tag.getAttribute('type'), texture_tag.getAttribute('name')))

		vertices = []
		vertices_chunk_tag = mesh_tag.getElementsByTagName('vertices_chunk')[0]
		vertex_tags = vertices_chunk_tag.getElementsByTagName('vertex')
		for vertex_tag in vertex_tags:
			pos = point3(vertex_tag.getAttribute('x'), vertex_tag.getAttribute('y'), vertex_tag.getAttribute('z'))

			normal_tags = vertex_tag.getElementsByTagName('normal')
			normal = point3(normal_tags[0].getAttribute('x'), normal_tags[0].getAttribute('y'), normal_tags[0].getAttribute('z'))

			texs = []
			tex_coord_tags = vertex_tag.getElementsByTagName('tex_coord')
			for tex_coord_tag in tex_coord_tags:
				texs.append(point2(tex_coord_tag.getAttribute('u'), tex_coord_tag.getAttribute('v')))

			vertices.append(vertex(pos, normal, texs))

		triangles = []
		triangles_chunk_tag = mesh_tag.getElementsByTagName('triangles_chunk')[0]
		triangle_tags = triangles_chunk_tag.getElementsByTagName('triangle')
		for triangle_tag in triangle_tags:
			triangles.append(triangle(triangle_tag.getAttribute('a'), triangle_tag.getAttribute('b'), triangle_tag.getAttribute('c')))

		ret.meshes.append(mesh(name, textures, vertices, triangles))

	return ret


if __name__ == '__main__':
	import sys

	if len(sys.argv) < 3:
		print "Usage: meshml2kmesh.py src.meshml dst.kmesh"
		sys.exit()

	in_file_name = sys.argv[1]
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
		print "Compiling:", mesh.name

		ofs.write(pack('B', len(mesh.name)))
		ofs.write(mesh.name)

		ofs.write(pack('B', len(mesh.textures)))
		for texture in mesh.textures:
			ofs.write(pack('B', len(texture.type)))
			ofs.write(texture.type)
			ofs.write(pack('B', len(texture.name)))
			ofs.write(texture.name)

		ofs.write(pack('L', len(mesh.vertices)))
		ofs.write(pack('L', len(mesh.vertices[0].texs)))
		for vertex in mesh.vertices:
			ofs.write(pack('fff', vertex.pos.x, vertex.pos.y, vertex.pos.z))
			ofs.write(pack('fff', vertex.normal.x, vertex.normal.y, vertex.normal.z))
			for tex in vertex.texs:
				ofs.write(pack('ff', tex.x, tex.y))

		ofs.write(pack('L', len(mesh.triangles)))
		for triangle in mesh.triangles:
			ofs.write(pack('HHH', triangle.a, triangle.b, triangle.c))

	print "Done"

