#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_Boost(build_info, compiler_arch):
	b2_name = ""
	os.chdir("External/boost")
	if "win" == build_info.host_platform:
		b2_name = "b2.exe"
		if not os.path.exists(b2_name):
			os.system("bootstrap.bat")
	elif "linux" == build_info.host_platform:
		b2_name = "./b2"
		if not os.path.exists(b2_name):
			os.system("./bootstrap.sh")

	if "vc" == build_info.compiler_name:
		boost_toolset = "ms%s-%d.0" % (build_info.compiler_name, (build_info.compiler_version / 10))
	else:
		if "android" == build_info.target_platform:
			if "armeabi-v7a" == compiler_arch[0]:
				boost_toolset = "gcc-android_armeabi_v7a"
			else:
				boost_toolset = "gcc-android_%s" % compiler_arch[0]
		else:
			if "clang" == build_info.compiler_name:
				boost_toolset = "clang"
			else:
				boost_toolset = "gcc"
		
	options = ""
	
	if ("x86" == compiler_arch[0]) or ("x86_app" == compiler_arch[0]):
		options += "address-model=32"
	elif ("x64" == compiler_arch[0]) or ("x64_app" == compiler_arch[0]):
		options += "address-model=64"
	
	if "vc" == build_info.compiler_name:
		if build_info.compiler_version >= 100:
			options += " --without-regex"
		if build_info.compiler_version >= 110:
			options += " --without-atomic --without-chrono --without-date_time --without-filesystem --without-system --without-thread"
	else:
		options += " --without-atomic --without-date_time"

	options += " --disable-filesystem2"
	if ("x86_app" == compiler_arch[0]) or ("x64_app" == compiler_arch[0]):
		options += " architecture=x86 --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	elif ("arm_app" == compiler_arch[0]):
		options += " architecture=arm --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	if "vc" == build_info.compiler_name:
		options += " cxxflags=-wd4819 cxxflags=-wd4910 define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE"
	elif ("mgw" == build_info.compiler_name) or ("gcc" == build_info.compiler_name):
		if (build_info.compiler_version < 47):
			options += " cxxflags=-std=c++0x linkflags=-std=c++0x"
		else:
			options += " cxxflags=-std=c++11 linkflags=-std=c++11"
		if ("x64" == compiler_arch[0]):
			options += " define=BOOST_USE_WINDOWS_H"
	elif ("clang" == build_info.compiler_name):
		import subprocess

		clang_path = subprocess.check_output(["where", "clang"])
		clang_path = clang_path.replace("\\", "/")
		clang_path = clang_path[0 : clang_path.rfind("/")]
		gcc_ver = subprocess.check_output(["gcc", "-dumpversion"])
		
		if os.path.exists("%s/../lib/gcc/i686-w64-mingw32/%s/include/c++/" % (clang_path, gcc_ver)):
			mingw_name = "i686-w64-mingw32"
			mingw_in_lib_folder = True
		elif os.path.exists("%s/../lib/gcc/x86_64-w64-mingw32/%s/include/c++/" % (clang_path, gcc_ver)):
			mingw_name = "x86_64-w64-mingw32"
			mingw_in_lib_folder = True
		elif os.path.exists("%s/../lib/gcc/mingw32/%s/include/c++/" % (clang_path, gcc_ver)):
			mingw_name = "mingw32"
			mingw_in_lib_folder = True
		elif os.path.exists("%s/../i686-w64-mingw32/include/c++/" % clang_path):
			mingw_name = "i686-w64-mingw32"
			mingw_in_lib_folder = False
		elif os.path.exists("%s/../x86_64-w64-mingw32/include/c++/" % clang_path):
			mingw_name = "x86_64-w64-mingw32"
			mingw_in_lib_folder = False
		elif os.path.exists("%s/../mingw32/include/c++/" % clang_path):
			mingw_name = "mingw32"
			mingw_in_lib_folder = False

		if mingw_in_lib_folder:
			mingw_cxx_include = "%s/../lib/gcc/%s/%s/include/c++/" % (clang_path, mingw_name, gcc_ver)
		else:
			mingw_cxx_include = "%s/../%s/include/c++/" % (clang_path, mingw_name)
		
		options += " define=BOOST_USE_WINDOWS_H cxxflags=\"-std=c++11 -I\"%s\" -I\"%s%s/\"\" linkflags=-std=c++11" % (mingw_cxx_include, mingw_cxx_include, mingw_name)

	if "android" == build_info.target_platform:
		options += " cxxflags=%%CXXFLAGS%% threadapi=pthread target-os=linux --user-config=./user-config-android-%s.jam" % compiler_arch[0]
	if ("Debug" in build_info.cfg):
		options += " variant=debug"
	if ("Release" in build_info.cfg) or ("RelWithDebInfo" in build_info.cfg) or ("MinSizeRel" in build_info.cfg):
		options += " variant=release"
		
	build_cmd = batch_command(build_info.host_platform)
	if "android" == build_info.target_platform:
		build_cmd.add_command('set CXXFLAGS="-I%%ANDROID_NDK%%/platforms/android-9/arch-arm/usr/include -I%%ANDROID_NDK%%/sources/cxx-stl/gnu-libstdc++/%s/include -I%%ANDROID_NDK%%/sources/cxx-stl/gnu-libstdc++/%s/libs/armeabi/include"' % (compiler_arch[2], compiler_arch[2]))
	if build_info.prefer_static:
		link = "static"
	else:
		link = "shared"
	build_cmd.add_command('%s --toolset=%s --stagedir=./lib/%s_%s --builddir=./ --layout=versioned %s link=%s runtime-link=%s threading=multi stage' % (b2_name, boost_toolset, build_info.target_platform, compiler_arch[0], options, link, link))
	if build_cmd.execute() != 0:
		log_error("Build boost failed.")

	os.chdir("../../")

