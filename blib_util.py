#!/usr/bin/env python
#-*- coding: ascii -*-

import os
try:
	import cfg_build
except:
	cfg_build_f = open("cfg_build.py", "w")
	cfg_build_f.write("""
################################################
# !!!! DO NOT DELETE ANY FIELD OF THIS FILE !!!!
################################################

compiler		= "auto"		# could be "vc11", "vc10", "vc9", "mingw", "gcc", "auto".
toolset			= "auto"		# could be "v110", "v110_xp", "v100", "v90", "auto".
arch			= ("x86", )		# could be "x86", "x64", "arm_app", "x86_app"
config			= ("Debug", "RelWithDebInfo") # could be "Debug", "Release", "MinSizeRel", "RelWithDebInfo"
	""")
	cfg_build_f.close()
	import cfg_build

def get_compiler_info(compiler, archs, cfg):
	import sys

	env = os.environ
	
	platform = sys.platform
	if 0 == platform.find("linux"):
		platform = "linux"

	try:
		cfg_build.compiler
	except:
		cfg_build.compiler = "auto"
	try:
		cfg_build.toolset
	except:
		cfg_build.toolset = "auto"
	try:
		cfg_build.arch
	except:
		cfg_build.arch = ("x86", )
	try:
		cfg_build.config
	except:
		cfg_build.config = ("Debug", "RelWithDebInfo")

	if "" == compiler:
		if ("" == cfg_build.compiler) or ("auto" == cfg_build.compiler):
			if "win32" == platform:
				if "VS110COMNTOOLS" in env:
					compiler = "vc11"
				elif "VS100COMNTOOLS" in env:
					compiler = "vc10"
				elif "VS90COMNTOOLS" in env:
					compiler = "vc9"
				elif os.path.exists("C:\MinGW\bin\gcc.exe"):
					compiler = "mingw"
			elif "linux" == platform:
				compiler = "gcc"
			else:
				print("Unsupported platform\n")
				sys.exit(1)
		else:
			compiler = cfg_build.compiler

	toolset = cfg_build.toolset
	if ("" == cfg_build.toolset) or ("auto" == cfg_build.toolset):
		if "win32" == platform:
			if "vc11" == compiler:
				toolset = "v110"
			elif "vc10" == compiler:
				toolset = "v100"
			elif "vc9" == compiler:
				toolset = "v90"
			
	if "" == archs:
		archs = cfg_build.arch
		if "" == archs:
			archs = ("x86", )
			
	if "" == cfg:
		cfg = cfg_build.config
		if "" == cfg:
			cfg = ("Debug", "RelWithDebInfo")

	arch_list = []
	if "vc11" == compiler:
		compiler_name = "vc"
		compiler_version = 11
		for arch in archs:
			if ("x86" == arch) or ("x86_app" == arch):
				arch_list.append((arch, "Visual Studio 11"))
			elif "arm_app" == arch:
				arch_list.append((arch, "Visual Studio 11 ARM"))
			elif "x64" == arch:
				arch_list.append((arch, "Visual Studio 11 Win64"))
	elif "vc10" == compiler:
		compiler_name = "vc"
		compiler_version = 10
		for arch in archs:
			if "x86" == arch:
				arch_list.append((arch, "Visual Studio 10"))
			elif "x64" == arch:
				arch_list.append((arch, "Visual Studio 10 Win64"))
	elif "vc9" == compiler:
		compiler_name = "vc"
		compiler_version = 9
		for arch in archs:
			if "x86" == arch:
				arch_list.append((arch, "Visual Studio 9 2008"))
			elif "x64" == arch:
				arch_list.append((arch, "Visual Studio 9 2008 Win64"))
	elif "mingw" == compiler:
		compiler_name = "mgw"
		compiler_version = 0
		for arch in archs:
			arch_list.append((arch, "MinGW Makefiles"))
	elif "gcc" == compiler:
		compiler_name = "gcc"
		compiler_version = 0
		for arch in archs:
			arch_list.append((arch, "Unix Makefiles"))
	else:
		return ()
		
	return (compiler_name, compiler_version, arch_list, cfg, platform, toolset)

class batch_command:
	def __init__(self):
		self.commands_ = []

	def add_command(self, cmd):
		self.commands_ += [cmd]

	def execute(self):
		batch_file = "kge_build.bat"
		batch_f = open(batch_file, "w")
		batch_f.writelines([cmd_line + "\n" for cmd_line in self.commands_])
		batch_f.close()
		os.system(batch_file)
		os.remove(batch_file)
