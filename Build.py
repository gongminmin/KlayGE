#!/usr/bin/env python
#-*- coding: ascii -*-

import os, sys, multiprocessing, subprocess, shutil, platform
from pathlib import Path, PosixPath

class CfgBuildDefault:
	def __init__(self):
		#################################################
		# !!!! DO NOT DELETE ANY FIELD OF THIS CLASS !!!!
		#################################################

		# The path of cmake executable. Could fill in the path, or "auto".
		self.cmake_path = "auto"

		# Project type.
		#   On Windows desktop, could be "vs2022", "vs2019", "make", "ninja", "auto".
		#   On Windows store, could be "vs2019", "auto".
		#   On Android, could be "make", "auto".
		#   On Linux, could be "make", "ninja", "auto".
		#   On macOS, could be "xcode", "ninja", "auto".
		#   On iOS, could be "xcode", "ninja", "auto".
		self.project = "auto"

		# Compiler name.
		#   On Windows desktop, could be "vc143", "vc142", "mingw", "clangcl", "auto".
		#   On Windows store, could be "vc142", "auto".
		#   On Android, could be "clang", "auto".
		#   On Linux, could be "gcc", "auto".
		#   On macOS, could be "clang", "auto".
		#   On iOS, could be "clang", "auto".
		self.compiler = "auto"

		# Target CPU architecture.
		#   On Windows desktop, could be "arm64", "x64".
		#   On Windows store, could be "arm64", "x64".
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
	print(f"[E] {message}")
	sys.stdout.flush()
	if sys.platform.startswith("win"):
		pause_cmd = "pause"
	else:
		pause_cmd = "read"
	subprocess.call(pause_cmd, shell = True)
	sys.exit(1)

def LogInfo(message):
	print(f"[I] {message}")
	sys.stdout.flush()

def LogWarning(message):
	print(f"[W] {message}")
	sys.stdout.flush()

class CompilerInfo:
	def __init__(self, build_info, arch, gen_name, compiler_root, vcvarsall_options = ""):
		self.arch = arch
		self.generator = gen_name
		self.compiler_root = compiler_root
		self.vcvarsall_options = vcvarsall_options

		self.is_cross_compiling = build_info.is_cross_compiling
		if build_info.host_arch != arch:
			self.is_cross_compiling = True

