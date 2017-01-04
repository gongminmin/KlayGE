#!/usr/bin/env python
#-*- coding: ascii -*-

import os, sys, multiprocessing, subprocess, shutil
try:
	import cfg_build
except:
	print("Generating cfg_build.py ...")
	shutil.copyfile("cfg_build_default.py", "cfg_build.py")
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
			self.cfg = (argv[base + 3], )
		else:
			self.cfg = ""

class compiler_info:
	def __init__(self, arch, gen_name, toolset, compiler_root, vcvarsall_path = ""):
		self.arch = arch
		self.generator = gen_name
		self.toolset = toolset
		self.compiler_root = compiler_root
		self.vcvarsall_path = vcvarsall_path

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
			cfg_build.arch = ("x64", )
		try:
			cfg_build.config
		except:
			cfg_build.config = ("Debug", "RelWithDebInfo")
		try:
			cfg_build.target
		except:
			cfg_build.target = "auto"
		try:
			cfg_build.shader_platform_name
		except:
			cfg_build.shader_platform_name = "auto"
			
		env = os.environ

		host_platform = sys.platform
		if 0 == host_platform.find("win"):
			host_platform = "win"
		elif 0 == host_platform.find("linux"):
			host_platform = "linux"
		elif 0 == host_platform.find("darwin"):
			host_platform = "darwin"
		if "auto" == cfg_build.target:
			target_platform = host_platform
		else:
			target_platform = cfg_build.target
			if 0 == target_platform.find("android"):
				if not "ANDROID_NDK" in env:
					log_error("You must define a ANDROID_NDK environ variable to your location of NDK.\n")

				space_place = target_platform.find(' ')
				if space_place != -1:
					android_ver = target_platform[space_place + 1:]
					target_platform = target_platform[0:space_place]
					if "5.1" == android_ver:
						target_api_level = 22
					elif "5.0" == android_ver:
						target_api_level = 21
					elif "4.4w" == android_ver:
						target_api_level = 20
					elif "4.4" == android_ver:
						target_api_level = 19
					elif "4.3" == android_ver:
						target_api_level = 18
					elif "4.2" == android_ver:
						target_api_level = 17
					elif "4.1" == android_ver:
						target_api_level = 16
					elif ("4.0.3" == android_ver) or ("4.0.4" == android_ver):
						target_api_level = 15
					elif "4.0" == android_ver:
						target_api_level = 14
					elif "3.2" == android_ver:
						target_api_level = 13
					elif "3.1" == android_ver:
						target_api_level = 12
					elif "3.0" == android_ver:
						target_api_level = 11
					elif ("2.3.4" == android_ver) or ("2.3.3" == android_ver):
						target_api_level = 10
					elif "2.3" == android_ver:
						target_api_level = 9
					elif "2.2" == android_ver:
						target_api_level = 8
					elif "2.1" == android_ver:
						target_api_level = 7
					elif "2.0.1" == android_ver:
						target_api_level = 6
					elif "2.0" == android_ver:
						target_api_level = 5
					elif "1.6" == android_ver:
						target_api_level = 4
					elif "1.5" == android_ver:
						target_api_level = 3
					elif "1.1" == android_ver:
						target_api_level = 2
					elif "1.0" == android_ver:
						target_api_level = 1
					else:
						log_error("Unsupported android version\n")
				else:
					target_api_level = 10
				self.target_api_level = target_api_level
			elif (0 == target_platform.find("win_store")) or (0 == target_platform.find("win_phone")):
				space_place = target_platform.find(' ')
				if space_place != -1:
					target_api_level = target_platform[space_place + 1:]
					target_platform = target_platform[0:space_place]
				else:
					target_api_level = "8.0"
				self.target_api_level = target_api_level
		if ("android" == target_platform) or ("ios" == target_platform):
			prefer_static = True
		else:
			prefer_static = False

		shader_platform_name = cfg_build.shader_platform_name
		if 0 == len(shader_platform_name):
			shader_platform_name = "auto"

		self.host_platform = host_platform
		self.target_platform = target_platform
		self.shader_platform_name = shader_platform_name
		self.prefer_static = prefer_static
		self.is_clean = ("clean" == compiler)

		self.is_windows_desktop = False
		self.is_windows_store = False
		self.is_windows_phone = False
		self.is_windows_runtime = False
		self.is_windows = False
		self.is_android = False
		self.is_linux = False
		self.is_darwin = False
		self.is_ios = False

		if "win" == target_platform:
			self.is_windows = True
			self.is_windows_desktop = True
		elif "win_store" == target_platform:
			self.is_windows = True
			self.is_windows_store = True
			self.is_windows_runtime = True
		elif "win_phone" == target_platform:
			self.is_windows = True
			self.is_windows_phone = True
			self.is_windows_runtime = True
		elif "android" == target_platform:
			self.is_android = True
		elif "linux" == target_platform:
			self.is_linux = True
		elif "darwin" == target_platform:
			self.is_darwin = True
		elif "ios" == target_platform:
			self.is_ios = True

		self.is_dev_platform = (self.is_windows_desktop or self.is_linux or self.is_darwin)

		if ("" == compiler) or self.is_clean:
			if ("" == cfg_build.compiler) or ("auto" == cfg_build.compiler):
				if 0 == target_platform.find("win"):
					if "ProgramFiles" in env:
						program_files_folder = env["ProgramFiles"]
					else:
						program_files_folder = "C:\Program Files"

					if ("VS140COMNTOOLS" in env) or os.path.exists(program_files_folder + "\\Microsoft Visual Studio 14.0\\VC\\VCVARSALL.BAT"):
						compiler = "vc140"
					elif ("VS120COMNTOOLS" in env) or os.path.exists(program_files_folder + "\\Microsoft Visual Studio 12.0\\VC\\VCVARSALL.BAT"):
						compiler = "vc120"
					elif 0 == os.system("where clang++"):
						compiler = "clang"
					elif os.path.exists("C:/MinGW/bin/g++.exe") or (0 == os.system("where g++")):
						compiler = "mingw"
				elif ("linux" == target_platform) or ("android" == target_platform):
					compiler = "gcc"
				elif ("darwin" == target_platform) or ("ios" == target_platform):
					compiler = "clang"
				else:
					log_error("Unsupported target platform.\n")
			else:
				compiler = cfg_build.compiler

				if compiler in ("vc14", "vc12"):
					compiler += '0'
					log_warning("Deprecated compiler name, please use " + compiler + " instead.\n")

		if 0 == target_platform.find("win"):
			if "ProgramFiles" in env:
				program_files_folder = env["ProgramFiles"]
			else:
				program_files_folder = "C:\Program Files"

			if "vc140" == compiler:
				if "VS140COMNTOOLS" in env:
					compiler_root = env["VS140COMNTOOLS"] + "..\\..\\VC\\bin\\"
					vcvarsall_path = "..\\VCVARSALL.BAT"
				else:
					try_folder = program_files_folder + "\\Microsoft Visual Studio 14.0\\VC\\bin\\"
					try_vcvarsall = "..\\VCVARSALL.BAT"
					if os.path.exists(try_folder + try_vcvarsall):
						compiler_root = try_folder
						vcvarsall_path = try_vcvarsall
					else:
						log_error("Can't find the compiler.\n")
			elif "vc120" == compiler:
				if "VS120COMNTOOLS" in env:
					compiler_root = env["VS120COMNTOOLS"] + "..\\..\\VC\\bin\\"
					vcvarsall_path = "..\\VCVARSALL.BAT"
				else:
					try_folder = program_files_folder + "\\Microsoft Visual Studio 12.0\\VC\\bin\\"
					try_vcvarsall = "..\\VCVARSALL.BAT"
					if os.path.exists(try_folder + try_vcvarsall):
						compiler_root = try_folder
						vcvarsall_path = try_vcvarsall
					else:
						log_error("Can't find the compiler.\n")
			elif "clang" == compiler:
				clang_loc = subprocess.check_output("where clang++").decode()
				clang_loc = clang_loc.split("\r\n")[0]
				compiler_root = clang_loc[0:clang_loc.rfind("\\clang++") + 1]
			elif "mingw" == compiler:
				if os.path.exists("C:/MinGW/bin/g++.exe"):
					compiler_root = "C:/MinGW/bin/"
				else:
					gcc_loc = subprocess.check_output("where g++").decode()
					gcc_loc = gcc_loc.split("\r\n")[0]
					compiler_root = gcc_loc[0:gcc_loc.rfind("\\g++") + 1]
		else:
			compiler_root = ""

		toolset = cfg_build.toolset
		if ("win_store" == target_platform) or ("win_phone" == target_platform):
			toolset = "auto"
		elif ("" == toolset) or ("auto" == toolset):
			if 0 == target_platform.find("win"):
				if "vc140" == compiler:
					toolset = "v140"
				elif "vc120" == compiler:
					toolset = "v120"
			elif "android" == target_platform:
				toolset = "3.6"

		if "" == archs:
			archs = cfg_build.arch
			if "" == archs:
				archs = ("x64", )
				
		if "" == cfg:
			cfg = cfg_build.config
			if "" == cfg:
				cfg = ("Debug", "RelWithDebInfo")

		multi_config = False
		compilers = []
		if "vc140" == compiler:
			compiler_name = "vc"
			compiler_version = 140
			multi_config = True
			for arch in archs:
				if "x86" == arch:
					gen_name = "Visual Studio 14"
				elif "arm" == arch:
					gen_name = "Visual Studio 14 ARM"
				elif "x64" == arch:
					gen_name = "Visual Studio 14 Win64"
				compilers.append(compiler_info(arch, gen_name, toolset, compiler_root, vcvarsall_path))
		elif "vc120" == compiler:
			compiler_name = "vc"
			compiler_version = 120
			multi_config = True
			for arch in archs:
				if "x86" == arch:
					gen_name = "Visual Studio 12"
				elif "arm" == arch:
					gen_name = "Visual Studio 12 ARM"
				elif "x64" == arch:
					gen_name = "Visual Studio 12 Win64"
				compilers.append(compiler_info(arch, gen_name, toolset, compiler_root, vcvarsall_path))
		elif "clang" == compiler:
			compiler_name = "clang"
			self.toolset = toolset
			compiler_version = self.retrive_clang_version()
			if "win" == host_platform:
				gen_name = "MinGW Makefiles"
			elif ("darwin" == host_platform) or ("ios" == target_platform):
				gen_name = "Xcode"
				multi_config = True
			else:
				gen_name = "Unix Makefiles"
			for arch in archs:
				compilers.append(compiler_info(arch, gen_name, toolset, compiler_root))
		elif "mingw" == compiler:
			compiler_name = "mgw"
			compiler_version = self.retrive_gcc_version()
			for arch in archs:
				compilers.append(compiler_info(arch, "MinGW Makefiles", toolset, compiler_root))
		elif "gcc" == compiler:
			compiler_name = "gcc"
			compiler_version = self.retrive_gcc_version()
			if "win" == host_platform:
				gen_name = "MinGW Makefiles"
			else:
				gen_name = "Unix Makefiles"
			for arch in archs:
				compilers.append(compiler_info(arch, gen_name, toolset, compiler_root))
		else:
			compiler_name = ""
			compiler_version = 0
			log_error("Unsupported compiler.\n")

		if "vc" == compiler_name:
			self.proj_ext_name = "vcxproj"
		else:
			self.proj_ext_name = ""

		self.compiler_name = compiler_name
		self.compiler_version = compiler_version
		self.multi_config = multi_config
		self.compilers = compilers
		self.cfg = cfg

	def msvc_add_build_command(self, batch_cmd, sln_name, proj_name, config, arch = ""):
		batch_cmd.add_command('@SET VisualStudioVersion=%d.0' % (self.compiler_version / 10))
		if len(proj_name) != 0:
			file_name = "%s.%s" % (proj_name, self.proj_ext_name)
		else:
			file_name = "%s.sln" % sln_name
		config_str = "Configuration=%s" % config
		if len(arch) != 0:
			config_str = "%s,Platform=%s" % (config_str, arch)
		batch_cmd.add_command('@MSBuild %s /nologo /m /v:m /p:%s' % (file_name, config_str))
		batch_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
		
	def xcodebuild_add_build_command(self, batch_cmd, target_name, config):
		batch_cmd.add_command('xcodebuild -target %s -configuration %s' % (target_name, config))
		batch_cmd.add_command('if (($? != 0)); then exit 1; fi')
		
	def retrive_gcc_version(self):
		gcc_ver = subprocess.check_output(["gcc", "-dumpversion"]).decode()
		gcc_ver_components = gcc_ver.split(".")
		return int(gcc_ver_components[0] + gcc_ver_components[1])

	def retrive_clang_version(self):
		if ("android" == self.target_platform):
			android_ndk_path = os.environ["ANDROID_NDK"]
			prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm-%s" % self.toolset
			if not os.path.isdir(prebuilt_llvm_path):
				prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
			prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows\\bin"
			if not os.path.isdir(prebuilt_clang_path):
				prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows-x86_64\\bin"
			clang_ver = subprocess.check_output([prebuilt_clang_path + "\\clang", "--version"]).decode()
		else:
			clang_ver = subprocess.check_output(["clang", "--version"]).decode()
		clang_ver_tokens = clang_ver.split()
		for i in range(0, len(clang_ver_tokens)):
			if "version" == clang_ver_tokens[i]:
				clang_ver_components = clang_ver_tokens[i + 1].split(".")
				break
		return int(clang_ver_components[0] + clang_ver_components[1])

