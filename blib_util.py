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
	def __init__(self, arch, gen_name, compiler_root, vcvarsall_path = ""):
		self.arch = arch
		self.generator = gen_name
		self.compiler_root = compiler_root
		self.vcvarsall_path = vcvarsall_path

class build_info:
	def __init__(self, compiler, archs, cfg):
		try:
			cfg_build.project
		except:
			cfg_build.project = "auto"
		try:
			cfg_build.compiler
		except:
			cfg_build.compiler = "auto"
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
					if "7.1" == android_ver:
						target_api_level = 25
					elif "7.0" == android_ver:
						target_api_level = 24
					elif "6.0" == android_ver:
						target_api_level = 23
					elif "5.1" == android_ver:
						target_api_level = 22
					else:
						log_error("Unsupported android version\n")
				else:
					target_api_level = 22
				self.target_api_level = target_api_level
			elif 0 == target_platform.find("win_store"):
				space_place = target_platform.find(' ')
				if space_place != -1:
					target_api_level = target_platform[space_place + 1:]
					target_platform = target_platform[0:space_place]
				else:
					target_api_level = "10.0"
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
			project_type = ""
			compiler = ""
			if ("auto" == cfg_build.project) and ("auto" == cfg_build.compiler):
				if 0 == target_platform.find("win"):
					if "ProgramFiles" in env:
						program_files_folder = env["ProgramFiles"]
					else:
						program_files_folder = "C:\Program Files"

					if "VS150COMNTOOLS" in env:
						project_type = "vs2017"
						compiler = "vc141"
					else:
						editions = ("Community", "Professional", "Enterprise")
						for edition in editions:
							if os.path.exists(program_files_folder + "\\Microsoft Visual Studio\\2017\\%s\\VC\\Auxiliary\\Build\\VCVARSALL.BAT" % edition):
								project_type = "vs2017"
								compiler = "vc141"
								break
					if 0 == len(compiler):
						if ("VS140COMNTOOLS" in env) or os.path.exists(program_files_folder + "\\Microsoft Visual Studio 14.0\\VC\\VCVARSALL.BAT"):
							project_type = "vs2015"
							compiler = "vc140"
						elif 0 == os.system("where clang++"):
							project_type = "make"
							compiler = "clang"
						elif 0 == os.system("where g++"):
							project_type = "make"
							compiler = "mingw"
				elif ("linux" == target_platform):
					project_type = "make"
					compiler = "gcc"
				elif ("android" == target_platform):
					project_type = "make"
					compiler = "clang"
				elif ("darwin" == target_platform) or ("ios" == target_platform):
					project_type = "xcode"
					compiler = "clang"
				else:
					log_error("Unsupported target platform.\n")
			else:
				if cfg_build.project != "auto":
					project_type = cfg_build.project
				if cfg_build.compiler != "auto":
					compiler = cfg_build.compiler

		if 0 == target_platform.find("win"):
			if "ProgramFiles" in env:
				program_files_folder = env["ProgramFiles"]
			else:
				program_files_folder = "C:\Program Files"

			if "vc141" == compiler:
				if "VS150COMNTOOLS" in env:
					compiler_root = env["VS150COMNTOOLS"] + "..\\..\\VC\\Auxiliary\\Build\\"
					vcvarsall_path = "VCVARSALL.BAT"
				else:
					editions = ("Community", "Professional", "Enterprise")
					found = False
					for edition in editions:
						try_folder = program_files_folder + "\\Microsoft Visual Studio\\2017\\%s\\VC\\Auxiliary\\Build\\" % edition
						try_vcvarsall = "VCVARSALL.BAT"
						if os.path.exists(try_folder + try_vcvarsall):
							compiler_root = try_folder
							vcvarsall_path = try_vcvarsall
							found = True
							break
					if not found:
						log_error("Can't find the compiler.\n")
			elif "vc140" == compiler:
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
			elif "clangc2" == compiler:
				if "VS140COMNTOOLS" in env:
					compiler_root = env["VS140COMNTOOLS"] + "..\\..\\VC\\ClangC2\\bin\\x86\\"
					vcvarsall_path = "..\\..\\..\\VCVARSALL.BAT"
				else:
					try_folder = program_files_folder + "\\Microsoft Visual Studio 14.0\\VC\\ClangC2\\bin\\x86\\"
					try_vcvarsall = "..\\..\\..\\VCVARSALL.BAT"
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
				gcc_loc = subprocess.check_output("where g++").decode()
				gcc_loc = gcc_loc.split("\r\n")[0]
				compiler_root = gcc_loc[0:gcc_loc.rfind("\\g++") + 1]
		else:
			compiler_root = ""

		if "" == project_type:
			if "vc141" == compiler:
				project_type = "vs2017"
			elif "vc140" == compiler:
				project_type = "vs2015"
			elif ("clang" == compiler) and (("darwin" == target_platform) or ("ios" == target_platform)):
				project_type = "xcode"
			else:
				project_type = "make"

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
		if "vs2017" == project_type:
			self.vs_version = 15
			if "vc141" == compiler:
				compiler_name = "vc"
				compiler_version = 141
			elif "vc140" == compiler:
				compiler_name = "vc"
				compiler_version = 140
			elif "clangc2" == compiler:
				compiler_name = "clang"
				compiler_version = self.retrive_clang_version(compiler_root)
			else:
				log_error("Wrong combination between project and compiler")
			multi_config = True
			for arch in archs:
				if "x86" == arch:
					gen_suffix = ""
				elif "arm" == arch:
					gen_suffix = " ARM"
				elif "x64" == arch:
					gen_suffix = " Win64"
				compilers.append(compiler_info(arch, "Visual Studio 15" + gen_suffix, compiler_root, vcvarsall_path))
		elif "vs2015" == project_type:
			self.vs_version = 14
			if "vc140" == compiler:
				compiler_name = "vc"
				compiler_version = 140
			elif "clangc2" == compiler:
				compiler_name = "clang"
				compiler_version = self.retrive_clang_version(compiler_root)
			else:
				log_error("Wrong combination between project and compiler")
			multi_config = True
			for arch in archs:
				if "x86" == arch:
					gen_suffix = ""
				elif "arm" == arch:
					gen_suffix = " ARM"
				elif "x64" == arch:
					gen_suffix = " Win64"
				compilers.append(compiler_info(arch, "Visual Studio 14" + gen_suffix, compiler_root, vcvarsall_path))
		elif "xcode" == project_type:
			if "clang" == compiler:
				compiler_name = "clang"
				compiler_version = self.retrive_clang_version()
				gen_name = "Xcode"
				multi_config = True
				for arch in archs:
					compilers.append(compiler_info(arch, gen_name, compiler_root))
			else:
				log_error("Wrong combination between project and compiler")
		elif "make" == project_type:
			if "win" == host_platform:
				gen_name = "MinGW Makefiles"
			else:
				gen_name = "Unix Makefiles"
			if "clang" == compiler:
				compiler_name = "clang"
				compiler_version = self.retrive_clang_version()
				for arch in archs:
					compilers.append(compiler_info(arch, gen_name, compiler_root))
			elif "mingw" == compiler:
				compiler_name = "mgw"
				compiler_version = self.retrive_gcc_version()
				for arch in archs:
					compilers.append(compiler_info(arch, gen_name, compiler_root))
			elif "gcc" == compiler:
				compiler_name = "gcc"
				compiler_version = self.retrive_gcc_version()
				for arch in archs:
					compilers.append(compiler_info(arch, gen_name, compiler_root))
			else:
				log_error("Wrong combination between project and compiler")
		else:
			compiler_name = ""
			compiler_version = 0
			log_error("Unsupported compiler.\n")

		if 0 == project_type.find("vs"):
			self.proj_ext_name = "vcxproj"

		self.project_type = project_type
		self.compiler_name = compiler_name
		self.compiler_version = compiler_version
		self.multi_config = multi_config
		self.compilers = compilers
		self.cfg = cfg

	def msbuild_add_build_command(self, batch_cmd, sln_name, proj_name, config, arch = ""):
		batch_cmd.add_command('@SET VisualStudioVersion=%d.0' % self.vs_version)
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

	def retrive_clang_version(self, path = ""):
		if ("android" == self.target_platform):
			android_ndk_path = os.environ["ANDROID_NDK"]
			prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
			prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows\\bin"
			if not os.path.isdir(prebuilt_clang_path):
				prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows-x86_64\\bin"
			clang_path = prebuilt_clang_path + "\\clang"
		else:
			clang_path = path + "clang"
		clang_ver = subprocess.check_output([clang_path, "--version"]).decode()
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
	if (0 == build_info.project_type.find("vs")) and (not build_info.is_windows_store):
		if "vc" == build_info.compiler_name:
			toolset_name = "v%s" % build_info.compiler_version
		else:
			if "vs2017" == build_info.project_type:
				toolset_name = "v141_clang_c2"
			else:
				toolset_name = "v140_clang_c2"
		toolset_name = "-T %s" % toolset_name
	elif ("android" == build_info.target_platform):
		android_ndk_path = os.environ["ANDROID_NDK"]
		prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
		toolset_name = "clang"

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
		if 0 == build_info.project_type.find("vs"):
			if "x86" == compiler_info.arch:
				vc_option = "x86"
				vc_arch = "Win32"
			elif "x64" == compiler_info.arch:
				vc_option = "x86_amd64"
				vc_arch = "x64"
			elif "arm" == compiler_info.arch:
				vc_option = "x86_arm"
				vc_arch = "ARM"

		if build_info.is_windows_store:
			additional_options += " -DCMAKE_SYSTEM_NAME=\"WindowsStore\" -DCMAKE_SYSTEM_VERSION=%s" % build_info.target_api_level

		build_dir = "%s/Build/%s_%s%d_%s_%s" % (build_path, build_info.project_type, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_info.arch)
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
			if 0 == build_info.project_type.find("vs"):
				build_cmd.add_command('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
			for config in build_info.cfg:
				if 0 == build_info.project_type.find("vs"):
					build_info.msbuild_add_build_command(build_cmd, name, "ALL_BUILD", config, vc_arch)
					if need_install:
						build_info.msbuild_add_build_command(build_cmd, name, "INSTALL", config, vc_arch)
				elif "xcode" == build_info.project_type:
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
			build_dir = "%s/Build/%s_%s%d_%s_%s-%s" % (build_path, build_info.project_type, build_info.compiler_name, build_info.compiler_version, build_info.target_platform, compiler_info.arch, config)
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
					if "x86" == compiler_info.arch:
						abi_arch = "x86"
						toolchain_arch = "x86"
					elif "x86_64" == compiler_info.arch:
						abi_arch = "x86_64"
						toolchain_arch = "x86_64"
					elif "arm64-v8a" == compiler_info.arch:
						abi_arch = "arm64-v8a"
						toolchain_arch = "aarch64-linux-android"
					else:
						abi_arch = "armeabi-v7a with NEON"
						toolchain_arch = "arm-linux-androideabi"
					config_options += " -DANDROID_STL=c++_static -DANDROID_ABI=\"%s\" -DANDROID_TOOLCHAIN_NAME=%s-clang" % (abi_arch, toolchain_arch)

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