class BuildInfo:
	@classmethod
	def FromArgv(BuildInfo, argv, base = 0):
		project = None
		compiler = None
		archs = None
		cfg = None
		target = None

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
		if project is not None:
			cfg_build.project = project
		else:
			try:
				cfg_build.project
			except:
				cfg_build.project = "auto"
		if (compiler is not None) and (compiler.lower() != "clean"):
			cfg_build.compiler = compiler
		else:
			try:
				cfg_build.compiler
			except:
				cfg_build.compiler = "auto"
		if archs is not None:
			cfg_build.arch = archs
		else:
			try:
				cfg_build.arch
			except:
				cfg_build.arch = ("x64", )
		if cfg is not None:
			cfg_build.config = cfg
		else:
			try:
				cfg_build.config
			except:
				cfg_build.config = ("Debug", "RelWithDebInfo")
		if target is not None:
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
			LogError(f"Unknown host architecture {self.host_arch}.\n")

		if self.host_platform == "win":
			self.sep = "\r\n"
		else:
			self.sep = "\n"

		if cfg_build.cmake_path == "auto":
			self.cmake_path = self.FindCMake()
		else:
			self.cmake_path = cfg_build.cmake_path
		self.cmake_ver = self.RetrieveCMakeVersion()

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
			project_type = None
		if (compiler is None) or self.is_clean:
			compiler = None
			if ("auto" == cfg_build.project) and ("auto" == cfg_build.compiler):
				if self.is_windows:
					program_files_folder = self.FindProgramFilesFolder()

					if self.FindVS2022Folder(program_files_folder) is not None:
						project_type = "vs2022"
						compiler = "vc143"
					elif self.FindVS2019Folder(program_files_folder) is not None:
						project_type = "vs2019"
						compiler = "vc142"
					elif self.FindClang(False) is not None:
						project_type = "make"
						compiler = "clang"
					elif self.FindGCC(False) is not None:
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
					LogError(f"Unsupported target platform {target_platform}.\n")
			else:
				if cfg_build.project != "auto":
					project_type = cfg_build.project
				if cfg_build.compiler != "auto":
					compiler = cfg_build.compiler

		if (project_type is not None) and (compiler is None):
			if project_type == "vs2022":
				compiler = "vc143"
			elif project_type == "vs2019":
				compiler = "vc142"
			elif project_type == "xcode":
				compiler = "clang"

		if self.is_windows:
			program_files_folder = self.FindProgramFilesFolder()

			if "vc143" == compiler:
				if project_type == "vs2022":
					try_folder = self.FindVS2022Folder(program_files_folder)
					if try_folder is not None:
						compiler_root = try_folder
						vcvarsall_options = ""
					else:
						LogError("Could NOT find vc143 compiler toolset for VS2022.\n")
				else:
					LogError("Could NOT find vc143 compiler.\n")
			elif "vc142" == compiler:
				if project_type == "vs2022":
					try_folder = self.FindVS2022Folder(program_files_folder)
					if try_folder is not None:
						compiler_root = try_folder
						vcvarsall_options = "-vcvars_ver=14.2"
					else:
						LogError("Could NOT find vc142 compiler toolset for VS2022.\n")
				elif project_type == "vs2019":
					try_folder = self.FindVS2019Folder(program_files_folder)
					if try_folder is not None:
						compiler_root = try_folder
						vcvarsall_options = ""
					else:
						LogError("Could NOT find vc142 compiler toolset for VS2019.\n")
				else:
					LogError("Could NOT find vc142 compiler.\n")
			elif "clangcl" == compiler:
				if project_type == "vs2022":
					try_folder = self.FindVS2022Folder(program_files_folder)
					if try_folder is not None:
						compiler_root = try_folder
						vcvarsall_options = ""
					else:
						LogError("Could NOT find clang-cl compiler toolset for VS2022.\n")
				elif project_type == "vs2019":
					try_folder = self.FindVS2019Folder(program_files_folder)
					if try_folder is not None:
						compiler_root = try_folder
						vcvarsall_options = ""
					else:
						LogError("Could NOT find clang-cl compiler toolset for VS2019.\n")
				else:
					LogError("Could NOT find clang-cl compiler.\n")
			elif "clang" == compiler:
				clang_loc = self.FindClang(True)
				compiler_root = clang_loc.parent
			elif "mingw" == compiler:
				gcc_loc = self.FindGCC(True)
				compiler_root = gcc_loc.parent
		else:
			compiler_root = Path("")

		if project_type is None:
			if "vc143" == compiler:
				project_type = "vs2022"
			elif "vc142" == compiler:
				project_type = "vs2019"
			elif ("clang" == compiler) and (self.is_darwin or self.is_ios):
				project_type = "xcode"
			else:
				project_type = "make"

		if archs is None:
			archs = cfg_build.arch
			if archs is None:
				archs = ("x64", )

		if cfg is None:
			cfg = cfg_build.config
			if cfg is None:
				cfg = ("Debug", "RelWithDebInfo")

		multi_config = False
		compilers = []
		if "vs2022" == project_type:
			self.vs_version = 17
			if "vc143" == compiler:
				compiler_name = "vc"
				compiler_version = 143
			elif "vc142" == compiler:
				compiler_name = "vc"
				compiler_version = 142
			elif "clangcl" == compiler:
				compiler_name = "clangcl"
				compiler_version = self.RetrieveClangVersion(compiler_root.joinpath("../../Tools/Llvm/bin/"))
			else:
				LogError(f"Wrong combination of project {project_type} and compiler {compiler}.\n")
			multi_config = True
			for arch in archs:
				compilers.append(CompilerInfo(self, arch, "Visual Studio 17", compiler_root, vcvarsall_options))
		elif "vs2019" == project_type:
			self.vs_version = 16
			if "vc142" == compiler:
				compiler_name = "vc"
				compiler_version = 142
			elif "clangcl" == compiler:
				compiler_name = "clangcl"
				compiler_version = self.RetrieveClangVersion(compiler_root.joinpath("../../Tools/Llvm/bin/"))
			else:
				LogError(f"Wrong combination of project {project_type} and compiler {compiler}.\n")
			multi_config = True
			for arch in archs:
				compilers.append(CompilerInfo(self, arch, "Visual Studio 16", compiler_root, vcvarsall_options))
		elif "xcode" == project_type:
			if "clang" == compiler:
				compiler_name = "clang"
				compiler_version = self.RetrieveClangVersion()
				gen_name = "Xcode"
				multi_config = True
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root))
			else:
				LogError(f"Wrong combination of project {project_type} and compiler {compiler}.\n")
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
			elif "vc143" == compiler:
				compiler_name = "vc"
				compiler_version = 143
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root, vcvarsall_options))
			elif "vc142" == compiler:
				compiler_name = "vc"
				compiler_version = 142
				for arch in archs:
					compilers.append(CompilerInfo(self, arch, gen_name, compiler_root, vcvarsall_options))
			else:
				LogError(f"Wrong combination of project {project_type} and compiler {compiler}.\n")
		else:
			compiler_name = ""
			compiler_version = 0
			LogError("Unsupported project type {project_type}.\n")

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

	def CMakeAddBuildCommand(self, batch_cmd, target_list, config):
		make_options = f"-j{self.jobs}"
		if target_list[0] != "ALL_BUILD":
			make_options += " -t "
			for target in target_list:
				make_options += f" {target}"
		if config != None:
			make_options += f" --config {config}"
		if "win" == self.host_platform:
			batch_cmd.AddCommand(f"@cmake --build . {make_options}")
			batch_cmd.AddCommand("@if ERRORLEVEL 1 exit /B 1")
		else:
			batch_cmd.AddCommand(f"cmake --build . {make_options}")
			batch_cmd.AddCommand("if [ $? -ne 0 ]; then exit 1; fi")

	def FindGCC(self, required):
		if self.host_platform != "win":
			env = os.environ
			if "CXX" in env:
				gcc_loc = env["CXX"]
				if len(gcc_loc) != 0:
					return Path(gcc_loc.split(self.sep)[0])

		gcc_loc = shutil.which("g++")
		if gcc_loc is None:
			if required:
				LogError("Could NOT find g++. Please install g++, set its path into CXX, or put its path into %%PATH%%.")
			else:
				return None
		else:
			return Path(gcc_loc.split(self.sep)[0])

	def RetrieveGCCVersion(self):
		gcc_ver = subprocess.check_output([self.FindGCC(True), "-dumpfullversion"]).decode()
		gcc_ver_components = gcc_ver.split(".")
		return int(gcc_ver_components[0] + gcc_ver_components[1])

	def FindClang(self, required):
		if self.host_platform != "win":
			env = os.environ
			if "CXX" in env:
				clang_loc = env["CXX"]
				if len(clang_loc) != 0:
					return Path(clang_loc.split(self.sep)[0])

		clang_loc = shutil.which("clang++")
		if clang_loc is None:
			if required:
				LogError("Could NOT find clang++. Please install clang++, set its path into CXX, or put its path into %%PATH%%.")
			else:
				return None
		else:
			return Path(clang_loc.split(self.sep)[0])

	def RetrieveClangVersion(self, path = None):
		if self.is_android:
			android_ndk_path = Path(os.environ["ANDROID_NDK"])
			prebuilt_llvm_path = android_ndk_path.joinpath("/toolchains/llvm/prebuilt/")
			prebuilt_clang_path = prebuilt_llvm_path.joinpath("windows/bin/")
			if not prebuilt_clang_path.is_dir():
				prebuilt_clang_path = prebuilt_llvm_path.joinpath("windows-x86_64/bin/")
			clang_path = prebuilt_clang_path.joinpath("clang")
		elif path is not None:
			clang_path = path.joinpath("clang")
		else:
			clang_path = self.FindClang(True)
		clang_ver = subprocess.check_output([str(clang_path), "--version"]).decode()
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
				program_files_folder = "C:/Program Files (x86)"
		else:
			if "ProgramFiles" in env:
				program_files_folder = env["ProgramFiles"]
			else:
				program_files_folder = "C:/Program Files"
		return Path(program_files_folder)

	def FindVS2022Folder(self, program_files_folder):
		return self.FindVS2017PlusFolder(program_files_folder, 17, "2022")

	def FindVS2019Folder(self, program_files_folder):
		return self.FindVS2017PlusFolder(program_files_folder, 16, "2019")

	def FindVS2017PlusFolder(self, program_files_folder, vs_version, vs_name):
		vcvarsall_name = "vcvarsall.bat"
		try_vswhere_location = program_files_folder.joinpath("Microsoft Visual Studio/Installer/vswhere.exe")
		if try_vswhere_location.exists():
			vs_location = Path(subprocess.check_output([str(try_vswhere_location),
				"-products", "*",
				"-latest",
				"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
				"-property", "installationPath",
				"-version", f"[{vs_version}.0,{vs_version + 1}.0)",
				"-prerelease"]).decode().split(self.sep)[0])
			try_folder = vs_location.joinpath("VC/Auxiliary/Build/")
			if try_folder.joinpath(vcvarsall_name).exists():
				return try_folder
		else:
			for name in ("Preview", vs_name):
				for sku in ("Community", "Professional", "Enterprise", "BuildTools"):
					try_folder = program_files_folder.joinpath(f"Microsoft Visual Studio/{name}/{sku}/VC/Auxiliary/Build/")
					if try_folder.joinpath(vcvarsall_name).exists():
						return try_folder
		return None

	def FindCMake(self):
		cmake_loc = shutil.which("cmake")
		if cmake_loc is None:
			LogError("Could NOT find CMake. Please install CMake 3.16+, set its path into CfgBuild's self.cmake_path, or put its path into %%PATH%%.")
		return Path(cmake_loc.split(self.sep)[0])

	def RetrieveCMakeVersion(self):
		cmake_ver = subprocess.check_output([str(self.cmake_path), "--version"]).decode()
		if len(cmake_ver) == 0:
			LogError("Could NOT find CMake. Please install CMake 3.16+, set its path into CfgBuild's self.cmake_path, or put its path into %%PATH%%.")
		cmake_ver = cmake_ver.split()[2]
		cmake_ver_components = cmake_ver.split('.')
		return int(cmake_ver_components[0] + cmake_ver_components[1])

	def DisplayInfo(self):
		print("Build information:")
		print(f"\tCMake path: {self.cmake_path}")
		print(f"\tCMake version: {self.cmake_ver}")
		print(f"\tHost platform: {self.host_platform}")
		print(f"\tTarget platform: {self.target_platform}")
		if self.is_android or self.is_windows_store:
			print(f"\tTarget API level: {self.target_api_level}")
		print(f"\tCPU count: {self.jobs}")
		print(f"\tPrefer static library: {self.prefer_static}")
		print(f"\tShader platform: {self.shader_platform_name}")
		print(f"\tIs dev platform: {self.is_dev_platform}")
		print(f"\tProject type: {self.project_type}")
		print(f"\tCompiler: {self.compiler_name}{self.compiler_version}")
		archs = ""
		for i in range(0, len(self.compilers)):
			archs += self.compilers[i].arch
			if i != len(self.compilers) - 1:
				archs += ", "
		print(f"\tArchitectures: {archs}")
		cfgs = ""
		for i in range(0, len(self.cfg)):
			cfgs += self.cfg[i]
			if i != len(self.cfg) - 1:
				cfgs += ", "
		print(f"\tConfigures: {cfgs}")
		print(f"\tGLES SDK include path: {self.gles_include_dir}")
		if self.is_windows_desktop:
			print(f"\tOculus LibOVR path: {self.libovr_path}")
		if len(self.host_bin_dir) > 0:
			print(f"\tHost binary dir: {self.host_bin_dir}")

		print("")
		sys.stdout.flush()

	def GetBuildDir(self, arch, config = None):
		env = os.environ
		if "BUILD_DIR" in env:
			build_dir_name = env["BUILD_DIR"]
		else:
			build_dir_name = f"{self.project_type}_{self.compiler_name}{self.compiler_version}_{self.target_platform}_{arch}"
			if not (config is None):
				build_dir_name += "-" + config
		return Path(build_dir_name)

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
		Path(batch_file).unlink()
		return ret_code

