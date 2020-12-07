#!/usr/bin/env python
#-*- coding: ascii -*-

import os, sys, multiprocessing, subprocess, shutil, platform

class CfgBuildDefault:
	def __init__(self):
		#################################################
		# !!!! DO NOT DELETE ANY FIELD OF THIS CLASS !!!!
		#################################################

		# The path of cmake executable. Could fill in the path, or "auto".
		self.cmake_path = "auto"

		# Project type.
		#   On Windows desktop, could be "vs2019", "vs2017", "make", "ninja", "auto".
		#   On Windows store, could be "vs2019", "vs2017", "auto".
		#   On Android, could be "make", "auto".
		#   On Linux, could be "make", "ninja", "auto".
		#   On macOS, could be "xcode", "ninja", "auto".
		#   On iOS, could be "xcode", "ninja", "auto".
		self.project = "auto"

		# Compiler name.
		#   On Windows desktop, could be "vc142", "vc141", "mingw", "clangcl", "auto".
		#   On Windows store, could be "vc142", "vc141", "auto".
		#   On Android, could be "clang", "auto".
		#   On Linux, could be "gcc", "auto".
		#   On macOS, could be "clang", "auto".
		#   On iOS, could be "clang", "auto".
		self.compiler = "auto"

		# Target CPU architecture.
		#   On Windows desktop, could be "arm64", "x64".
		#   On Windows store, could be "arm64", "arm", "x64".
		#   On Android, cound be "arm64-v8a", "x86", "x86_64".
		#   On Linux, could be "x64".
		#   On macOS, could be "x64".
		#   On iOS, could be "arm", "x86".
		self.arch = ("x64", )

		# Configuration. Could be "Debug", "Release", "MinSizeRel", "RelWithDebInfo".
		self.config = ("Debug", "RelWithDebInfo")

		# Target platform for cross compiling.
		#   On Windows desktop, could be "auto".
		#   On Windows store, could be "win_store 10.0".
		#   On Android, cound be "android 5.1", "android 6.0", "android 7.0", "android 7.1".
		#   On Linux, could be "auto".
		#   On macOS, could be "auto".
		#   On iOS, could be "ios".
		self.target = "auto"

		# A name for offline FXML compiling. Could be one of "d3d_11_0", "gles_3_0", "gles_3_1", "gles_3_2", or "auto".
		self.shader_platform_name = "auto"

		# The path of GLES SDK's include. Could fill in the path, or "auto".
		self.gles_include_dir = "auto"

		# The path of LibOVR. Could fill in the path, or "auto".
		self.libovr_path = "auto"

		# The directory of prebuilt host binaries for cross-compiling.
		self.host_bin_dir = ""

def GenerateCfgBuildFromDefault():
	print("Generating CfgBuild.py ...")
	sys.stdout.flush()
	shutil.copyfile("CfgBuildDefault.py", "CfgBuild.py")

def LogError(message):
	print("[E] %s" % message)
	sys.stdout.flush()
	if sys.platform.startswith("win"):
		pause_cmd = "pause"
	else:
		pause_cmd = "read"
	subprocess.call(pause_cmd, shell = True)
	sys.exit(1)

def LogInfo(message):
	print("[I] %s" % message)
	sys.stdout.flush()

def LogWarning(message):
	print("[W] %s" % message)
	sys.stdout.flush()

class CompilerInfo:
	def __init__(self, build_info, arch, gen_name, compiler_root, vcvarsall_path = "", vcvarsall_options = ""):
		self.arch = arch
		self.generator = gen_name
		self.compiler_root = compiler_root
		self.vcvarsall_path = vcvarsall_path
		self.vcvarsall_options = vcvarsall_options

		self.is_cross_compiling = build_info.is_cross_compiling
		if build_info.host_arch != arch:
			self.is_cross_compiling = True

