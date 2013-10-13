#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def copy_to_dst(src_name, dst_dir):
	print("Copy %s to %s" % (src_name, dst_dir))
	import shutil
	shutil.copy(src_name, dst_dir)

def build_Boost(compiler_info, compiler_arch):
	bjam_name = ""
	os.chdir("External/boost")
	if "win32" == compiler_info.platform:
		bjam_name = "bjam.exe"
		if not os.path.exists(bjam_name):
			os.system("bootstrap.bat")
	elif "linux" == compiler_info.platform:
		bjam_name = "./bjam"
		if not os.path.exists(bjam_name):
			os.system("./bootstrap.sh")

	if "vc" == compiler_info.name:
		boost_toolset = "ms%s-%d.0" % (compiler_info.name, compiler_info.version)
	else:
		boost_toolset = "gcc"
		
	address_model = 32
	if "x64" == compiler_arch[0]:
		address_model = 64
	
	options = ""
	if "vc" == compiler_info.name:
		if compiler_info.version >= 10:
			options += " --without-regex"
		if compiler_info.version >= 11:
			options += " --without-atomic --without-chrono --without-date_time --without-filesystem --without-system --without-thread"
	else:
		options += " --without-atomic --without-date_time --without-regex"

	if ("x86" == compiler_arch[0]) or ("x64" == compiler_arch[0]):
		options += " --disable-filesystem2"
	elif ("x86_app" == compiler_arch[0]):
		options += " architecture=x86 --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	elif ("arm_app" == compiler_arch[0]):
		options += " architecture=arm --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	if "vc" == compiler_info.name:
		options += " cxxflags=-wd4819 cxxflags=-wd4910 define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE"
	else:
		if ("x64" == compiler_arch[0]):
			options += " define=BOOST_USE_WINDOWS_H"

	config = ""
	if ("Debug" in compiler_info.cfg):
		config += " variant=debug"
	if ("Release" in compiler_info.cfg) or ("RelWithDebInfo" in compiler_info.cfg) or ("MinSizeRel" in compiler_info.cfg):
		config += " variant=release"
		
	if "vc" == compiler_info.name:
		compiler_version_str = str(compiler_info.version)
	else:
		compiler_version_str = ""

	build_cmd = batch_command()
	build_cmd.add_command('%s --toolset=%s --stagedir=./lib_%s%s_%s --builddir=./ --layout=versioned address-model=%d %s %s link=shared runtime-link=shared threading=multi stage' % (bjam_name, boost_toolset, compiler_info.name, compiler_version_str, compiler_arch[0], address_model, config, options))
	build_cmd.execute()

	os.chdir("../../")

def build_Python(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/Python/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "External/Python/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = "-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_HK:BOOL=\"ON\" -D BUILTIN_CODECS_ISO2022:BOOL=\"ON\" -D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_JP:BOOL=\"ON\" -D BUILTIN_CODECS_KR:BOOL=\"ON\" -D BUILTIN_CODECS_TW:BOOL=\"ON\" -D BUILTIN_COLLECTIONS:BOOL=\"ON\" -D BUILTIN_FUNCTOOLS:BOOL=\"ON\" -D BUILTIN_IO:BOOL=\"ON\" -D BUILTIN_ITERTOOLS:BOOL=\"ON\" -D BUILTIN_LOCALE:BOOL=\"ON\" -D BUILTIN_MATH:BOOL=\"ON\" -D BUILTIN_MSI:BOOL=\"OFF\" -D BUILTIN_MULTIBYTECODEC:BOOL=\"ON\" -D BUILTIN_OPERATOR:BOOL=\"ON\" -D BUILTIN_UNICODEDATA:BOOL=\"ON\""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""
	if compiler_info.name != "vc":
		additional_options += "-D KLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, ".."))
	cmake_cmd.execute()

	arch_name = compiler_arch[0]
	if ("x86_app" == arch_name):
		arch_name = "x86"
	elif ("arm_app" == arch_name):
		arch_name = "x86_arm"
	elif ("x64" == arch_name):
		arch_name = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, arch_name))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "Python32", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)

def build_libogg(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/libogg/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "External/libogg/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = ""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""
	if compiler_info.name != "vc":
		additional_options += "-D KLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	arch_name = compiler_arch[0]
	if ("x86_app" == arch_name):
		arch_name = "x86"
	elif ("arm_app" == arch_name):
		arch_name = "x86_arm"
	elif ("x64" == arch_name):
		arch_name = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, arch_name))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "libogg", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)

def build_libvorbis(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/libvorbis/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "External/libvorbis/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = ""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""
	if compiler_info.name != "vc":
		additional_options += "-D KLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	arch_name = compiler_arch[0]
	if ("x86_app" == arch_name):
		arch_name = "x86"
	elif ("arm_app" == arch_name):
		arch_name = "x86_arm"
	elif ("x64" == arch_name):
		arch_name = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, arch_name))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "vorbis", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)