class batch_command:
	def __init__(self, host_platform):
		self.commands_ = []
		self.host_platform_ = host_platform

	def add_command(self, cmd):
		self.commands_ += [cmd]

	def execute(self):
		batch_file = "kge_build."
		if "win" == self.host_platform_:
			batch_file += "bat"
		else:
			batch_file += "sh"
		batch_f = open(batch_file, "w")
		batch_f.writelines([cmd_line + "\n" for cmd_line in self.commands_])
		batch_f.close()
		if "win" == self.host_platform_:
			ret_code = os.system(batch_file)
		else:
			os.system("chmod 777 " + batch_file)
			ret_code = os.system("./" + batch_file)
		os.remove(batch_file)
		return ret_code

def log_error(message):
	print("[E] %s" % message)
	if 0 == sys.platform.find("win"):
		os.system("pause")
	else:
		os.system("read")
	sys.exit(1)

def log_info(message):
	print("[I] %s" % message)

def log_warning(message):
	print("[W] %s" % message)

def build_a_project(name, build_path, build_info, compiler_info, need_install = False, additional_options = ""):
	curdir = os.path.abspath(os.curdir)

	toolset_name = ""
	if ("vc" == build_info.compiler_name) and (not build_info.is_windows_runtime):
		toolset_name = "-T %s" % compiler_info.toolset
	elif ("android" == build_info.target_platform):
		if ("clang" == build_info.compiler_name):
			android_ndk_path = os.environ["ANDROID_NDK"]
			prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
			if os.path.isdir(prebuilt_llvm_path):
				toolset_name = "clang"
			else:
				toolset_name = "clang%s" % compiler_info.toolset
		else:
			toolset_name = compiler_info.toolset

	if build_info.compiler_name != "vc":
		additional_options += " -DKLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_info.arch
	if "android" == build_info.target_platform:
		additional_options += " -DCMAKE_TOOLCHAIN_FILE=\"%s/cmake/android.toolchain.cmake\"" % curdir
		additional_options += " -DANDROID_NATIVE_API_LEVEL=%d" % build_info.target_api_level
		if "win" == build_info.host_platform:
			android_ndk_path = os.environ["ANDROID_NDK"]
			prebuilt_make_path = android_ndk_path + "\\prebuilt\\windows"
			if not os.path.isdir(prebuilt_make_path):
				prebuilt_make_path = android_ndk_path + "\\prebuilt\\windows-x86_64"
			make_name = prebuilt_make_path + "\\bin\\make.exe"
			additional_options += " -DCMAKE_MAKE_PROGRAM=\"%s\"" % make_name
	elif "darwin" == build_info.target_platform:
		if "x64" == compiler_info.arch:
			additional_options += " -DCMAKE_OSX_ARCHITECTURES=x86_64"
		elif "x86" == compiler_info.arch:
			additional_options += " -DCMAKE_OSX_ARCHITECTURES=i386"
		else:
			log_error("Unsupported Darwin arch\n")
	elif "ios" == build_info.target_platform:
		additional_options += " -DCMAKE_TOOLCHAIN_FILE=\"%s/cmake/iOS.cmake\"" % curdir
		if "arm" == compiler_info.arch:
			additional_options += " -DIOS_PLATFORM=OS"
		elif "x86" == compiler_info.arch:
			additional_options += " -DIOS_PLATFORM=SIMULATOR"
		else:
			log_error("Unsupported iOS arch\n")

	if build_info.multi_config:
		if "vc" == build_info.compiler_name:
			if "x86" == compiler_info.arch:
				vc_option = "x86"
				vc_arch = "Win32"
			elif "x64" == compiler_info.arch:
				vc_option = "x86_amd64"
				vc_arch = "x64"
			elif "arm" == compiler_info.arch:
				vc_option = "x86_arm"
				vc_arch = "ARM"

		if build_info.is_windows_runtime:
			if build_info.is_windows_store:
				system_name = "WindowsStore"
			elif build_info.is_windows_phone:
				system_name = "WindowsPhone"
			additional_options += " -DCMAKE_SYSTEM_NAME=%s -DCMAKE_SYSTEM_VERSION=%s" % (system_name, build_info.target_api_level)

		build_dir = "%s/Build/%s%d_%s_%s" % (build_path, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_info.arch)
		if build_info.is_clean:
			print("Cleaning %s..." % name)

			if os.path.isdir(build_dir):
				shutil.rmtree(build_dir)
		else:
			print("Building %s..." % name)

			if not os.path.exists(build_dir):
				os.makedirs(build_dir)

			os.chdir(build_dir)

			cmake_cmd = batch_command(build_info.host_platform)
			new_path = sys.exec_prefix
			if len(compiler_info.compiler_root) > 0:
				new_path += ";" + compiler_info.compiler_root
			if "win" == build_info.host_platform:
				cmake_cmd.add_command('@SET PATH=%s;%%PATH%%' % new_path)
			else:
				cmake_cmd.add_command('export PATH=$PATH:%s' % new_path)
			cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_info.generator, toolset_name, additional_options, "../cmake"))
			if cmake_cmd.execute() != 0:
				log_error("Config %s failed." % name)

			build_cmd = batch_command(build_info.host_platform)
			if "vc" == build_info.compiler_name:
				build_cmd.add_command('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
			for config in build_info.cfg:
				if "vc" == build_info.compiler_name:
					build_info.msvc_add_build_command(build_cmd, name, "ALL_BUILD", config, vc_arch)
					if need_install:
						build_info.msvc_add_build_command(build_cmd, name, "INSTALL", config, vc_arch)
				elif "clang" == build_info.compiler_name:
					build_info.xcodebuild_add_build_command(build_cmd, "ALL_BUILD", config)
					if need_install and (not build_info.prefer_static):
						build_info.xcodebuild_add_build_command(build_cmd, "install", config)
			if build_cmd.execute() != 0:
				log_error("Build %s failed." % name)

			os.chdir(curdir)

			print("")
	else:
		if "win" == build_info.host_platform:
			if build_info.target_platform != "android":
				make_name = "mingw32-make.exe"
		else:
			make_name = "make"
		make_name += " -j%d" % multiprocessing.cpu_count()

		for config in build_info.cfg:
			build_dir = "%s/Build/%s%d_%s_%s-%s" % (build_path, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_info.arch, config)
			if build_info.is_clean:
				print("Cleaning %s %s..." % (name, config))

				if os.path.isdir(build_dir):
					shutil.rmtree(build_dir)
			else:
				print("Building %s %s..." % (name, config))

				if not os.path.exists(build_dir):
					os.makedirs(build_dir)
					if ("clang" == build_info.compiler_name) and (build_info.target_platform != "android"):
						additional_options += " -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++"

				os.chdir(build_dir)

				config_options = "-DCMAKE_BUILD_TYPE:STRING=\"%s\"" % config
				if "android" == build_info.target_platform:
					config_options += " -DANDROID_ABI=%s" % compiler_info.arch

					if "x86" == compiler_info.arch:
						config_options += " -DANDROID_TOOLCHAIN_NAME=x86-%s" % toolset_name
					elif "x86_64" == compiler_info.arch:
						config_options += " -DANDROID_TOOLCHAIN_NAME=x86_64-%s" % toolset_name
					elif "arm64-v8a" == compiler_info.arch:
						config_options += " -DANDROID_TOOLCHAIN_NAME=aarch64-linux-android-%s" % toolset_name
					else:
						config_options += " -DANDROID_TOOLCHAIN_NAME=arm-linux-androideabi-%s" % toolset_name

					if "clang" == build_info.compiler_name:
						config_options += " -DANDROID_STL=c++_static"

				cmake_cmd = batch_command(build_info.host_platform)
				cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_info.generator, additional_options, config_options, "../cmake"))
				if cmake_cmd.execute() != 0:
					log_error("Config %s failed." % name)		

				install_str = ""
				if need_install and (not build_info.prefer_static):
					install_str = "install"
				build_cmd = batch_command(build_info.host_platform)
				if "win" == build_info.host_platform:
					build_cmd.add_command("@%s %s" % (make_name, install_str))
					build_cmd.add_command('@if ERRORLEVEL 1 exit /B 1')
				else:
					build_cmd.add_command("%s %s" % (make_name, install_str))
					build_cmd.add_command('if [ $? -ne 0 ]; then exit 1; fi')
				if build_cmd.execute() != 0:
					log_error("Build %s failed." % name)

				os.chdir(curdir)

				print("")