class BuildInfo:
	@classmethod
	def FromArgv(BuildInfo, argv, base = 0):
		project = ""
		compiler = ""
		archs = ""
		cfg = ""
		target = ""

		argc = len(argv)
		if argc > base + 1:
			project = argv[base + 1]
		if argc > base + 2:
			compiler = argv[base + 2]
		if argc > base + 3:
			archs = (argv[base + 3], )
		if argc > base + 4:
			cfg = (argv[base + 4], )
		if argc > base + 5:
			target = argv[base + 5]

		return BuildInfo(project, compiler, archs, cfg, target)

	def __init__(self, project, compiler, archs, cfg, target):
		try:
			import cfg_build
			GenerateCfgBuildFromDefault()
			hint_for_migrating = True
		except:
			hint_for_migrating = False

		if hint_for_migrating:
			LogError("cfg_build.py is retired. Please migrate your configurations to new CfgBuild.py.")

		try:
			from CfgBuild import ActivedCfgBuild
		except:
			GenerateCfgBuildFromDefault()
			from CfgBuild import ActivedCfgBuild

		cfg_build = ActivedCfgBuild()
		try:
			cfg_build.cmake_path
		except:
			cfg_build.cmake_path = "auto"
		if len(project) > 0:
			cfg_build.project = project
		else:
			try:
				cfg_build.project
			except:
				cfg_build.project = "auto"
		if (len(compiler) > 0) and (compiler.lower() != "clean"):
			cfg_build.compiler = compiler
		else:
			try:
				cfg_build.compiler
			except:
				cfg_build.compiler = "auto"
		if len(archs) > 0:
			cfg_build.arch = archs
		else:
			try:
				cfg_build.arch
			except:
				cfg_build.arch = ("x64", )
		if len(cfg) > 0:
			cfg_build.config = cfg
		else:
			try:
				cfg_build.config
			except:
				cfg_build.config = ("Debug", "RelWithDebInfo")
		if len(target) > 0:
			cfg_build.target = target
		else:
			try:
				cfg_build.target
			except:
				cfg_build.target = "auto"
		try:
			cfg_build.shader_platform_name
		except:
			cfg_build.shader_platform_name = "auto"
		try:
			cfg_build.gles_include_dir
		except:
			cfg_build.gles_include_dir = "auto"
		try:
			cfg_build.libovr_path
		except:
			cfg_build.libovr_path = "auto"
		env = os.environ

		host_platform = sys.platform
		if host_platform.startswith("win"):
			host_platform = "win"
		elif host_platform.startswith("linux"):
			host_platform = "linux"
		elif host_platform.startswith("darwin"):
			host_platform = "darwin"
		if "auto" == cfg_build.target:
			target_platform = host_platform
		else:
			target_platform = cfg_build.target
			if target_platform.startswith("android"):
				if not "ANDROID_NDK" in env:
					LogError("You must define an ANDROID_NDK environment variable to your location of NDK.\n")

				space_place = target_platform.find(' ')
				if space_place != -1:
					android_ver = target_platform[space_place + 1:]
					target_platform = target_platform[0:space_place]
					if "8.1" == android_ver:
						target_api_level = 27
					elif "8.0" == android_ver:
						target_api_level = 26
					elif "7.1" == android_ver:
						target_api_level = 25
					elif "7.0" == android_ver:
						target_api_level = 24
					elif "6.0" == android_ver:
						target_api_level = 23
					elif "5.1" == android_ver:
						target_api_level = 22
					else:
						LogError("Unsupported Android version.\n")
				else:
					target_api_level = 22
				self.target_api_level = target_api_level
			elif target_platform.startswith("win_store"):
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
		self.is_cross_compiling = host_platform != target_platform
		self.shader_platform_name = shader_platform_name
		self.prefer_static = prefer_static
		self.is_clean = ("clean" == compiler)

		self.host_arch = platform.machine()
		if (self.host_arch == "AMD64") or (self.host_arch == "x86_64"):
			self.host_arch = "x64"
		elif (self.host_arch == "ARM64"):
			self.host_arch = "arm64"
		else:
			LogError("Unknown host architecture %s.\n" % self.host_arch)

		if self.host_platform == "win":
			self.sep = "\r\n"
		else:
			self.sep = "\n"

		self.cmake_path = cfg_build.cmake_path
		if self.cmake_path == "auto":
			self.cmake_path = self.FindCMake()
		self.cmake_ver = self.RetrieveCMakeVersion()
		if self.cmake_ver < 39:
			LogError("CMake 3.9+ is required.")

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

		if len(cfg_build.project) > 0:
			project_type = cfg_build.project
		else:
			project_type = ""
		if ("" == compiler) or self.is_clean:
			compiler = ""
			if ("auto" == cfg_build.project) and ("auto" == cfg_build.compiler):
				if self.is_windows:
					program_files_folder = self.FindProgramFilesFolder()

					if len(self.FindVS2019Folder(program_files_folder)) > 0:
						project_type = "vs2019"
						compiler = "vc142"
					elif len(self.FindVS2017Folder(program_files_folder)) > 0:
						project_type = "vs2017"
						compiler = "vc141"
					elif len(self.FindClang()) != 0:
						project_type = "make"
						compiler = "clang"
					elif len(self.FindGCC()) != 0:
						project_type = "make"
						compiler = "mingw"
				elif self.is_linux:
					project_type = "make"
					compiler = "gcc"
				elif self.is_android:
					project_type = "make"
					compiler = "clang"
				elif self.is_darwin or self.is_ios:
					project_type = "xcode"
					compiler = "clang"
				else:
					LogError("Unsupported target platform %s.\n" % target_platform)
			else:
				if cfg_build.project != "auto":
					project_type = cfg_build.project
				if cfg_build.compiler != "auto":
					compiler = cfg_build.compiler

		if (project_type != "") and (compiler == ""):
			if project_type == "vs2019":
				compiler = "vc142"
			elif project_type == "vs2017":
				compiler = "vc141"
			elif project_type == "xcode":
				compiler = "clang"

		if self.is_windows:
			program_files_folder = self.FindProgramFilesFolder()

			if "vc142" == compiler:
				if project_type == "vs2019":
					try_folder = self.FindVS2019Folder(program_files_folder)
					if len(try_folder) > 0:
						compiler_root = try_folder
						vcvarsall_path = "VCVARSALL.BAT"
						vcvarsall_options = ""
					else:
						LogError("Could NOT find vc142 compiler toolset for VS2019.\n")
				else:
					LogError("Could NOT find vc142 compiler.\n")
			elif "vc141" == compiler:
				if project_type == "vs2019":
					try_folder = self.FindVS2019Folder(program_files_folder)
					if len(try_folder) > 0:
						compiler_root = try_folder
						vcvarsall_path = "VCVARSALL.BAT"
						vcvarsall_options = "-vcvars_ver=14.1"
					else:
						LogError("Could NOT find vc141 compiler toolset for VS2019.\n")
				else:
					try_folder = self.FindVS2017Folder(program_files_folder)
					if len(try_folder) > 0:
						compiler_root = try_folder
						vcvarsall_path = "VCVARSALL.BAT"
					else:
						LogError("Could NOT find vc141 compiler.\n")
					vcvarsall_options = ""
			elif "clangcl" == compiler:
				if project_type == "vs2019":
					try_folder = self.FindVS2019Folder(program_files_folder)
					if len(try_folder) > 0:
						compiler_root = try_folder
						vcvarsall_path = "VCVARSALL.BAT"
						vcvarsall_options = ""
					else:
						LogError("Could NOT find clang-cl compiler toolset for VS2019.\n")
				else:
					LogError("Could NOT find clang-cl compiler.\n")
			elif "clang" == compiler:
				clang_loc = self.FindClang()
				compiler_root = clang_loc[0:clang_loc.rfind("\\clang++") + 1]
			elif "mingw" == compiler:
				gcc_loc = self.FindGCC()
				compiler_root = gcc_loc[0:gcc_loc.rfind("\\g++") + 1]
		else:
			compiler_root = ""

		if "" == project_type:
			if "vc142" == compiler:
				project_type = "vs2019"
			elif "vc141" == compiler:
				project_type = "vs2017"
			elif ("clang" == compiler) and (self.is_darwin or self.is_ios):
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
		if "vs2019" == project_type:
			self.vs_version = 16
			if "vc142" == compiler:
				compiler_name = "vc"
				compiler_version = 142
			elif "vc141" == compiler:
				compiler_name = "vc"
				compiler_version = 141
			elif "clangcl" == compiler:
				compiler_name = "clangcl"
				compiler_version = self.RetrieveClangVersion(compiler_root + "../../Tools/Llvm/bin/")
			else:
				LogError("Wrong combination of project %s and compiler %s.\n" % (project_type, compiler))
			multi_config = True
			for arch in archs:
				compilers.append(CompilerInfo(self, arch, "Visual Studio 16", compiler_root, vcvarsall_path, vcvarsall_options))
		elif "vs2017" == project_type:
			self.vs_version = 15
			if "vc141" == compiler:
				compiler_name = "vc"
				compiler_version = 141
			else:
				LogError("Wrong combination of project %s and compiler %s.\n" % (project_type, compiler))
			multi_config = True
			for arch in archs:
				compilers.append(CompilerInfo(self, arch, "Visual Studio 15", compiler_root, vcvarsall_path, vcvarsall_options))
		elif "xcode" == project_type:
			if "clang" == compiler:
				compiler_name = "clang"
				compiler_version = self.RetrieveClangVersion()
				gen_name = "Xcode"
				multi_config = True
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root))
			else:
				LogError("Wrong combination of project %s and compiler %s.\n" % (project_type, compiler))
		elif ("make" == project_type) or ("ninja" == project_type):
			if "ninja" == project_type:
				gen_name = "Ninja"
			else:
				if "win" == host_platform:
					gen_name = "MinGW Makefiles"
				else:
					gen_name = "Unix Makefiles"
			if "clang" == compiler:
				compiler_name = "clang"
				compiler_version = self.RetrieveClangVersion()
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root))
			elif "mingw" == compiler:
				compiler_name = "mgw"
				compiler_version = self.RetrieveGCCVersion()
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root))
			elif "gcc" == compiler:
				compiler_name = "gcc"
				compiler_version = self.RetrieveGCCVersion()
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root))
			elif "vc142" == compiler:
				compiler_name = "vc"
				compiler_version = 142
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root, vcvarsall_path, vcvarsall_options))
			elif "vc141" == compiler:
				compiler_name = "vc"
				compiler_version = 141
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root, vcvarsall_path, vcvarsall_options))
			else:
				LogError("Wrong combination of project %s and compiler %s.\n" % (project_type, compiler))
		else:
			compiler_name = ""
			compiler_version = 0
			LogError("Unsupported project type %s.\n" % project_type)

		if project_type.startswith("vs"):
			self.proj_ext_name = "vcxproj"

		self.project_type = project_type
		self.compiler_name = compiler_name
		self.compiler_version = compiler_version
		self.multi_config = multi_config
		self.compilers = compilers
		self.archs = archs
		self.cfg = cfg

		self.gles_include_dir = cfg_build.gles_include_dir
		self.libovr_path = cfg_build.libovr_path
		self.host_bin_dir = cfg_build.host_bin_dir

		self.jobs = multiprocessing.cpu_count()

		self.DisplayInfo();

	def MSBuildAddBuildCommand(self, batch_cmd, sln_name, proj_name, config, arch = ""):
		batch_cmd.AddCommand('@SET VisualStudioVersion=%d.0' % self.vs_version)
		if len(proj_name) != 0:
			file_name = "%s.%s" % (proj_name, self.proj_ext_name)
		else:
			file_name = "%s.sln" % sln_name
		config_str = "Configuration=%s" % config
		if len(arch) != 0:
			config_str = "%s,Platform=%s" % (config_str, arch)
		batch_cmd.AddCommand('@MSBuild %s /nologo /m:%d /v:m /p:%s' % (file_name, self.jobs, config_str))
		batch_cmd.AddCommand('@if ERRORLEVEL 1 exit /B 1')

	def XCodeBuildAddBuildCommand(self, batch_cmd, target_name, config):
		batch_cmd.AddCommand('xcodebuild -quiet -target %s -jobs %d -configuration %s' % (target_name, self.jobs, config))
		batch_cmd.AddCommand('if (($? != 0)); then exit 1; fi')

	def MakeAddBuildCommand(self, batch_cmd, make_name, target):
		make_options = "-j%d" % self.jobs
		if target != "ALL_BUILD":
			make_options += " %s" % target
		if "win" == self.host_platform:
			batch_cmd.AddCommand("@%s %s" % (make_name, make_options))
			batch_cmd.AddCommand('@if ERRORLEVEL 1 exit /B 1')
		else:
			batch_cmd.AddCommand("%s %s" % (make_name, make_options))
			batch_cmd.AddCommand('if [ $? -ne 0 ]; then exit 1; fi')

	def FindGCC(self):
		if self.host_platform != "win":
			env = os.environ
			if "CXX" in env:
				gcc_loc = env["CXX"]
				if len(gcc_loc) != 0:
					return gcc_loc.split(self.sep)[0]

		gcc_loc = shutil.which("g++")
		if len(gcc_loc) == 0:
			LogError("Could NOT find g++. Please install g++, set its path into CXX, or put its path into %%PATH%%.")
		return gcc_loc.split(self.sep)[0]

	def RetrieveGCCVersion(self):
		gcc_ver = subprocess.check_output([self.FindGCC(), "-dumpfullversion"]).decode()
		gcc_ver_components = gcc_ver.split(".")
		return int(gcc_ver_components[0] + gcc_ver_components[1])

	def FindClang(self):
		if self.host_platform != "win":
			env = os.environ
			if "CXX" in env:
				clang_loc = env["CXX"]
				if len(clang_loc) != 0:
					return clang_loc.split(self.sep)[0]

		clang_loc = shutil.which("clang++")
		if len(clang_loc) == 0:
			LogError("Could NOT find g++. Please install clang++, set its path into CXX, or put its path into %%PATH%%.")
		return clang_loc.split(self.sep)[0]

	def RetrieveClangVersion(self, path = ""):
		if self.is_android:
			android_ndk_path = os.environ["ANDROID_NDK"]
			prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
			prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows\\bin"
			if not os.path.isdir(prebuilt_clang_path):
				prebuilt_clang_path = prebuilt_llvm_path + "\\prebuilt\\windows-x86_64\\bin"
			clang_path = prebuilt_clang_path + "\\clang"
		elif (len(path) > 0):
			clang_path = path + "clang"
		else:
			clang_path = self.FindClang()
		clang_ver = subprocess.check_output([clang_path, "--version"]).decode()
		clang_ver_tokens = clang_ver.split()
		for i in range(0, len(clang_ver_tokens)):
			if "version" == clang_ver_tokens[i]:
				clang_ver_components = clang_ver_tokens[i + 1].split(".")
				break
		return int(clang_ver_components[0] + clang_ver_components[1])

	def FindProgramFilesFolder(self):
		env = os.environ
		if platform.architecture()[0] == '64bit':
			if "ProgramFiles(x86)" in env:
				program_files_folder = env["ProgramFiles(x86)"]
			else:
				program_files_folder = "C:\Program Files (x86)"
		else:
			if "ProgramFiles" in env:
				program_files_folder = env["ProgramFiles"]
			else:
				program_files_folder = "C:\Program Files"
		return program_files_folder

	def FindVS2019Folder(self, program_files_folder):
		return self.FindVS2017PlusFolder(program_files_folder, 16, "2019")

	def FindVS2017Folder(self, program_files_folder):
		return self.FindVS2017PlusFolder(program_files_folder, 15, "2017")

	def FindVS2017PlusFolder(self, program_files_folder, vs_version, vs_name):
		try_vswhere_location = program_files_folder + "\\Microsoft Visual Studio\\Installer\\vswhere.exe"
		if os.path.exists(try_vswhere_location):
			vs_location = subprocess.check_output([try_vswhere_location,
				"-products", "*",
				"-latest",
				"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
				"-property", "installationPath",
				"-version", "[%d.0,%d.0)" % (vs_version, vs_version + 1),
				"-prerelease"]).decode().split("\r\n")[0]
			try_folder = vs_location + "\\VC\\Auxiliary\\Build\\"
			try_vcvarsall = "VCVARSALL.BAT"
			if os.path.exists(try_folder + try_vcvarsall):
				return try_folder
		else:
			names = ("Preview", vs_name)
			skus = ("Community", "Professional", "Enterprise", "BuildTools")
			for name in names:
				for sku in skus:
					try_folder = program_files_folder + "\\Microsoft Visual Studio\\%s\\%s\\VC\\Auxiliary\\Build\\" % (name, sku)
					try_vcvarsall = "VCVARSALL.BAT"
					if os.path.exists(try_folder + try_vcvarsall):
						return try_folder
		return ""

	def FindCMake(self):
		cmake_loc = shutil.which("cmake")
		if len(cmake_loc) == 0:
			LogError("Could NOT find CMake. Please install CMake 3.9+, set its path into CfgBuild's self.cmake_path, or put its path into %%PATH%%.")
		return cmake_loc.split(self.sep)[0]

	def RetrieveCMakeVersion(self):
		cmake_ver = subprocess.check_output([self.cmake_path, "--version"]).decode()
		if len(cmake_ver) == 0:
			LogError("Could NOT find CMake. Please install CMake 3.9+, set its path into CfgBuild's self.cmake_path, or put its path into %%PATH%%.")
		cmake_ver = cmake_ver.split()[2]
		cmake_ver_components = cmake_ver.split('.')
		return int(cmake_ver_components[0] + cmake_ver_components[1])

	def DisplayInfo(self):
		print("Build information:")
		print("\tCMake path: %s" % self.cmake_path)
		print("\tCMake version: %s" % self.cmake_ver)
		print("\tHost platform: %s" % self.host_platform)
		print("\tTarget platform: %s" % self.target_platform)
		if self.is_android:
			print("\tTarget API level: %d" % self.target_api_level)
		elif self.is_windows_store:
			print("\tTarget API level: %s" % self.target_api_level)
		print("\tCPU count: %d" % self.jobs)
		print("\tPrefer static library: %s" % self.prefer_static)
		print("\tShader platform: %s" % self.shader_platform_name)
		print("\tIs dev platform: %s" % self.is_dev_platform)
		print("\tProject type: %s" % self.project_type)
		print("\tCompiler: %s%d" % (self.compiler_name, self.compiler_version))
		archs = ""
		for i in range(0, len(self.compilers)):
			archs += self.compilers[i].arch
			if i != len(self.compilers) - 1:
				archs += ", "
		print("\tArchitectures: %s" % archs)
		cfgs = ""
		for i in range(0, len(self.cfg)):
			cfgs += self.cfg[i]
			if i != len(self.cfg) - 1:
				cfgs += ", "
		print("\tConfigures: %s" % cfgs)
		print("\tGLES SDK include path: %s" % self.gles_include_dir)
		if self.is_windows_desktop:
			print("\tOculus LibOVR path: %s" % self.libovr_path)
		if len(self.host_bin_dir) > 0:
			print("\tHost binary dir: %s" % self.host_bin_dir)
		
		print("")
		sys.stdout.flush()

	def GetBuildDir(self, arch, config = None):
		env = os.environ
		if "BUILD_DIR" in env:
			build_dir_name = env["BUILD_DIR"]
		else:
			build_dir_name = "%s_%s%d_%s_%s" % (self.project_type, self.compiler_name, self.compiler_version, self.target_platform, arch)
			if not (config is None):
				build_dir_name += "-" + config
		return build_dir_name

