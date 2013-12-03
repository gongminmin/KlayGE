#!/usr/bin/env python
#-*- coding: ascii -*-

import os, sys
try:
	import cfg_build
except:
	cfg_build_f = open("cfg_build.py", "w")
	cfg_build_f.write("""
################################################
# !!!! DO NOT DELETE ANY FIELD OF THIS FILE !!!!
################################################

compiler		= "auto"		# could be "vc12", vc11", "vc10", "vc9", "mingw", "gcc", "auto".
toolset			= "auto"		# could be "v120", "v120_xp", "v110", "v110_xp", "v100", "v90", "auto".
arch			= ("x86", )		# could be "x86", "x64", "arm_app", "x86_app", "x64_app"
config			= ("Debug", "RelWithDebInfo") # could be "Debug", "Release", "MinSizeRel", "RelWithDebInfo"
""")
	cfg_build_f.close()
	import cfg_build

class cfg_from_argv:
	def __init__(self, argv, base = 0):
		if len(argv) > base + 1:
			self.compiler = argv[base + 1]
		else:
			self.compiler = ""
		if len(argv) > base + 2:
			self.archs = (argv[base + 2], )
		else:
			self.archs = ""
		if len(argv) > base + 3:
			self.cfg = argv[base + 3]
		else:
			self.cfg = ""

class compiler_info:
	def __init__(self, compiler, archs, cfg):
		env = os.environ
		
		platform = sys.platform
		if 0 == platform.find("win"):
			platform = "win"
		elif 0 == platform.find("linux"):
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
				if "win" == platform:
					if "VS120COMNTOOLS" in env:
						compiler = "vc12"
					elif "VS110COMNTOOLS" in env:
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
					log_error("Unsupported platform\n")
			else:
				compiler = cfg_build.compiler

		toolset = cfg_build.toolset
		if ("" == cfg_build.toolset) or ("auto" == cfg_build.toolset):
			if "win" == platform:
				if "vc12" == compiler:
					toolset = "v120"
				elif "vc11" == compiler:
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
		if "vc12" == compiler:
			compiler_name = "vc"
			compiler_version = 12
			for arch in archs:
				is_winrt = False
				if (arch.find("_app") > 0):
					toolset = "v120"
					is_winrt = True
				is_xp_toolset = False
				if (toolset.find("_xp") > 0):
					is_xp_toolset = True
				if ("x86" == arch) or ("x86_app" == arch):
					arch_list.append((arch, "Visual Studio 12", toolset, is_winrt, is_xp_toolset))
				elif "arm_app" == arch:
					arch_list.append((arch, "Visual Studio 12 ARM", toolset, is_winrt, is_xp_toolset))
				elif ("x64" == arch) or ("x64_app" == arch):
					arch_list.append((arch, "Visual Studio 12 Win64", toolset, is_winrt, is_xp_toolset))
		elif "vc11" == compiler:
			compiler_name = "vc"
			compiler_version = 11
			for arch in archs:
				is_winrt = False
				if (arch.find("_app") > 0):
					toolset = "v110"
					is_winrt = True
				is_xp_toolset = False
				if (toolset.find("_xp") > 0):
					is_xp_toolset = True
				if ("x86" == arch) or ("x86_app" == arch):
					arch_list.append((arch, "Visual Studio 11", toolset, is_winrt, is_xp_toolset))
				elif "arm_app" == arch:
					arch_list.append((arch, "Visual Studio 11 ARM", toolset, is_winrt, is_xp_toolset))
				elif ("x64" == arch) or ("x64_app" == arch):
					arch_list.append((arch, "Visual Studio 11 Win64", toolset, is_winrt, is_xp_toolset))
		elif "vc10" == compiler:
			compiler_name = "vc"
			compiler_version = 10
			for arch in archs:
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 10", toolset, False, False))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 10 Win64", toolset, False, False))
		elif "vc9" == compiler:
			compiler_name = "vc"
			compiler_version = 9
			for arch in archs:
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 9 2008", toolset, False, False))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 9 2008 Win64", toolset, False, False))
		elif "mingw" == compiler:
			compiler_name = "mgw"
			compiler_version = 0
			for arch in archs:
				arch_list.append((arch, "MinGW Makefiles", toolset, False, False))
		elif "gcc" == compiler:
			compiler_name = "gcc"
			compiler_version = 0
			for arch in archs:
				arch_list.append((arch, "Unix Makefiles", toolset, False, False))
		else:
			compiler_name = ""
			compiler_version = 0
			log_error("Wrong configuration\n")

		if "vc" == compiler_name:
			if compiler_version >= 10:
				self.use_msbuild = True
				self.proj_ext_name = "vcxproj"
			else:
				self.use_msbuild = False
				self.proj_ext_name = "vcproj"
		else:
			self.use_msbuild = False
			self.proj_ext_name = ""

		self.name = compiler_name
		self.version = compiler_version
		self.arch_list = arch_list
		self.cfg = cfg
		self.platform = platform

	def msvc_add_build_command(self, batch_cmd, sln_name, proj_name, config, arch = ""):
		if self.use_msbuild:
			batch_cmd.add_command('@SET VisualStudioVersion=%d.0' % self.version)
			if len(proj_name) != 0:
				file_name = "%s.%s" % (proj_name, self.proj_ext_name)
			else:
				file_name = "%s.sln" % sln_name
			config_str = "Configuration=%s" % config
			if len(arch) != 0:
				config_str = "%s,Platform=%s" % (config_str, arch)
			batch_cmd.add_command('@MSBuild %s /nologo /m /v:m /p:%s' % (file_name, config_str))
		else:
			config_str = config
			if len(arch) != 0:
				config_str = "%s|%s" % (config_str, arch)
			proj_str = ""
			if len(proj_name) != 0:
				proj_str = "/project %s" % proj_name
			batch_cmd.add_command('@devenv %s.sln /Build %s %s' % (sln_name, config_str, proj_str))
		batch_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')

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
		ret_code = os.system(batch_file)
		os.remove(batch_file)
		return ret_code