def build_Python(build_info, compiler_arch):
	additional_options = "-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_HK:BOOL=\"ON\" -D BUILTIN_CODECS_ISO2022:BOOL=\"ON\" \
		-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_JP:BOOL=\"ON\" -D BUILTIN_CODECS_KR:BOOL=\"ON\" -D BUILTIN_CODECS_TW:BOOL=\"ON\" \
		-D BUILTIN_COLLECTIONS:BOOL=\"ON\" -D BUILTIN_FUNCTOOLS:BOOL=\"ON\" -D BUILTIN_IO:BOOL=\"ON\" -D BUILTIN_ITERTOOLS:BOOL=\"ON\" \
		-D BUILTIN_LOCALE:BOOL=\"ON\" -D BUILTIN_MATH:BOOL=\"ON\" -D BUILTIN_MSI:BOOL=\"OFF\" -D BUILTIN_MULTIBYTECODEC:BOOL=\"ON\" \
		-D BUILTIN_OPERATOR:BOOL=\"ON\" -D BUILTIN_UNICODEDATA:BOOL=\"ON\" \
		-D ENABLE_AUDIOOP:BOOL=\"OFF\" -D ENABLE_OSSAUDIODEV:BOOL=\"OFF\" \
		-D USE_SYSTEM_Curses:BOOL=\"OFF\" -D USE_SYSTEM_EXPAT:BOOL=\"OFF\" -D USE_SYSTEM_DB:BOOL=\"OFF\" -D USE_SYSTEM_GDBM:BOOL=\"OFF\" \
		-D USE_SYSTEM_OpenSSL:BOOL=\"OFF\" -D USE_SYSTEM_READLINE:BOOL=\"OFF\" -D USE_SYSTEM_SQLITE3:BOOL=\"OFF\" -D USE_SYSTEM_TCL:BOOL=\"OFF\" \
		-D USE_SYSTEM_ZLIB:BOOL=\"OFF\""
	build_a_project("Python", "External/Python", build_info, compiler_arch, False, additional_options)

def build_libogg(build_info, compiler_arch):
	build_a_project("libogg", "External/libogg", build_info, compiler_arch)

def build_libvorbis(build_info, compiler_arch):
	build_a_project("libvorbis", "External/libvorbis", build_info, compiler_arch)

def build_freetype(build_info, compiler_arch):
	build_a_project("freetype", "External/freetype", build_info, compiler_arch)

def build_7z(build_info, compiler_arch):
	build_a_project("7z", "External/7z", build_info, compiler_arch)

def setup_DXSDK(build_info, compiler_arch):
	build_a_project("DXSDK", "External/DXSDK", build_info, compiler_arch)

def setup_OpenALSDK(build_info, compiler_arch):
	build_a_project("OpenALSDK", "External/OpenALSDK", build_info, compiler_arch)

def setup_Cg(build_info, compiler_arch):
	build_a_project("Cg", "External/Cg", build_info, compiler_arch)

def build_external_libs(build_info):
	import glob

	if "win" == build_info.host_platform:
		bat_suffix = "bat"
	elif "linux" == build_info.host_platform:
		bat_suffix = "sh"
	if "win" == build_info.target_platform:
		dll_suffix = "dll"
	elif "linux" == build_info.target_platform:
		dll_suffix = "so"

	for arch in build_info.arch_list:
		platform_dir = "%s_%s" % (build_info.target_platform, arch[0])
		dst_dir = "KlayGE/bin/%s/" % platform_dir

		if not arch[3]:
			print("\nBuilding boost...\n")
			build_Boost(build_info, arch)

			if not build_info.prefer_static:
				for fname in glob.iglob("External/boost/lib/%s_%s/lib/*.%s" % (build_info.target_platform, arch[0], dll_suffix)):
					copy_to_dst(fname, dst_dir)

		if not arch[3]:
			print("\nBuilding Python...\n")
			build_Python(build_info, arch)

			if not build_info.prefer_static:
				if not os.path.exists("%sLib" % dst_dir):
					os.mkdir("%sLib" % dst_dir)
				for fname in glob.iglob("External/Python/Lib/*.py"):
					copy_to_dst(fname, "%sLib/" % dst_dir)
				if not os.path.exists("%sLib/encodings" % dst_dir):
					os.mkdir("%sLib/encodings" % dst_dir)
				for fname in glob.iglob("External/Python/Lib/encodings/*.py"):
					copy_to_dst(fname, "%sLib/encodings/" % dst_dir)

		if not arch[3]:
			print("\nBuilding libogg...\n")
			build_libogg(build_info, arch)

		if not arch[3]:
			print("\nBuilding libvorbis...\n")
			build_libvorbis(build_info, arch)

		if (not arch[3]) and (build_info.target_platform != "android"):
			print("\nBuilding freetype...\n")
			build_freetype(build_info, arch)

		print("\nBuilding 7z...\n")
		build_7z(build_info, arch)

		if not build_info.prefer_static:
			for fname in glob.iglob("External/7z/lib/%s/*.%s" % (platform_dir, dll_suffix)):
				copy_to_dst(fname, dst_dir)

		if (not arch[3]) and (build_info.target_platform != "android"):
			if "win" == build_info.target_platform:
				print("\nSeting up DXSDK...\n")
				setup_DXSDK(build_info, arch)

		if (not arch[3]) and (build_info.target_platform != "android"):
			if "win" == build_info.target_platform:
				print("\nSeting up OpenAL SDK...\n")
				setup_OpenALSDK(build_info, arch)

		if (not arch[3]) and (build_info.target_platform != "android"):
			print("\nSeting up Cg...\n")
			setup_Cg(build_info, arch)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(bi)