class BatchCommand:
	def __init__(self, host_platform):
		self.commands_ = []
		self.host_platform_ = host_platform

	def AddCommand(self, cmd):
		self.commands_ += [cmd]

	def Execute(self):
		batch_file = "kge_build."
		if "win" == self.host_platform_:
			batch_file += "bat"
		else:
			batch_file += "sh"
		batch_f = open(batch_file, "w")
		batch_f.writelines([cmd_line + "\n" for cmd_line in self.commands_])
		batch_f.close()
		if "win" == self.host_platform_:
			ret_code = subprocess.call(batch_file, shell = True)
		else:
			subprocess.call("chmod 777 " + batch_file, shell = True)
			ret_code = subprocess.call("./" + batch_file, shell = True)
		os.remove(batch_file)
		return ret_code

def BuildProjects(name, build_path, build_info, compiler_info, project_list, additional_options = ""):
	curdir = os.path.abspath(os.curdir)

	toolset_name = ""
	if build_info.project_type.startswith("vs"):
		toolset_name = "-T"
		if build_info.compiler_name == "clangcl":
			toolset_name += " ClangCL"
		else:
			if not build_info.is_windows_store:
				toolset_name += " v%s," % build_info.compiler_version
			toolset_name += "host=x64"
	elif build_info.is_android:
		android_ndk_path = os.environ["ANDROID_NDK"]
		prebuilt_llvm_path = android_ndk_path + "\\toolchains\\llvm"
		toolset_name = "clang"

	if (build_info.compiler_name != "vc") or (build_info.project_type == "ninja"):
		additional_options += " -DKLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_info.arch
	if build_info.is_android:
		additional_options += " -DCMAKE_TOOLCHAIN_FILE=\"%s/Build/CMake/Modules/android.toolchain.cmake\"" % curdir
		additional_options += " -DANDROID_NATIVE_API_LEVEL=%d" % build_info.target_api_level
	elif build_info.is_darwin:
		if "x64" == compiler_info.arch:
			additional_options += " -DCMAKE_OSX_ARCHITECTURES=x86_64"
		else:
			LogError("Unsupported Darwin architecture.\n")
	elif build_info.is_ios:
		additional_options += " -DCMAKE_TOOLCHAIN_FILE=\"%s/Build/CMake/Modules/iOS.cmake\"" % curdir
		if "arm" == compiler_info.arch:
			additional_options += " -DIOS_PLATFORM=OS"
		elif "x86" == compiler_info.arch:
			additional_options += " -DIOS_PLATFORM=SIMULATOR"
		else:
			LogError("Unsupported iOS architecture.\n")

	if compiler_info.is_cross_compiling:
		additional_options += " -DKLAYGE_HOST_BIN_DIR=\"%s\"" % build_info.host_bin_dir

	if build_info.project_type.startswith("vs"):
		if "x64" == compiler_info.arch:
			vc_option = "amd64"
			vc_arch = "x64"
		elif "arm" == compiler_info.arch:
			vc_option = "amd64_arm"
			vc_arch = "ARM"
		elif "arm64" == compiler_info.arch:
			vc_option = "amd64_arm64"
			vc_arch = "ARM64"
		else:
			LogError("Unsupported VS architecture.\n")
		if len(compiler_info.vcvarsall_options) > 0:
			vc_option += " %s" % compiler_info.vcvarsall_options

	if build_info.multi_config:
		if build_info.project_type.startswith("vs"):
			additional_options += " -A %s" % vc_arch
			if build_info.compiler_name == "clangcl":
				additional_options += " -DClangCL_Path=\"" + compiler_info.compiler_root + "../../Tools/Llvm/bin/\""

		if build_info.is_windows_store:
			additional_options += " -DCMAKE_SYSTEM_NAME=\"WindowsStore\" -DCMAKE_SYSTEM_VERSION=%s" % build_info.target_api_level

		build_dir = "%s/Build/%s" % (build_path, build_info.GetBuildDir(compiler_info.arch))
		if build_info.is_clean:
			print("Cleaning %s..." % name)
			sys.stdout.flush()

			if os.path.isdir(build_dir):
				shutil.rmtree(build_dir)
		else:
			print("Building %s..." % name)
			sys.stdout.flush()

			if not os.path.exists(build_dir):
				os.makedirs(build_dir)

			build_dir = os.path.abspath(build_dir)
			os.chdir(build_dir)

			cmake_cmd = BatchCommand(build_info.host_platform)
			new_path = sys.exec_prefix
			if len(compiler_info.compiler_root) > 0:
				new_path += ";" + compiler_info.compiler_root
			if "win" == build_info.host_platform:
				cmake_cmd.AddCommand('@SET PATH=%s;%%PATH%%' % new_path)
				if build_info.project_type.startswith("vs"):
					cmake_cmd.AddCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
					cmake_cmd.AddCommand('@CD /d "%s"' % build_dir)
			else:
				cmake_cmd.AddCommand('export PATH=$PATH:%s' % new_path)
			cmake_cmd.AddCommand('"%s" -G "%s" %s %s ../CMake' % (build_info.cmake_path, compiler_info.generator, toolset_name, additional_options))
			if cmake_cmd.Execute() != 0:
				LogWarning("Config %s failed, retry 1...\n" % name)
				if cmake_cmd.Execute() != 0:
					LogWarning("Config %s failed, retry 2...\n" % name)
					if cmake_cmd.Execute() != 0:
						LogError("Config %s failed.\n" % name)

			build_cmd = BatchCommand(build_info.host_platform)
			if build_info.project_type.startswith("vs"):
				build_cmd.AddCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
				build_cmd.AddCommand('@CD /d "%s"' % build_dir)
			for config in build_info.cfg:
				for target in project_list:
					if build_info.project_type.startswith("vs"):
						build_info.MSBuildAddBuildCommand(build_cmd, name, target, config, vc_arch)
					elif "xcode" == build_info.project_type:
						build_info.XCodeBuildAddBuildCommand(build_cmd, target, config)
			if build_cmd.Execute() != 0:
				LogError("Build %s failed.\n" % name)

			os.chdir(curdir)

			print("")
			sys.stdout.flush()
	else:
		if build_info.project_type == "ninja":
			make_name = "ninja"
		else:
			if "win" == build_info.host_platform:
				if build_info.is_android:
					prebuilt_make_path = android_ndk_path + "\\prebuilt\\windows"
					if not os.path.isdir(prebuilt_make_path):
						prebuilt_make_path = android_ndk_path + "\\prebuilt\\windows-x86_64"
					make_name = prebuilt_make_path + "\\bin\\make.exe"
				else:
					make_name = "mingw32-make.exe"
			else:
				make_name = "make"

		for config in build_info.cfg:
			build_dir = "%s/Build/%s" % (build_path, build_info.GetBuildDir(compiler_info.arch, config))
			if build_info.is_clean:
				print("Cleaning %s %s..." % (name, config))
				sys.stdout.flush()

				if os.path.isdir(build_dir):
					shutil.rmtree(build_dir)
			else:
				print("Building %s %s..." % (name, config))
				sys.stdout.flush()

				if not os.path.exists(build_dir):
					os.makedirs(build_dir)
					if build_info.is_android:
						additional_options += " -DCMAKE_MAKE_PROGRAM=\"%s\"" % make_name
					elif ("clang" == build_info.compiler_name):
						env = os.environ
						if not ("CC" in env):
							additional_options += " -DCMAKE_C_COMPILER=clang"
						if not ("CXX" in env):
							additional_options += " -DCMAKE_CXX_COMPILER=clang++"

				build_dir = os.path.abspath(build_dir)
				os.chdir(build_dir)

				additional_options += " -DCMAKE_BUILD_TYPE:STRING=\"%s\"" % config
				if build_info.is_android:
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
						LogError("Unsupported Android architecture.\n")
					additional_options += " -DANDROID_STL=c++_static -DANDROID_ABI=\"%s\" -DANDROID_TOOLCHAIN_NAME=%s-clang" % (abi_arch, toolchain_arch)

				cmake_cmd = BatchCommand(build_info.host_platform)
				new_path = sys.exec_prefix
				if len(compiler_info.compiler_root) > 0:
					new_path += ";" + compiler_info.compiler_root
				if build_info.compiler_name == "vc":
					cmake_cmd.AddCommand('@SET PATH=%s;%%PATH%%' % new_path)
					cmake_cmd.AddCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
					cmake_cmd.AddCommand('@CD /d "%s"' % build_dir)
					additional_options += " -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe"
				cmake_cmd.AddCommand('"%s" -G "%s" %s ../CMake' % (build_info.cmake_path, compiler_info.generator, additional_options))
				if cmake_cmd.Execute() != 0:
					LogWarning("Config %s failed, retry 1...\n" % name)
					if cmake_cmd.Execute() != 0:
						LogWarning("Config %s failed, retry 2...\n" % name)
						if cmake_cmd.Execute() != 0:
							LogError("Config %s failed.\n" % name)

				build_cmd = BatchCommand(build_info.host_platform)
				if build_info.compiler_name == "vc":
					build_cmd.AddCommand('@CALL "%s%s" %s' % (compiler_info.compiler_root, compiler_info.vcvarsall_path, vc_option))
					build_cmd.AddCommand('@CD /d "%s"' % build_dir)
				for target in project_list:
					build_info.MakeAddBuildCommand(build_cmd, make_name, target)
				if build_cmd.Execute() != 0:
					LogError("Build %s failed.\n" % name)

				os.chdir(curdir)

				print("")
				sys.stdout.flush()

