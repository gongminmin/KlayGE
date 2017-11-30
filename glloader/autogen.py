#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
try:
	from StringIO import StringIO
except:
	from io import StringIO

GPLNotice = """// glloader
// Copyright (C) 2004-2009 Minmin Gong
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
"""

class Typedef:
	def __init__(self, type_name, synonym):
		assert len(type_name) > 0
		assert len(synonym) > 0

		self.type_name = type_name
		self.synonym = synonym

	def __str__(self):
		return "typedef %s %s;" % (self.type_name, self.synonym)

class Token:
	def __init__(self, name, value):
		assert len(name) > 0
		assert len(value) > 0

		self.name = name
		self.value = value

	def __str__(self):
		return "#define %s %s" % (self.name, self.value)

class Param:
	def __init__(self, type_name, name):
		assert len(type_name) > 0
		assert len(name) > 0

		self.type_name = type_name
		self.name = name

	def __str__(self):
		return "%s %s" % (self.type_name, self.name)

class Mapping:
	def __init__(self, from_ext, name):
		assert len(from_ext) > 0
		assert len(name) > 0

		self.from_ext = from_ext
		self.name = name

class Function:
	def __init__(self, return_type, name, static_link, params, mappings):
		assert len(return_type) > 0
		assert len(name) > 0

		self.return_type = return_type
		self.name = name
		self.static_link = static_link
		self.params = params
		self.mappings = mappings

	def params_str(self):
		ret = ''
		i = 0
		for i, param in enumerate(self.params):
			ret += str(param)
			if i != len(self.params) - 1:
				ret += ', '
		return ret

	def param_names_str(self):
		ret = ''
		i = 0
		for i, param in enumerate(self.params):
			ret += param.name
			if i != len(self.params) - 1:
				ret += ', '
		return ret

class Extension:
	def __init__(self, dom, quite_mode):
		self.name = dom.documentElement.getAttribute("name")
		if dom.documentElement.hasAttribute("predefined"):
			self.predefined = dom.documentElement.getAttribute("predefined")
		else:
			self.predefined = None

		if not quite_mode:
			if dom.documentElement.getAttributeNode("reg_no") == None:
				print("\tWarning: %s is not in the OpenGL Extension Registry." % dom.documentElement.getAttribute("name"))

		self.typedefs = []
		typedefsTag = dom.documentElement.getElementsByTagName("typedefs")
		if (typedefsTag):
			for typedef in typedefsTag[0].getElementsByTagName("typedef"):
				self.typedefs.append(Typedef(typedef.getAttribute("type"),
								typedef.getAttribute("synonym")))

		self.tokens = []
		tokensTag = dom.documentElement.getElementsByTagName("tokens")
		if (tokensTag):
			for token in tokensTag[0].getElementsByTagName("token"):
				self.tokens.append(Token(token.getAttribute("name"),
								token.getAttribute("value")))

		self.functions = []
		funcionsTag = dom.documentElement.getElementsByTagName("functions")
		if (funcionsTag):
			for function in funcionsTag[0].getElementsByTagName("function"):
				params = []
				paramsTag = function.getElementsByTagName("params")
				if (paramsTag):
					for param in paramsTag[0].getElementsByTagName("param"):
						params.append(Param(param.getAttribute("type"),
								param.getAttribute("name")))

				mappings = []
				mappingsTag = function.getElementsByTagName("mappings")
				if (mappingsTag):
					for mapping in mappingsTag[0].getElementsByTagName("mapping"):
						mappings.append(Mapping(mapping.getAttribute("from"),
								mapping.getAttribute("name")))

				static_link = False
				link_attr = function.getAttribute("link")
				if (link_attr != None):
					if "static" == str(link_attr):
						static_link = True
				self.functions.append(Function(function.getAttribute("return"),
							function.getAttribute("name"),
							static_link,
							params, mappings))

		self.additionals = []
		additionalsTag = dom.documentElement.getElementsByTagName("additionals")
		if (additionalsTag):
			for ext_tag in additionalsTag[0].getElementsByTagName("ext"):
				if ext_tag.parentNode == additionalsTag[0]:
					self.additionals.append([ext_tag.getAttribute("name")])
			for one_of_tag in additionalsTag[0].getElementsByTagName("one_of"):
				one_of = []
				for ext in one_of_tag.getElementsByTagName("ext"):
					one_of.append(ext.getAttribute("name"))
				self.additionals.append(one_of)