def BuildProjects(build_info, compiler_info, project_list, additional_options = ""):
	curdir = Path(os.curdir).absolute()

	toolset_name = ""
	if build_info.project_type.startswith("vs"):
		toolset_name = "-T"
		if build_info.compiler_name == "clangcl":
			toolset_name += " ClangCL"
		else:
			if not build_info.is_windows_store:
				toolset_name += f" v{build_info.compiler_version},"
			toolset_name += "host=x64"
	elif build_info.is_android:
		android_ndk_path = Path(os.environ["ANDROID_NDK"])
		toolset_name = "clang"

	if (build_info.compiler_name != "vc") or (build_info.project_type == "ninja"):
		additional_options += f' -DKLAYGE_ARCH_NAME:STRING="{compiler_info.arch}"'
	if build_info.is_android:
		additional_options += f' -DCMAKE_TOOLCHAIN_FILE="{PosixPath(android_ndk_path.joinpath("build/cmake/android.toolchain.cmake"))}"'
		additional_options += f" -DANDROID_PLATFORM=android-{build_info.target_api_level}"
		additional_options += " -DANDROID_STL=c++_static -DANDROID_TOOLCHAIN=clang"
	elif build_info.is_darwin:
		if "x64" == compiler_info.arch:
			additional_options += " -DCMAKE_OSX_ARCHITECTURES=x86_64"
		else:
			LogError("Unsupported Darwin architecture.\n")
	elif build_info.is_ios:
		additional_options += f' -DCMAKE_TOOLCHAIN_FILE="{PosixPath(curdir.joinpath("Build/CMake/Modules/iOS.cmake"))}'
		if "arm" == compiler_info.arch:
			additional_options += " -DIOS_PLATFORM=OS"
		elif "x86" == compiler_info.arch:
			additional_options += " -DIOS_PLATFORM=SIMULATOR"
		else:
			LogError("Unsupported iOS architecture.\n")

	if compiler_info.is_cross_compiling:
		additional_options += f' -DKLAYGE_HOST_BIN_DIR="{build_info.host_bin_dir}"'

	if build_info.project_type.startswith("vs"):
		vcvarsall_path = compiler_info.compiler_root.joinpath("vcvarsall.bat")
		if "x64" == compiler_info.arch:
			vc_option = "amd64"
			vc_arch = "x64"
		elif "arm64" == compiler_info.arch:
			vc_option = "amd64_arm64"
			vc_arch = "ARM64"
		else:
			LogError("Unsupported VS architecture.\n")
		if len(compiler_info.vcvarsall_options) > 0:
			vc_option += f" {compiler_info.vcvarsall_options}"

	if build_info.multi_config:
		if build_info.project_type.startswith("vs"):
			additional_options += f" -A {vc_arch}"

		if build_info.is_windows_store:
			additional_options += f' -DCMAKE_SYSTEM_NAME="WindowsStore" -DCMAKE_SYSTEM_VERSION={build_info.target_api_level}'

		build_dir = Path("./Build/").joinpath(build_info.GetBuildDir(compiler_info.arch))
		if build_info.is_clean:
			print("Cleaning...")
			sys.stdout.flush()

			if build_dir.is_dir():
				build_dir.rmdir();
		else:
			print("Building...")
			sys.stdout.flush()

			if not build_dir.exists():
				build_dir.mkdir()

			build_dir = build_dir.absolute()
			os.chdir(build_dir)

			cmake_cmd = BatchCommand(build_info.host_platform)
			if build_info.project_type.startswith("vs"):
				cmake_cmd.AddCommand(f'@CALL "{vcvarsall_path}" {vc_option}')
				cmake_cmd.AddCommand(f'@CD /d "{build_dir}"')
			cmake_cmd.AddCommand(f'"{build_info.cmake_path}" -G "{compiler_info.generator}" {toolset_name} {additional_options} ../../')
			if cmake_cmd.Execute() != 0:
				LogWarning("Config failed, retry 1...\n")
				if cmake_cmd.Execute() != 0:
					LogWarning("Config failed, retry 2...\n")
					if cmake_cmd.Execute() != 0:
						LogError("Config failed.\n")

			build_cmd = BatchCommand(build_info.host_platform)
			if build_info.project_type.startswith("vs"):
				build_cmd.AddCommand(f'@CALL "{vcvarsall_path}" {vc_option}')
				build_cmd.AddCommand(f'@CD /d "{build_dir}"')
			for config in build_info.cfg:
				build_info.CMakeAddBuildCommand(build_cmd, project_list, config)
			if build_cmd.Execute() != 0:
				LogError("Build failed.\n")

			os.chdir(curdir)

			print("")
			sys.stdout.flush()
	else:
		if build_info.project_type == "ninja":
			make_name = "ninja"
		else:
			if "win" == build_info.host_platform:
				if build_info.is_android:
					prebuilt_make_path = android_ndk_path.joinpath("prebuilt/windows")
					if not prebuilt_make_path.is_dir():
						prebuilt_make_path = android_ndk_path.joinpath("prebuilt/windows-x86_64")
					make_name = prebuilt_make_path.joinpath("bin/make.exe")
				else:
					make_name = "mingw32-make.exe"
			else:
				make_name = "make"

		additional_options_backup = additional_options

		for config in build_info.cfg:
			additional_options = additional_options_backup

			build_dir = Path("./Build/").joinpath(build_info.GetBuildDir(compiler_info.arch, config))
			if build_info.is_clean:
				print(f"Cleaning {config}...")
				sys.stdout.flush()

				if build_dir.is_dir():
					build_dir.rmdir()
			else:
				print(f"Building {config}...")
				sys.stdout.flush()

				if not build_dir.exists():
					build_dir.mkdir()
					if build_info.is_android:
						additional_options += f' -DCMAKE_MAKE_PROGRAM="{make_name}"'
					elif ("clang" == build_info.compiler_name):
						env = os.environ
						if not ("CC" in env):
							additional_options += " -DCMAKE_C_COMPILER=clang"
						if not ("CXX" in env):
							additional_options += " -DCMAKE_CXX_COMPILER=clang++"

				build_dir = build_dir.absolute()
				os.chdir(build_dir)

				additional_options += f' -DCMAKE_BUILD_TYPE:STRING="{config}"'
				if build_info.is_android:
					if "x86" == compiler_info.arch:
						abi_arch = "x86"
					elif "x86_64" == compiler_info.arch:
						abi_arch = "x86_64"
					elif "arm64-v8a" == compiler_info.arch:
						abi_arch = "arm64-v8a"
					else:
						LogError("Unsupported Android architecture.\n")
					additional_options += f' -DANDROID_ABI="{abi_arch}"'
					cpp_feature = "exceptions"
					if config == "Debug":
						cpp_feature += " rtti"
					additional_options += f' -DANDROID_CPP_FEATURES="{cpp_feature}"'

				cmake_cmd = BatchCommand(build_info.host_platform)
				if build_info.compiler_name == "vc":
					cmake_cmd.AddCommand(f'@CALL "{vcvarsall_path}" {vc_option}')
					cmake_cmd.AddCommand(f'@CD /d "{build_dir}"')
					additional_options += " -DCMAKE_C_COMPILER=cl.exe -DCMAKE_CXX_COMPILER=cl.exe"
				cmake_cmd.AddCommand(f'"{build_info.cmake_path}" -G "{compiler_info.generator}" {additional_options} ../../')
				if cmake_cmd.Execute() != 0:
					LogWarning(f"Config {config} failed, retry 1...\n")
					if cmake_cmd.Execute() != 0:
						LogWarning(f"Config {config} failed, retry 2...\n")
						if cmake_cmd.Execute() != 0:
							LogError(f"Config {config} failed.\n")

				build_cmd = BatchCommand(build_info.host_platform)
				if build_info.compiler_name == "vc":
					build_cmd.AddCommand(f'@CALL "{vcvarsall_path}" {vc_option}')
					build_cmd.AddCommand(f'@CD /d "{build_dir}"')
				build_info.CMakeAddBuildCommand(build_cmd, project_list, None)
				if build_cmd.Execute() != 0:
					LogError(f"Build {config} failed.\n")

				os.chdir(curdir)

				print("")
				sys.stdout.flush()