def build_freetype(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/freetype/builds/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "External/freetype/builds/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = ""
	if compiler_info.name != "vc":
		additional_options += "-D KLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	arch_name = compiler_arch[0]
	if ("x86_app" == arch_name):
		arch_name = "x86"
	elif ("arm_app" == arch_name):
		arch_name = "x86_arm"
	elif ("x64" == arch_name):
		arch_name = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, arch_name))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "freetype", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)

def build_7z(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/7z/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "External/7z/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = ""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""
	if compiler_info.name != "vc":
		additional_options += "-D KLAYGE_ARCH_NAME:STRING=\"%s\"" % compiler_arch[0]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (compiler_arch[1], toolset_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	arch_name = compiler_arch[0]
	if ("x86_app" == arch_name):
		arch_name = "x86"
	elif ("arm_app" == arch_name):
		arch_name = "x86_arm"
	elif ("x64" == arch_name):
		arch_name = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, arch_name))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "7z", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)
			
def build_external_libs(compiler_info, compiler_arch):
	import glob

	if "win32" == compiler_info.platform:
		platform_dir = "win_%s" % compiler_arch[0]
		dst_dir = "KlayGE/bin/%s/" % platform_dir
		bat_suffix = "bat"
		dll_suffix = "dll"
	elif "linux" == compiler_info.platform:
		platform_dir = "linux_%s" % compiler_arch[0]
		dst_dir = "KlayGE/bin/%s/" % platform_dir
		bat_suffix = "sh"
		dll_suffix = "so"


	if not compiler_arch[3]:
		print("\nBuilding boost...\n")
		build_Boost(compiler_info, compiler_arch)

		if "vc" == compiler_info.name:
			compiler_version_str = str(compiler_info.version)
		else:
			compiler_version_str = ""
		for fname in glob.iglob("External/boost/lib_%s%s_%s/lib/*.%s" % (compiler_info.name, compiler_version_str, compiler_arch[0], dll_suffix)):
			copy_to_dst(fname, dst_dir)

	if not compiler_arch[3]:
		print("\nBuilding Python...\n")
		build_Python(compiler_info, compiler_arch)

		if not os.path.exists("%sLib" % dst_dir):
			os.mkdir("%sLib" % dst_dir)
		for fname in glob.iglob("External/Python/Lib/*.py"):
			copy_to_dst(fname, "%sLib/" % dst_dir)
		if not os.path.exists("%sLib/encodings" % dst_dir):
			os.mkdir("%sLib/encodings" % dst_dir)
		for fname in glob.iglob("External/Python/Lib/encodings/*.py"):
			copy_to_dst(fname, "%sLib/encodings/" % dst_dir)

	if not compiler_arch[3]:
		print("\nBuilding libogg...\n")
		build_libogg(compiler_info, compiler_arch)

	if not compiler_arch[3]:
		print("\nBuilding libvorbis...\n")
		build_libvorbis(compiler_info, compiler_arch)

	if not compiler_arch[3]:
		print("\nBuilding freetype...\n")
		build_freetype(compiler_info, compiler_arch)

	if ("vc" == compiler_info.name):
		print("\nBuilding 7z...\n")
		build_7z(compiler_info, compiler_arch)

		for fname in glob.iglob("External/7z/lib/%s/7zxa*.%s" % (platform_dir, dll_suffix)):
			copy_to_dst(fname, dst_dir)
		for fname in glob.iglob("External/7z/lib/%s/LZMA*.%s" % (platform_dir, dll_suffix)):
			copy_to_dst(fname, dst_dir)

	if not compiler_arch[3]:
		if "win32" == compiler_info.platform:
			print("\nSeting up DXSDK...\n")

			copy_to_dst("External/DXSDK/Redist/%s/d3dcompiler_46.%s" % (compiler_arch[0], dll_suffix), dst_dir)

	if not compiler_arch[3]:
		if "win32" == compiler_info.platform:
			print("\nSeting up OpenAL SDK...\n")

			copy_to_dst("External/OpenALSDK/redist/%s/OpenAL32.%s" % (compiler_arch[0], dll_suffix), dst_dir)

	if not compiler_arch[3]:
		print("\nSeting up Cg...\n")

		if "x64" == compiler_arch[0]:
			subdir = ".x64"
		else:
			subdir = ""
		copy_to_dst("External/Cg/bin%s/cg.%s" % (subdir, dll_suffix), dst_dir)

if __name__ == "__main__":
	if len(sys.argv) > 1:
		compiler = sys.argv[1]
	else:
		compiler = ""
	if len(sys.argv) > 2:
		arch = (sys.argv[2], )
	else:
		arch = ""
	if len(sys.argv) > 3:
		cfg = sys.argv[3]
	else:
		cfg = ""

	ci = compiler_info(compiler, arch, cfg)

	if 0 == len(ci.name):
		print("Wrong configuration\n")
		sys.exit(1)

	for arch in ci.arch_list:
		build_external_libs(ci, arch)
