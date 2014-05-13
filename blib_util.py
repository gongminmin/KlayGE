#!/usr/bin/env python
#-*- coding: ascii -*-

import os, sys, multiprocessing, subprocess
try:
	import cfg_build
except:
	cfg_build_f = open("cfg_build.py", "w")
	cfg_build_f.write("""################################################
# !!!! DO NOT DELETE ANY FIELD OF THIS FILE !!!!
################################################

# Compiler name.
#   On Windows, could be "vc120", "vc110", "vc100", "vc90", "mingw", "auto".
#   On WinRT, could be "vc120", "vc110", "auto".
#   On Android, could be "gcc", "auto".
#   On Linux, could be "gcc", "auto".
compiler		= "auto"

# Toolset name.
#   On Windows, could be "v120", "v120_xp", "v110", "v110_xp", "v100", "auto".
#   On WinRT, could be "v120", "v110", "auto".
#   On Android, could be "4.4.3", "4.6", "4.8", "auto".
#   On Linux, could be "auto".
toolset			= "auto"

# Target CPU architecture.
#   On Windows, could be "x86", "x64".
#   On WinRT, could be  "arm_app", "x86_app", "x64_app".
#   On Android, cound be "armeabi", "armeabi-v7a", "x86".
#   On Linux, could be "x86", "x64".
arch			= ("x86", )

# Configuration. Could be "Debug", "Release", "MinSizeRel", "RelWithDebInfo".
config			= ("Debug", "RelWithDebInfo")

# Target platform for cross compiling. Could be "android", "auto".
target			= "auto"
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

class build_info:
	def __init__(self, compiler, archs, cfg):
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
		try:
			cfg_build.target
		except:
			cfg_build.target = "auto"
			
		env = os.environ
		
		host_platform = sys.platform
		if 0 == host_platform.find("win"):
			host_platform = "win"
		elif 0 == host_platform.find("linux"):
			host_platform = "linux"
		if "auto" == cfg_build.target:
			target_platform = host_platform
		else:
			target_platform = cfg_build.target
		if "android" == target_platform:
			prefer_static = True
		else:
			prefer_static = False

		self.host_platform = host_platform
		self.target_platform = target_platform
		self.prefer_static = prefer_static

		if "" == compiler:
			if ("" == cfg_build.compiler) or ("auto" == cfg_build.compiler):
				if "win" == target_platform:
					if "VS120COMNTOOLS" in env:
						compiler = "vc120"
					elif "VS110COMNTOOLS" in env:
						compiler = "vc110"
					elif "VS100COMNTOOLS" in env:
						compiler = "vc100"
					elif "VS90COMNTOOLS" in env:
						compiler = "vc90"
					elif 0 == os.system("where clang++"):
						compiler = "clang"
					elif os.path.exists("C:/MinGW/bin/g++.exe") or (0 == os.system("where g++")):
						compiler = "mingw"
				elif "linux" == target_platform:
					compiler = "gcc"
				else:
					log_error("Unsupported host platform\n")
			else:
				compiler = cfg_build.compiler

				if compiler in ("vc12", "vc11", "vc10", "vc9"):
					compiler += '0'
					log_warning("Deprecated compiler name, please use " + compiler + " instead.\n")

		toolset = cfg_build.toolset
		if ("" == cfg_build.toolset) or ("auto" == cfg_build.toolset):
			if "win" == target_platform:
				if "vc120" == compiler:
					toolset = "v120"
				elif "vc110" == compiler:
					toolset = "v110"
				elif "vc100" == compiler:
					toolset = "v100"
				elif "vc90" == compiler:
					toolset = "v90"
			elif "android" == target_platform:
				toolset = "4.6"
				
		if "" == archs:
			archs = cfg_build.arch
			if "" == archs:
				archs = ("x86", )
				
		if "" == cfg:
			cfg = cfg_build.config
			if "" == cfg:
				cfg = ("Debug", "RelWithDebInfo")

		arch_list = []
		if "vc120" == compiler:
			compiler_name = "vc"
			compiler_version = 120
			for arch in archs:
				is_winrt = False
				if (arch.find("_app") > 0):
					toolset = "v120"
					is_winrt = True
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 12", toolset, is_winrt))
				elif "x86_app" == arch:
					arch_list.append((arch, "Visual Studio 12 WinRT-x86", toolset, is_winrt))
				elif "arm_app" == arch:
					arch_list.append((arch, "Visual Studio 12 WinRT-ARM", toolset, is_winrt))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 12 Win64", toolset, is_winrt))
				elif "x64_app" == arch:
					arch_list.append((arch, "Visual Studio 12 WinRT-x64", toolset, is_winrt))
		elif "vc110" == compiler:
			compiler_name = "vc"
			compiler_version = 110
			for arch in archs:
				is_winrt = False
				if (arch.find("_app") > 0):
					toolset = "v110"
					is_winrt = True
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 11", toolset, is_winrt))
				elif "x86_app" == arch:
					arch_list.append((arch, "Visual Studio 11 WinRT", toolset, is_winrt))
				elif "arm_app" == arch:
					arch_list.append((arch, "Visual Studio 11 ARM WinRT", toolset, is_winrt))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 11 Win64", toolset, is_winrt))
				elif "x64_app" == arch:
					arch_list.append((arch, "Visual Studio 11 Win64 WinRT", toolset, is_winrt))
		elif "vc100" == compiler:
			compiler_name = "vc"
			compiler_version = 100
			for arch in archs:
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 10", toolset, False))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 10 Win64", toolset, False))
		elif "vc90" == compiler:
			compiler_name = "vc"
			compiler_version = 90
			for arch in archs:
				if "x86" == arch:
					arch_list.append((arch, "Visual Studio 9 2008", toolset, False))
				elif "x64" == arch:
					arch_list.append((arch, "Visual Studio 9 2008 Win64", toolset, False))
		elif "clang" == compiler:
			compiler_name = "clang"
			compiler_version = self.retrive_clang_version()
			if ("win" == host_platform):
				for arch in archs:
					arch_list.append((arch, "MinGW Makefiles", toolset, False))
			else:
				for arch in archs:
					arch_list.append((arch, "Unix Makefiles", toolset, False))			
		elif "mingw" == compiler:
			compiler_name = "mgw"
			compiler_version = self.retrive_gcc_version()
			for arch in archs:
				arch_list.append((arch, "MinGW Makefiles", toolset, False))
		elif "gcc" == compiler:
			compiler_name = "gcc"
			self.toolset = toolset
			compiler_version = self.retrive_gcc_version()
			if ("android" == target_platform) and ("win" == host_platform):
				for arch in archs:
					arch_list.append((arch, "MinGW Makefiles", toolset, False))
			else:
				for arch in archs:
					arch_list.append((arch, "Unix Makefiles", toolset, False))
		else:
			compiler_name = ""
			compiler_version = 0
			log_error("Wrong configuration\n")

		if "vc" == compiler_name:
			if compiler_version >= 100:
				self.use_msbuild = True
				self.proj_ext_name = "vcxproj"
			else:
				self.use_msbuild = False
				self.proj_ext_name = "vcproj"
		else:
			self.use_msbuild = False
			self.proj_ext_name = ""

		self.compiler_name = compiler_name
		self.compiler_version = compiler_version
		self.arch_list = arch_list
		self.cfg = cfg

	def msvc_add_build_command(self, batch_cmd, sln_name, proj_name, config, arch = ""):
		if self.use_msbuild:
			batch_cmd.add_command('@SET VisualStudioVersion=%d.0' % (self.compiler_version / 10))
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
			proj_str = ""
			if len(proj_name) != 0:
				proj_str = "/project %s" % proj_name
			batch_cmd.add_command('@devenv %s.sln /Build %s %s' % (sln_name, config_str, proj_str))
		batch_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		
	def retrive_gcc_version(self):
		if ("android" == self.target_platform):
			gcc_ver = self.toolset
		else:
			gcc_ver = subprocess.check_output(["gcc", "-dumpversion"])
		gcc_ver_components = gcc_ver.split(".")
		return int(gcc_ver_components[0] + gcc_ver_components[1])

	def retrive_clang_version(self):
		clang_ver = subprocess.check_output(["clang", "--version"])
		clang_ver_components = clang_ver.split()[2].split(".")
		return int(clang_ver_components[0] + clang_ver_components[1])

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

def build_a_project(name, build_path, build_info, compiler_arch, need_install = False, additional_options = ""):
	curdir = os.path.abspath(os.curdir)

	toolset_name = ""
	if ("vc" == build_info.compiler_name) and (build_info.compiler_version >= 100):
		toolset_name = "-T %s" % compiler_arch[2]

	if compiler_arch[3]:
		additional_options += " -DKLAYGE_BUILD_PLATFORM_WINRT:BOOL=\"TRUE\""
	if build_info.compiler_name != "vc":
		additional_options += " -DKLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]
	if "android" == build_info.target_platform:
		additional_options += " -DCMAKE_TOOLCHAIN_FILE=\"%s/cmake/android.toolchain.cmake\"" % curdir
		additional_options += " -DANDROID_NATIVE_API_LEVEL=9"
		if "win" == build_info.host_platform:
			additional_options += " -DCMAKE_MAKE_PROGRAM=\"%ANDROID_NDK%\\prebuilt\\windows\\bin\\make.exe\""

	if "vc" == build_info.compiler_name:
		if ("x86" == compiler_arch[0]) or ("x86_app" == compiler_arch[0]):
			vc_option = "x86"
			vc_arch = "Win32"
		elif ("x64" == compiler_arch[0]) or ("x64_app" == compiler_arch[0]):
			vc_option = "x86_amd64"
			vc_arch = "x64"
		elif ("arm_app" == compiler_arch[0]):
			vc_option = "x86_arm"
			vc_arch = "ARM"

		build_dir = "%s/build/%s%d_%s_%s" % (build_path, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_arch[0])
		if not os.path.exists(build_dir):
			os.makedirs(build_dir)

		os.chdir(build_dir)

		cmake_cmd = batch_command()
		cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
		if cmake_cmd.execute() != 0:
			log_error("Config %s failed." % name)

		build_cmd = batch_command()
		build_cmd.add_command('@CALL "%%VS%dCOMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (build_info.compiler_version, vc_option))
		for config in build_info.cfg:
			build_info.msvc_add_build_command(build_cmd, name, "ALL_BUILD", config, vc_arch)
			if need_install:
				build_info.msvc_add_build_command(build_cmd, name, "INSTALL", config, vc_arch)
		if build_cmd.execute() != 0:
			log_error("Build %s failed." % name)

		os.chdir(curdir)
	else:
		if "win" == build_info.host_platform:
			if "android" == build_info.target_platform:
				make_name = "%ANDROID_NDK%\\prebuilt\\windows\\bin\\make.exe"
			else:
				make_name = "mingw32-make.exe"
		else:
			make_name = "make"
		make_name += " -j%d" % multiprocessing.cpu_count()

		for config in build_info.cfg:
			build_dir = "%s/build/%s%d_%s_%s-%s" % (build_path, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_arch[0], config)
			if not os.path.exists(build_dir):
				os.makedirs(build_dir)
				if ("clang" == build_info.compiler_name) and (build_info.target_platform != "android"):
					additional_options += " -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"

			os.chdir(build_dir)
			
			config_options = "-DCMAKE_BUILD_TYPE:STRING=\"%s\"" % config
			if "android" == build_info.target_platform:
				config_options += " -DANDROID_ABI=%s" % compiler_arch[0]
				if "x86" == compiler_arch[0]:
					config_options += " -DANDROID_TOOLCHAIN_NAME=x86-%s" % compiler_arch[2]
				else:
					config_options += " -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-%s" % compiler_arch[2]
			
			cmake_cmd = batch_command()
			cmake_cmd.add_command('cmake -G "%s" %s %s %s %s' % (compiler_arch[1], toolset_name, additional_options, config_options, "../cmake"))
			if cmake_cmd.execute() != 0:
				log_error("Config %s failed." % name)		

			install_str = ""
			if need_install and (not build_info.prefer_static):
				install_str = "install"
			build_cmd = batch_command()
			build_cmd.add_command("@%s %s" % (make_name, install_str))
			build_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
			if build_cmd.execute() != 0:
				log_error("Build %s failed." % name)

			os.chdir(curdir)

def copy_to_dst(src_name, dst_dir):
	print("Copy %s to %s" % (src_name, dst_dir))
	import shutil
	shutil.copy2(src_name, dst_dir)