if __name__ == "__main__":
	build_info = BuildInfo.FromArgv(sys.argv)

	additional_options = f' -DKLAYGE_SHADER_PLATFORM_NAME:STRING="{build_info.shader_platform_name}"'
	if build_info.gles_include_dir != "auto":
		additional_options += f' -DKLAYGE_GLES_INCLUDE_DIR:STRING="{build_info.gles_include_dir}"'
	if build_info.is_windows_desktop:
		if build_info.libovr_path != "auto":
			additional_options += f' -DKLAYGE_LibOVR_PATH:STRING="{build_info.libovr_path}"'

	for compiler_info in build_info.compilers:
		if (not build_info.is_clean) and compiler_info.is_cross_compiling and (len(build_info.host_bin_dir) == 0):
			host_compiler = build_info.compiler_name
			if build_info.compiler_name == "vc":
				host_compiler += str(build_info.compiler_version)
			host_build_info = BuildInfo(build_info.project_type, host_compiler, (build_info.host_arch, ), build_info.cfg, "auto")
			BuildProjects(host_build_info, host_build_info.compilers[0], ("FxmlJit", "Cooker"), additional_options)
			build_info.host_bin_dir = Path(os.curdir).absolute().joinpath(f"KlayGE/bin/{host_build_info.host_platform}_{build_info.host_arch}")
			break

	for compiler_info in build_info.compilers:
		BuildProjects(build_info, compiler_info, ("ALL_BUILD", ), additional_options)

	if (len(sys.argv) > 1) and (sys.argv[1].lower() == "clean"):
		clean_dir_list = ["7z/7z", "7z/7zxa", "android_native_app_glue/android_native_app_glue", "assimp/assimp", "boost/assert", "boost/config", "boost/core", "boost/static_assert", "boost/throw_exception",
			"cxxopts/cxxopts", "d3dcompiler/d3dcompiler", "DirectX-Headers/DirectX-Headers", "dxsdk/dxsdk", "fmt/fmt", "FreeImage/FreeImage", "freetype/freetype", "googletest/googletest",
			"libogg/libogg", "libvorbis/libvorbis", "NanoRtti/NanoRtti", "nanosvg/nanosvg", "openal-soft/openal-soft", "Python/cpython", "Python/python-cmake-buildsystem", "rapidjson/rapidjson", "rapidxml/rapidxml",
			"scope-lite/scope-lite", "wpftoolkit/wpftoolkit", "zlib/zlib"]
		for dir in clean_dir_list:
			dir_name = Path("Externals/").joinpath(dir)
			if dir_name.is_dir():
				dir_name.rmdir()