def log_error(message):
	print("[E] %s" % message)
	os.system("pause")
	sys.exit(1)

def log_info(message):
	print("[I] %s" % message)

def log_warning(message):
	print("[W] %s" % message)

def build_a_project(name, build_path, compiler_info, compiler_arch, need_install = False, additional_options = ""):
	curdir = os.path.abspath(os.curdir)

	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	if compiler_arch[3]:
		additional_options += " -D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""
	if compiler_arch[4]:
		additional_options += " -D KLAYGE_WITH_XP_TOOLSET:BOOL=\"TRUE\""
	if compiler_info.name != "vc":
		additional_options += " -D KLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]

	if "vc" == compiler_info.name:
		if ("x86" == compiler_arch[0]) or ("x86_app" == compiler_arch[0]):
			vc_option = "x86"
		elif ("x64" == compiler_arch[0]) or ("x64_app" == compiler_arch[0]):
			vc_option = "x86_amd64"
		elif ("arm_app" == compiler_arch[0]):
			vc_option = "x86_arm"

		build_dir = "%s/build/%s-%d_0-%s" % (build_path, compiler_info.name, compiler_info.version, compiler_arch[0])
		if not os.path.exists(build_dir):
			os.makedirs(build_dir)

		os.chdir(build_dir)

		cmake_cmd = batch_command()
		cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
		if cmake_cmd.execute() != 0:
			log_error("Config %s failed." % name)

		build_cmd = batch_command()
		build_cmd.add_command('@CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, vc_option))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, name, "ALL_BUILD", config)
			if need_install:
				compiler_info.msvc_add_build_command(build_cmd, name, "INSTALL", config)
		if build_cmd.execute() != 0:
			log_error("Build %s failed." % name)

		os.chdir(curdir)
	else:
		for config in compiler_info.cfg:
			build_dir = "%s/build/%s-%s-%s" % (build_path, compiler_info.name, compiler_arch[0], config)
			if not os.path.exists(build_dir):
				os.makedirs(build_dir)

			os.chdir(build_dir)
			
			additional_options += " -D CMAKE_BUILD_TYPE:STRING=\"%s\"" % config
			
			cmake_cmd = batch_command()
			cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
			if cmake_cmd.execute() != 0:
				log_error("Config %s failed." % name)		

			install_str = ""
			if need_install:
				install_str = "install"
			build_cmd = batch_command()
			if "mgw" == compiler_info.name:
				build_cmd.add_command("@mingw32-make.exe %s" % install_str)
			else:
				build_cmd.add_command("@make %s" % install_str)
			build_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
			if build_cmd.execute() != 0:
				log_error("Build %s failed." % name)

			os.chdir(curdir)

def copy_to_dst(src_name, dst_dir):
	print("Copy %s to %s" % (src_name, dst_dir))
	import shutil
	shutil.copy2(src_name, dst_dir)