if __name__ == "__main__":
	build_info = BuildInfo.FromArgv(sys.argv)

	additional_options = " -DKLAYGE_SHADER_PLATFORM_NAME:STRING=\"%s\"" % build_info.shader_platform_name
	if build_info.gles_include_dir != "auto":
		additional_options += " -DKLAYGE_GLES_INCLUDE_DIR:STRING=\"%s\"" % build_info.gles_include_dir
	if build_info.is_windows_desktop:
		if build_info.libovr_path != "auto":
			additional_options += " -DKLAYGE_LibOVR_PATH:STRING=\"%s\"" % build_info.libovr_path

	for compiler_info in build_info.compilers:
		if (not build_info.is_clean) and compiler_info.is_cross_compiling and (len(build_info.host_bin_dir) == 0):
			host_compiler = build_info.compiler_name
			if build_info.compiler_name == "vc":
				host_compiler += str(build_info.compiler_version)
			host_build_info = BuildInfo(build_info.project_type, host_compiler, (build_info.host_arch, ), build_info.cfg, "auto")
			BuildProjects("KlayGE", ".", host_build_info, host_build_info.compilers[0], ("KlayGE/Tools/FxmlJit/FxmlJit", "KlayGE/Tools/PlatformDeployer/PlatformDeployer"), additional_options)
			build_info.host_bin_dir = "%s/KlayGE/bin/%s_%s" % (os.path.abspath(os.curdir), host_build_info.host_platform, build_info.host_arch)
			break

	for compiler_info in build_info.compilers:
		BuildProjects("KlayGE", ".", build_info, compiler_info, ("ALL_BUILD", ), additional_options)

	if (len(sys.argv) > 1) and (sys.argv[1].lower() == "clean"):
		clean_dir_list = [ "7z", "android_native_app_glue", "assimp", "cxxopts", "d3dcompiler", "dxsdk", "filesystem", "fmt", "FreeImage", "freetype", "googletest", "GSL", "libogg", "libvorbis", "nanosvg", "openal-soft", "optional-lite", "Python", "python-cmake-buildsystem", "rapidjson", "rapidxml", "string-view-lite", "wpftoolkit", "zlib" ]
		for dir in clean_dir_list:
			dir_name = "External/" + dir
			if os.path.isdir(dir_name):
				shutil.rmtree(dir_name)