def create_header(prefix, extensions, base_dir, quite_mode):
	header_str = StringIO()

	header_str.write("/*\n%s*/\n\n" % GPLNotice);

	header_str.write("#ifndef _GLLOADER_%s_H\n" % prefix.upper())
	header_str.write("#define _GLLOADER_%s_H\n\n" % prefix.upper())

	header_str.write("#ifdef __cplusplus\n")
	header_str.write("extern \"C\"\n")
	header_str.write("{\n")
	header_str.write("#endif\n\n")

	typedef_set = set()
	token_set = set()
	function_set = set()

	for extension in extensions:
		header_str.write("#ifndef %s\n" % extension.name)
		header_str.write("#define %s 1\n" % extension.name)
		header_str.write("#endif\n\n")

	for extension in extensions:
		if extension.tokens:
			header_str.write("#ifdef %s\n\n" % extension.name)
			if extension.predefined != None:
				header_str.write("#ifdef %s\n\n" % extension.predefined)

			for token in extension.tokens:
				if (token.name not in token_set):
					header_str.write("%s\n" % token)
					token_set.add(token.name)

			header_str.write("\n")

			if extension.predefined != None:
				header_str.write("#endif\n\n")

			header_str.write("#endif\n\n")

	for extension in extensions:
		if (extension.typedefs):
			header_str.write("#ifdef %s\n\n" % extension.name)
			if extension.predefined != None:
				header_str.write("#ifdef %s\n\n" % extension.predefined)

			for typedef in extension.typedefs:
				if (typedef.synonym not in typedef_set):
					header_str.write("%s\n" % typedef)
					typedef_set.add(typedef.synonym)

			header_str.write("\n")

			if extension.predefined != None:
				header_str.write("#endif\n\n")

			header_str.write("#endif\n\n")

	for extension in extensions:
		if (extension.functions):
			header_str.write("#ifdef %s\n\n" % extension.name)
			if extension.predefined != None:
				header_str.write("#ifdef %s\n\n" % extension.predefined)

			for function in extension.functions:
				if (function.name not in function_set):
					header_str.write("typedef %s (GLLOADER_APIENTRY *%sFUNC)(%s);\n" % (function.return_type, function.name, function.params_str()))

			header_str.write("\n")

			for function in extension.functions:
				if (function.name not in function_set):
					header_str.write("extern GLLOADER_API %sFUNC %s;\n" % (function.name, function.name))
					function_set.add(function.name)

			header_str.write("\n")

			if extension.predefined != None:
				header_str.write("#endif\n\n")

			header_str.write("#endif\n\n")

	for extension in extensions:
		header_str.write("typedef char (GLLOADER_APIENTRY *glloader_%sFUNC)();\n" % extension.name)
	header_str.write("\n")

	for extension in extensions:
		header_str.write("extern GLLOADER_API glloader_%sFUNC glloader_%s;\n" % (extension.name, extension.name))
	header_str.write("\n")

	header_str.write("#ifdef __cplusplus\n")
	header_str.write("}\n")
	header_str.write("#endif\n\n")

	header_str.write("#endif		/* _GLLOADER_%s_H */\n" % prefix.upper())

	try:
		cur_header_file = open("%s/include/glloader/glloader_%s.h" % (base_dir, prefix.lower()), "r")
		cur_header_str = cur_header_file.read()
		cur_header_file.close()
	except:
		cur_header_str = ""
	new_header_str = header_str.getvalue()
	if new_header_str != cur_header_str:
		if not quite_mode:
			print("glloader_%s.h is updated" % prefix.lower())
		header_file = open("%s/include/glloader/glloader_%s.h" % (base_dir, prefix.lower()), "w")
		header_file.write(new_header_str)
		header_file.close()
	else:
		if not quite_mode:
			print("No change detected. Skip glloader_%s.h" % prefix.lower())

