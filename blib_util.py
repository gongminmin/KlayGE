#!/usr/bin/env python
#-*- coding: ascii -*-

import os

def get_compiler_info(compiler, cfg):
	env = os.environ

	if "" == compiler:
		if "VS110COMNTOOLS" in env:
			compiler = "vc11"
		elif "VS100COMNTOOLS" in env:
			compiler = "vc10"
		elif "VS90COMNTOOLS" in env:
			compiler = "vc9"
		elif "VS80COMNTOOLS" in env:
			compiler = "vc8"
		elif os.path.exists("C:\MinGW\bin\gcc.exe"):
			compiler = "mingw"

	if "vc11" == compiler:
		compiler_name = "vc"
		compiler_version = 11
		if "x86" == cfg:
			arch_list = (("x86", "Visual Studio 11"), )
		else:
			arch_list = (("x64", "Visual Studio 11 Win64"), )
	elif "vc10" == compiler:
		compiler_name = "vc"
		compiler_version = 10
		if "x86" == cfg:
			arch_list = (("x86", "Visual Studio 10"), )
		else:
			arch_list = (("x64", "Visual Studio 10 Win64"), )
	elif "vc9" == compiler:
		compiler_name = "vc"
		compiler_version = 9
		if "x86" == cfg:
			arch_list = (("x86", "Visual Studio 9 2008"), )
		else:
			arch_list = (("x64", "Visual Studio 9 2008 Win64"), )
	else:
		return ()

	return (compiler_name, compiler_version, arch_list)

class batch_command:
	def __init__(self):
		self.commands_ = []

	def add_command(self, cmd):
		self.commands_ += [cmd]

	def execute(self):
		import hashlib, datetime

		tmp_gen = hashlib.md5()
		dt = datetime.datetime.now()
		tmp_gen.update( str(dt).encode('utf-8') )
		batch_file = tmp_gen.hexdigest() + ".bat"
		batch_f = open( batch_file, "w" )
		batch_f.writelines( [cmd_line + "\n" for cmd_line in self.commands_] )
		batch_f.close()
		os.system(batch_file)
		os.remove(batch_file)
