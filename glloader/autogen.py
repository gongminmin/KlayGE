#!/usr/bin/env python
#-*- coding: ascii -*-

GPLNotice = """// glloader
// Copyright (C) 2004-2005 Minmin Gong
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
		self.type_name = type_name
		self.synonym = synonym

	def __str__(self):
		return "typedef %s %s;" % (self.type_name, self.synonym)

class Token:
	def __init__(self, name, value):
		self.name = name
		self.value = value

	def __str__(self):
		return "#define %s %s" % (self.name, self.value)

class Param:
	def __init__(self, type_name, name):
		self.type_name = type_name
		self.name = name

	def __str__(self):
		return "%s %s" % (self.type_name, self.name)

class Mapping:
	def __init__(self, from_ext, name):
		self.from_ext = from_ext
		self.name = name

class Function:
	def __init__(self, return_type, name, params, mappings):
		self.return_type = return_type
		self.name = name
		self.params = params
		self.mappings = mappings

	def params_str(self):
		ret = ''
		i = 0
		for i in range(0, len(self.params)):
			ret += str(self.params[i])
			if i != len(self.params) - 1:
				ret += ', '
		return ret

	def param_names_str(self):
		ret = ''
		i = 0
		for i in range(0, len(self.params)):
			ret += self.params[i].name
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
			print "\tWarning:", dom.documentElement.getAttribute("name"), "is not in the OpenGL Extension Registry."

		self.typedefs = []
		typedefsTag = dom.documentElement.getElementsByTagName("typedefs")
		if (len(typedefsTag) != 0):
			for typedef in typedefsTag[0].getElementsByTagName("typedef"):
				self.typedefs.append(Typedef(typedef.getAttribute("type"),
								typedef.getAttribute("synonym")))

		self.tokens = []
		tokensTag = dom.documentElement.getElementsByTagName("tokens")
		if (len(tokensTag) != 0):
			for token in tokensTag[0].getElementsByTagName("token"):
				self.tokens.append(Token(token.getAttribute("name"),
								token.getAttribute("value")))

		self.functions = []
		funcionsTag = dom.documentElement.getElementsByTagName("functions")
		if (len(funcionsTag) != 0):
			for function in funcionsTag[0].getElementsByTagName("function"):
				params = []
				paramsTag = function.getElementsByTagName("params")
				if (len(paramsTag) != 0):
					for param in paramsTag[0].getElementsByTagName("param"):
						params.append(Param(param.getAttribute("type"),
								param.getAttribute("name")))

				mappings = []
				mappingsTag = function.getElementsByTagName("mappings")
				if (len(mappingsTag) != 0):
					for mapping in mappingsTag[0].getElementsByTagName("mapping"):
						mappings.append(Mapping(mapping.getAttribute("from"),
								mapping.getAttribute("name")))

				self.functions.append(Function(function.getAttribute("return"),
							function.getAttribute("name"),
							params, mappings))

def create_header(prefix, extensions):
	headerFile = open("include/glloader/glloader_%s.h" % prefix.lower(), "w")

	headerFile.write("/*\n" + GPLNotice + "*/\n\n");

	headerFile.write("#ifndef _GLLOADER_%s_H\n" % prefix.upper())
	headerFile.write("#define _GLLOADER_%s_H\n\n" % prefix.upper())

	for extension in extensions:
		headerFile.write("#ifndef %s\n" % extension.name)
		headerFile.write("#define %s 1\n\n" % extension.name)
		if extension.predefined != None:
			headerFile.write("#ifdef %s\n\n" % extension.predefined)

		if (len(extension.tokens) != 0):
			for token in extension.tokens:
				headerFile.write(str(token) + "\n")

			headerFile.write("\n")

		if (len(extension.typedefs) != 0):
			for typedef in extension.typedefs:
				headerFile.write(str(typedef) + "\n")

			headerFile.write("\n")

		if (len(extension.functions) != 0):
			for function in extension.functions:
				headerFile.write("typedef %s (APIENTRY *%sFUNC)(%s);\n" % (function.return_type, function.name, function.params_str()))

			headerFile.write("\n")

		if (len(extension.functions) != 0):
			for function in extension.functions:
				headerFile.write("extern %sFUNC %s;\n" % (function.name, function.name))

			headerFile.write("\n")

		if extension.predefined != None:
			headerFile.write("#endif\n\n")

		headerFile.write("#endif\n\n")

	for extension in extensions:
		headerFile.write("typedef char (APIENTRY *glloader_%sFUNC)();\n" % extension.name)
	headerFile.write("\n")

	for extension in extensions:
		headerFile.write("extern glloader_%sFUNC glloader_%s;\n" % (extension.name, extension.name))
	headerFile.write("\n")

	headerFile.write("#endif		/* _GLLOADER_%s_H */\n" % prefix.upper())

	headerFile.close()

def create_source(prefix, extensions):
	sourceFile = open("src/glloader_%s.cpp" % prefix.lower(), "w")

	sourceFile.write(GPLNotice + "\n");

	sourceFile.write("#include <glloader/glloader.h>\n")
	sourceFile.write("#include \"utils.hpp\"\n\n")

	sourceFile.write("#ifdef GLLOADER_%s\n\n" % prefix.upper())

	sourceFile.write("using glloader::load_funcs;\n")
	sourceFile.write("using glloader::gl_features_extractor;\n\n")

	sourceFile.write("namespace\n")
	sourceFile.write("{\n")
	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n" % extension.predefined)

		sourceFile.write("\tbool _%s = false;\n" % extension.name)

		if extension.predefined != None:
			sourceFile.write("#endif\n")

	sourceFile.write("}\n\n")

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n\n" % extension.predefined)

		sourceFile.write("namespace\n")
		sourceFile.write("{\n")
		sourceFile.write("\tchar APIENTRY _glloader_%s()\n" % extension.name)
		sourceFile.write("\t{\n")
		sourceFile.write("\t\treturn _%s;\n" % extension.name)
		sourceFile.write("\t}\n")
		sourceFile.write("\n")

		sourceFile.write("\tchar APIENTRY self_init_glloader_%s()\n" % extension.name)
		sourceFile.write("\t{\n")
		sourceFile.write("\t\tglloader_init();\n")
		sourceFile.write("\t\treturn glloader_%s();\n" % extension.name)
		sourceFile.write("\t}\n")
		sourceFile.write("}\n\n")

		sourceFile.write("glloader_%sFUNC glloader_%s = self_init_glloader_%s;\n\n" % (extension.name, extension.name, extension.name))

		if (len(extension.functions) != 0):
			sourceFile.write("#ifdef %s\n\n" % extension.name)

			sourceFile.write("namespace\n")
			sourceFile.write("{\n")
			for function in extension.functions:
				sourceFile.write("\t%s APIENTRY self_init_%s(%s)\n" % (function.return_type, function.name, function.params_str()))
				sourceFile.write("\t{\n")
				sourceFile.write("\t\tglloader_init();\n")
				sourceFile.write("\t\treturn %s(%s);\n" % (function.name, function.param_names_str()))
				sourceFile.write("\t}\n")
			sourceFile.write("}\n\n")

			for function in extension.functions:
				sourceFile.write("%sFUNC %s = self_init_%s;\n" % (function.name, function.name, function.name))

			sourceFile.write("\n#endif\n\n")

		if extension.predefined != None:
			sourceFile.write("#endif\n\n")

	sourceFile.write("namespace\n")
	sourceFile.write("{\n");

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n" % extension.predefined)

		sourceFile.write("\tvoid init_%s()\n" % extension.name)
		sourceFile.write("\t{\n")

		sourceFile.write("\t\tglloader_%s = _glloader_%s;\n\n" % (extension.name, extension.name))

		if (len(extension.functions) != 0):
			sourceFile.write("\t\t{\n")

			for function in extension.functions:
				sourceFile.write("\t\t\t%s = NULL;\n" % function.name)

			sourceFile.write("\t\t}\n\n")

		if (len(extension.functions) != 0):
			sourceFile.write("\t\tentries_t entries(%d);\n" % len(extension.functions))
			sourceFile.write("\t\t{\n")

			for i in range(0, len(extension.functions)):
				sourceFile.write("\t\t\tentries[%d] = reinterpret_cast<void**>(&%s);\n" % (i, extension.functions[i].name))

			sourceFile.write("\t\t}\n\n")
			sourceFile.write("\t\tfuncs_names_t names(%d);\n" % len(extension.functions))

		sourceFile.write("\t\tif (glloader_is_supported(\"%s\"))\n" % extension.name)
		sourceFile.write("\t\t{\n")
		sourceFile.write("\t\t\t_%s = true;\n" % extension.name)
		if len(extension.functions) > 0:
			sourceFile.write("\n")
		for i in range(0, len(extension.functions)):
			sourceFile.write("\t\t\tnames[%d] = \"%s\";\n" % (i, extension.functions[i].name))
		sourceFile.write("\t\t}\n")

		backup = False
		for function in extension.functions:
			if len(function.mappings) > 0:
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

			sourceFile.write("\t\telse\n")
			sourceFile.write("\t\t{\n")
			for plan in plans:
				for i in range(0, len(plan[0])):
					sourceFile.write("\t\t\t")
					if i != 0:
						sourceFile.write("else")
					sourceFile.write("if (glloader_is_supported(\"%s\"))\n" % plan[0][i])
					sourceFile.write("\t\t\t{\n")
					for j in range(0, len(plan[1])):
						sourceFile.write("\t\t\t\tnames[%d] = \"%s\";\n" % (plan[1][j], extension.functions[plan[1][j]].mappings[i].name))

					if all_covered and len(plans) == 1:
						sourceFile.write("\n\t\t\t\t_%s = true;\n" % extension.name)
						sourceFile.write("\t\t\t\tgl_features_extractor::instance().promote(\"%s\");\n" % extension.name)

					sourceFile.write("\t\t\t}\n")

					if i != len(plan[0]) - 1:
						sourceFile.write("\n")

			if all_covered and len(plans) > 1:
				sourceFile.write("\n\t\t\tif (")
				for i in range(0, len(plans)):
					plan = plans[i]

					if len(plan[0]) > 1:
						sourceFile.write("(")

					for j in range(0, len(plan[0])):
						sourceFile.write("glloader_is_supported(\"%s\")" % plan[0][j])
						if j != len(plan[0]) - 1:
							sourceFile.write(" || ")

					if len(plan[0]) > 1:
						sourceFile.write(")")
					if i != len(plans) - 1:
						sourceFile.write("\n")

					if len(plans) > 1 and i != len(plans) - 1:
						sourceFile.write("\t\t\t\t&& ")

					if i == len(plans) - 1:
						sourceFile.write(")\n")
				sourceFile.write("\t\t\t{\n")
				sourceFile.write("\t\t\t\t_%s = true;\n" % extension.name)
				sourceFile.write("\t\t\t\tgl_features_extractor::instance().promote(\"%s\");\n" % extension.name)
				sourceFile.write("\t\t\t}\n")

			sourceFile.write("\t\t}\n")

		if (len(extension.functions) != 0):
			sourceFile.write("\n\t\tload_funcs(entries, names);\n")

		sourceFile.write("\t}\n")

		if extension.predefined != None:
			sourceFile.write("#endif\n")

		sourceFile.write("\n")

	sourceFile.write("}\n\n");

	sourceFile.write("void glloader::%s_init()\n" % prefix.lower())
	sourceFile.write("{\n")

	for extension in extensions:
		if extension.predefined != None:
			sourceFile.write("#ifdef %s\n" % extension.predefined)

		sourceFile.write("\tinit_%s();\n" % extension.name)

		if extension.predefined != None:
			sourceFile.write("#endif\n")


	sourceFile.write("}\n\n")

	sourceFile.write("#endif\t\t// GLLOADER_%s\n" % prefix.upper())

	sourceFile.close()


if __name__ == "__main__":
	import os
	exts = os.listdir("xml")

	extension_set = {}

	from xml.dom.minidom import parse
	for ext in exts:
		if ext[-4:] == ".xml":
			print "Processing " + ext
			prefix = ext[0 : ext.find("_")]
			if not extension_set.has_key(prefix):
				extension_set[prefix] = []
			extension_set[prefix].append(Extension(parse("xml/" + ext)))

	print

	print "Creating Header Files..."
	for extensions in extension_set.items():
		create_header(extensions[0], extensions[1])

	print "Creating Source Files..."
	for extensions in extension_set.items():
		create_source(extensions[0], extensions[1])