def create_source(prefix, extensions, base_dir, quite_mode):
	source_str = StringIO()

	source_str.write("/*\n%s*/\n\n" % GPLNotice);

	source_str.write("#include <glloader/glloader.h>\n")
	source_str.write("#include \"utils.h\"\n\n")

	source_str.write("#ifdef GLLOADER_%s\n\n" % prefix.upper())

	source_str.write("#ifdef __cplusplus\n")
	source_str.write("extern \"C\"\n")
	source_str.write("{\n")
	source_str.write("#endif\n\n")

	function_set = set()

	for extension in extensions:
		if extension.predefined != None:
			source_str.write("#ifdef %s\n" % extension.predefined)

		source_str.write("char _%s = 0;\n" % extension.name)

		if extension.predefined != None:
			source_str.write("#endif\n")
	source_str.write("\n")

	for extension in extensions:
		if extension.predefined != None:
			source_str.write("#ifdef %s\n\n" % extension.predefined)

		source_str.write("static char GLLOADER_APIENTRY _glloader_%s()\n" % extension.name)
		source_str.write("{\n")
		source_str.write("\treturn _%s;\n" % extension.name)
		source_str.write("}\n")
		source_str.write("\n")

		source_str.write("static char GLLOADER_APIENTRY self_init_glloader_%s()\n" % extension.name)
		source_str.write("{\n")
		source_str.write("\tglloader_init();\n")
		source_str.write("\treturn glloader_%s();\n" % extension.name)
		source_str.write("}\n")

		source_str.write("glloader_%sFUNC glloader_%s = self_init_glloader_%s;\n\n" % (extension.name, extension.name, extension.name))

		if (len(extension.functions) != 0):
			source_str.write("#ifdef %s\n" % extension.name)

			source_str.write("\n")

			for function in extension.functions:
				if (function.name not in function_set):
					source_str.write("static %s GLLOADER_APIENTRY self_init_%s(%s)\n" % (function.return_type, function.name, function.params_str()))
					source_str.write("{\n")
					if function.static_link:
						source_str.write("\tLOAD_FUNC1(%s);\n" % function.name)
					else:
						source_str.write("\tglloader_init();\n")
					source_str.write("\t")
					if (function.return_type != "void") and (function.return_type != "VOID"):
						source_str.write("return ")
					source_str.write("%s(%s);\n" % (function.name, function.param_names_str()))
					source_str.write("}\n")

			source_str.write("\n")

			for function in extension.functions:
				if (function.name not in function_set):
					source_str.write("%sFUNC %s = self_init_%s;\n" % (function.name, function.name, function.name))
					function_set.add(function.name)

			source_str.write("\n")

			source_str.write("#endif\n\n")

		if extension.predefined != None:
			source_str.write("#endif\n\n")

	for extension in extensions:
		if extension.predefined != None:
			source_str.write("#ifdef %s\n" % extension.predefined)

		source_str.write("void init_%s()\n" % extension.name)
		source_str.write("{\n")

		source_str.write("\tglloader_%s = _glloader_%s;\n\n" % (extension.name, extension.name))
		
		all_static = True
		for function in extension.functions:
			if not function.static_link:
				all_static = False

		source_str.write("\t_%s = 0;\n" % extension.name)
		source_str.write("\tif (glloader_is_supported(\"%s\"))\n" % extension.name)
		source_str.write("\t{\n")
		source_str.write("\t\t_%s = 1;\n" % extension.name)
		if len(extension.functions) > 0:
			source_str.write("\n")
		for function in extension.functions:
			source_str.write("\t\tLOAD_FUNC1(%s);\n" % function.name)
		for addi in extension.additionals:
			for addi_name in addi:
				for addi_ext in extensions:
					if (addi_ext.name == addi_name):
						for function in addi_ext.functions:
							source_str.write("\t\tLOAD_FUNC1(%s);\n" % function.name)
		source_str.write("\t}\n")

		backup = False
		if extension.additionals:
			backup = True
		else:
			for function in extension.functions:
				if function.mappings:
					backup = True
					break

		if backup:
			plans = []
			for i in range(0, len(extension.functions)):
				froms = []
				for m in extension.functions[i].mappings:
					froms.append(m.from_ext)
					
				found = False
				for plan in plans:
					if plan[0] == froms:
						found = True
						plan[1].append(i)
						break
				if not found:
					plans.append([froms, [i]])

			all_covered = True
			for function in extension.functions:
				if len(function.mappings) == 0:
					all_covered = False
					break

			source_str.write("\telse\n")
			source_str.write("\t{\n")
			for plan in plans:
				for i in range(0, len(plan[0])):
					source_str.write("\t\t")
					if i != 0:
						source_str.write("else ")
					source_str.write("if (glloader_is_supported(\"%s\"))\n" % plan[0][i])
					source_str.write("\t\t{\n")
					for j in range(0, len(plan[1])):
						function = extension.functions[plan[1][j]]
						source_str.write("\t\t\tLOAD_FUNC2(%s, %s);\n" % (function.name, function.mappings[i].name))

					if all_covered and len(plans) == 1 and len(extension.additionals) == 0:
						source_str.write("\n\t\t\t_%s = 1;\n" % extension.name)
						source_str.write("\t\t\tpromote_high(\"%s\");\n" % extension.name)

					source_str.write("\t\t}\n")

			if all_covered and len(plans) != 1:
				all_backup_exts = []
				for plan in plans:
					all_backup_exts.append(plan[0])
				for addi in extension.additionals:
					all_backup_exts.append(addi)

				if len(plans) > 0:
					source_str.write("\n")
				source_str.write("\t\tif (")
				for i in range(0, len(all_backup_exts)):
					plan = all_backup_exts[i]

					if len(plan) > 1:
						source_str.write("(")

					for j in range(0, len(plan)):
						source_str.write("glloader_is_supported(\"%s\")" % plan[j])
						if j != len(plan) - 1:
							source_str.write(" || ")

					if len(plan) > 1:
						source_str.write(")")
					if i != len(all_backup_exts) - 1:
						source_str.write("\n")

					if len(all_backup_exts) > 1 and i != len(all_backup_exts) - 1:
						source_str.write("\t\t\t&& ")

					if i == len(all_backup_exts) - 1:
						source_str.write(")\n")
				source_str.write("\t\t{\n")
				source_str.write("\t\t\t_%s = 1;\n" % extension.name)
				source_str.write("\t\t\tpromote_high(\"%s\");\n" % extension.name)
				source_str.write("\t\t}\n")

			source_str.write("\t}\n")

		source_str.write("}\n")

		if extension.predefined != None:
			source_str.write("#endif\n")

		source_str.write("\n")

	source_str.write("\n");

	source_str.write("void %s_init()\n" % prefix.lower())
	source_str.write("{\n")

	for extension in extensions:
		if extension.predefined != None:
			source_str.write("#ifdef %s\n" % extension.predefined)

		source_str.write("\tinit_%s();\n" % extension.name)

		if extension.predefined != None:
			source_str.write("#endif\n")


	source_str.write("}\n\n")

	source_str.write("#ifdef __cplusplus\n")
	source_str.write("}\n")
	source_str.write("#endif\n\n")

	source_str.write("#endif\t\t/* GLLOADER_%s */\n" % prefix.upper())

	try:
		cur_source_file = open("%s/src/glloader_%s.c" % (base_dir, prefix.lower()), "r")
		cur_source_str = cur_source_file.read()
		cur_source_file.close()
	except:
		cur_source_str = ""
	new_source_str = source_str.getvalue()
	if new_source_str != cur_source_str:
		if not quite_mode:
			print("glloader_%s.c is updated" % prefix.lower())
		source_file = open("%s/src/glloader_%s.c" % (base_dir, prefix.lower()), "w")
		source_file.write(new_source_str)
		source_file.close()
	else:
		if not quite_mode:
			print("No change detected. Skip glloader_%s.c" % prefix.lower())


