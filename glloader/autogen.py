#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function

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
	def __init__(self, dom):
		self.name = dom.documentElement.getAttribute("name")
		if dom.documentElement.hasAttribute("predefined"):
			self.predefined = dom.documentElement.getAttribute("predefined")
		else:
			self.predefined = None

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

def create_header(prefix, extensions):
	headerFile = open("include/glloader/glloader_%s.h" % prefix.lower(), "w")

	headerFile.write("/*\n%s*/\n\n" % GPLNotice);

	headerFile.write("#ifndef _GLLOADER_%s_H\n" % prefix.upper())
	headerFile.write("#define _GLLOADER_%s_H\n\n" % prefix.upper())

	for extension in extensions:
		headerFile.write("#ifndef %s\n" % extension.name)
		headerFile.write("#define %s 1\n" % extension.name)
		headerFile.write("#endif\n\n")

	for extension in extensions:
		if extension.tokens:
			headerFile.write("#ifdef %s\n\n" % extension.name)
			if extension.predefined != None:
				headerFile.write("#ifdef %s\n\n" % extension.predefined)

			for token in extension.tokens:
				headerFile.write("%s\n" % token)

			headerFile.write("\n")

			if extension.predefined != None:
				headerFile.write("#endif\n\n")

			headerFile.write("#endif\n\n")

	for extension in extensions:
		if (extension.typedefs):
			headerFile.write("#ifdef %s\n\n" % extension.name)
			if extension.predefined != None:
				headerFile.write("#ifdef %s\n\n" % extension.predefined)

			for typedef in extension.typedefs:
				headerFile.write("%s\n" % typedef)

			headerFile.write("\n")

			if extension.predefined != None:
				headerFile.write("#endif\n\n")

			headerFile.write("#endif\n\n")

	for extension in extensions:
		if (extension.functions):
			headerFile.write("#ifdef %s\n\n" % extension.name)
			if extension.predefined != None:
				headerFile.write("#ifdef %s\n\n" % extension.predefined)

			all_static = True
			for function in extension.functions:
				if not function.static_link:
					headerFile.write("typedef %s (APIENTRY *%sFUNC)(%s);\n" % (function.return_type, function.name, function.params_str()))
					all_static = False

			if not all_static:
				headerFile.write("\n")

			for function in extension.functions:
				if function.static_link:
					headerFile.write("EXTERN_C %s APIENTRY %s(%s);\n" % (function.return_type, function.name, function.params_str()))
				else:
					headerFile.write("EXTERN_C GLLOADER_API %sFUNC %s;\n" % (function.name, function.name))

			headerFile.write("\n")

			if extension.predefined != None:
				headerFile.write("#endif\n\n")

			headerFile.write("#endif\n\n")

	for extension in extensions:
		headerFile.write("typedef char (APIENTRY *glloader_%sFUNC)();\n" % extension.name)
	headerFile.write("\n")

	for extension in extensions:
		headerFile.write("EXTERN_C GLLOADER_API glloader_%sFUNC glloader_%s;\n" % (extension.name, extension.name))
	headerFile.write("\n")

	headerFile.write("#endif		/* _GLLOADER_%s_H */\n" % prefix.upper())

	headerFile.close()

def create_source(prefix, extensions):
	sourceFile = open("src/glloader_%s.c" % prefix.lower(), "w")

	sourceFile.write("/*\n%s*/\n\n" % GPLNotice);

	sourceFile.write("#include <glloader/glloader.h>\n")
	sourceFile.write("#include \"utils.h\"\n\n")

	sourceFile.write("#ifdef GLLOADER_%s\n\n" % prefix.upper())

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n" % extension.predefined)

		sourceFile.write("char _%s = 0;\n" % extension.name)

		if extension.predefined != None:
			sourceFile.write("#endif\n")
	sourceFile.write("\n")

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n\n" % extension.predefined)

		all_static = True
		for function in extension.functions:
			if not function.static_link:
				all_static = False

		sourceFile.write("static char APIENTRY _glloader_%s()\n" % extension.name)
		sourceFile.write("{\n")
		sourceFile.write("\treturn _%s;\n" % extension.name)
		sourceFile.write("}\n")
		sourceFile.write("\n")

		sourceFile.write("static char APIENTRY self_init_glloader_%s()\n" % extension.name)
		sourceFile.write("{\n")
		sourceFile.write("\tglloader_init();\n")
		sourceFile.write("\treturn glloader_%s();\n" % extension.name)
		sourceFile.write("}\n")

		sourceFile.write("glloader_%sFUNC glloader_%s = self_init_glloader_%s;\n\n" % (extension.name, extension.name, extension.name))

		if (len(extension.functions) != 0):
			sourceFile.write("#ifdef %s\n" % extension.name)

			if not all_static:
				sourceFile.write("\n")

			for function in extension.functions:
				if not function.static_link:
					sourceFile.write("static %s APIENTRY self_init_%s(%s)\n" % (function.return_type, function.name, function.params_str()))
					sourceFile.write("{\n")
					sourceFile.write("\tglloader_init();\n")
					sourceFile.write("\t")
					if (function.return_type != "void") and (function.return_type != "VOID"):
						sourceFile.write("return ")
					sourceFile.write("%s(%s);\n" % (function.name, function.param_names_str()))
					sourceFile.write("}\n")
			if not all_static:
				sourceFile.write("\n")

			for function in extension.functions:
				if not function.static_link:
					sourceFile.write("%sFUNC %s = self_init_%s;\n" % (function.name, function.name, function.name))

			if not all_static:
				sourceFile.write("\n")

			sourceFile.write("#endif\n\n")

		if extension.predefined != None:
			sourceFile.write("#endif\n\n")

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n" % extension.predefined)

		all_static = True
		for function in extension.functions:
			if not function.static_link:
				all_static = False

		sourceFile.write("void init_%s()\n" % extension.name)
		sourceFile.write("{\n")

		sourceFile.write("\tglloader_%s = _glloader_%s;\n\n" % (extension.name, extension.name))

		if not all_static:
			if (len(extension.functions) != 0):
				sourceFile.write("\t{\n")

				for function in extension.functions:
					if not function.static_link:
						sourceFile.write("\t\t%s = NULL;\n" % function.name)

				sourceFile.write("\t}\n\n")

		sourceFile.write("\tif (glloader_is_supported(\"%s\"))\n" % extension.name)
		sourceFile.write("\t{\n")
		sourceFile.write("\t\t_%s = 1;\n" % extension.name)
		if not all_static:
			if len(extension.functions) > 0:
				sourceFile.write("\n")
			for function in extension.functions:
				if not function.static_link:
					sourceFile.write("\t\tLOAD_FUNC1(%s);\n" % function.name)
		sourceFile.write("\t}\n")

		backup = False
		if extension.additionals:
			backup = True
		else:
			for function in extension.functions:
				if not function.static_link:
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

			sourceFile.write("\telse\n")
			sourceFile.write("\t{\n")
			for plan in plans:
				for i in range(0, len(plan[0])):
					sourceFile.write("\t\t")
					if i != 0:
						sourceFile.write("else ")
					sourceFile.write("if (glloader_is_supported(\"%s\"))\n" % plan[0][i])
					sourceFile.write("\t\t{\n")
					for j in range(0, len(plan[1])):
						function = extension.functions[plan[1][j]]
						sourceFile.write("\t\t\tLOAD_FUNC2(%s, %s);\n" % (function.name, function.mappings[i].name))

					if all_covered and len(plans) == 1 and len(extension.additionals) == 0:
						sourceFile.write("\n\t\t\t_%s = 1;\n" % extension.name)
						sourceFile.write("\t\t\tpromote_high(\"%s\");\n" % extension.name)

					sourceFile.write("\t\t}\n")

			if all_covered and len(plans) != 1:
				all_backup_exts = []
				for plan in plans:
					all_backup_exts.append(plan[0])
				for addi in extension.additionals:
					all_backup_exts.append(addi)

				if len(plans) > 0:
					sourceFile.write("\n")
				sourceFile.write("\t\tif (")
				for i in range(0, len(all_backup_exts)):
					plan = all_backup_exts[i]

					if len(plan) > 1:
						sourceFile.write("(")

					for j in range(0, len(plan)):
						sourceFile.write("glloader_is_supported(\"%s\")" % plan[j])
						if j != len(plan) - 1:
							sourceFile.write(" || ")

					if len(plan) > 1:
						sourceFile.write(")")
					if i != len(all_backup_exts) - 1:
						sourceFile.write("\n")

					if len(all_backup_exts) > 1 and i != len(all_backup_exts) - 1:
						sourceFile.write("\t\t\t&& ")

					if i == len(all_backup_exts) - 1:
						sourceFile.write(")\n")
				sourceFile.write("\t\t{\n")
				sourceFile.write("\t\t\t_%s = 1;\n" % extension.name)
				sourceFile.write("\t\t\tpromote_high(\"%s\");\n" % extension.name)
				sourceFile.write("\t\t}\n")

			sourceFile.write("\t}\n")

		sourceFile.write("}\n")

		if extension.predefined != None:
			sourceFile.write("#endif\n")

		sourceFile.write("\n")

	sourceFile.write("\n");

	sourceFile.write("void %s_init()\n" % prefix.lower())
	sourceFile.write("{\n")

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n" % extension.predefined)

		sourceFile.write("\tinit_%s();\n" % extension.name)

		if extension.predefined != None:
			sourceFile.write("#endif\n")


	sourceFile.write("}\n\n")

	sourceFile.write("#endif\t\t/* GLLOADER_%s */\n" % prefix.upper())

	sourceFile.close()


if __name__ == "__main__":
	import os
	exts = os.listdir("xml")

	extension_set = {}

	from xml.dom.minidom import parse
	for ext in exts:
		if ext[-4:] == ".xml":
			print("Processing " + ext)
			prefix = ext[0 : ext.find("_")]
			if prefix not in extension_set:
				extension_set[prefix] = []
			extension_set[prefix].append(Extension(parse("xml/" + ext)))

	print("")

	print("Creating Header Files...")
	for extensions in extension_set.items():
		create_header(extensions[0], extensions[1])

	print("Creating Source Files...")
	for extensions in extension_set.items():
		create_source(extensions[0], extensions[1])