def auto_gen_glloader_files(base_dir, quite_mode):
	import os
	exts = os.listdir(base_dir + "/xml")

	core_set = {}
	extension_set = {}
	feature_set = {}

	from xml.dom.minidom import parse
	for ext in exts:
		if ext[-4:] == ".xml":
			if not quite_mode:
				print("Processing " + ext)
			prefix = ext[0 : ext.find("_")]
			if (-1 == ext.find("_VERSION_")):
				if prefix not in extension_set:
					extension_set[prefix] = []
				extension_set[prefix].append(Extension(parse(base_dir + "/xml/" + ext), quite_mode))
			else:
				if prefix not in core_set:
					core_set[prefix] = []
				core_set[prefix].append(Extension(parse(base_dir + "/xml/" + ext), quite_mode))

	for cores in core_set.items():
		feature_set[cores[0]] = cores[1]
	for extensions in extension_set.items():
		prefix = extensions[0]
		if prefix not in feature_set:
			feature_set[prefix] = []
		feature_set[prefix].extend(extensions[1])

	if not quite_mode:
		print("")

	if not quite_mode:
		print("Creating header files...")
	for features in feature_set.items():
		create_header(features[0], features[1], base_dir, quite_mode)
	if not quite_mode:
		print("")

	if not quite_mode:
		print("Creating source files...")
	for features in feature_set.items():
		create_source(features[0], features[1], base_dir, quite_mode)
	if not quite_mode:
		print("")


if __name__ == "__main__":
	import os
	import sys

	print("Generating glloader files...")

	quite_mode = False
	if (len(sys.argv) >= 2):
		if ("-q" == sys.argv[1]):
			quite_mode = True

	auto_gen_glloader_files(os.curdir, quite_mode)
