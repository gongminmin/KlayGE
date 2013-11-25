#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_Boost(compiler_info, compiler_arch):
	b2_name = ""
	os.chdir("External/boost")
	if "win" == compiler_info.platform:
		b2_name = "b2.exe"
		if not os.path.exists(b2_name):
			os.system("bootstrap.bat")
	elif "linux" == compiler_info.platform:
		b2_name = "./b2"
		if not os.path.exists(b2_name):
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
		
	build_cmd = batch_command()
	build_cmd.add_command('%s --toolset=%s --stagedir=./lib/%s_%s --builddir=./ --layout=versioned address-model=%d %s %s link=shared runtime-link=shared threading=multi stage' % (b2_name, boost_toolset, compiler_info.platform, compiler_arch[0], address_model, config, options))
	if build_cmd.execute() != 0:
		log_error("Build boost failed.")

	os.chdir("../../")

def build_Python(compiler_info, compiler_arch):
	additional_options = "-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_HK:BOOL=\"ON\" -D BUILTIN_CODECS_ISO2022:BOOL=\"ON\" -D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_JP:BOOL=\"ON\" -D BUILTIN_CODECS_KR:BOOL=\"ON\" -D BUILTIN_CODECS_TW:BOOL=\"ON\" -D BUILTIN_COLLECTIONS:BOOL=\"ON\" -D BUILTIN_FUNCTOOLS:BOOL=\"ON\" -D BUILTIN_IO:BOOL=\"ON\" -D BUILTIN_ITERTOOLS:BOOL=\"ON\" -D BUILTIN_LOCALE:BOOL=\"ON\" -D BUILTIN_MATH:BOOL=\"ON\" -D BUILTIN_MSI:BOOL=\"OFF\" -D BUILTIN_MULTIBYTECODEC:BOOL=\"ON\" -D BUILTIN_OPERATOR:BOOL=\"ON\" -D BUILTIN_UNICODEDATA:BOOL=\"ON\""
	build_a_project("Python", "External/Python", compiler_info, compiler_arch, False, additional_options)

def build_libogg(compiler_info, compiler_arch):
	build_a_project("libogg", "External/libogg", compiler_info, compiler_arch)

def build_libvorbis(compiler_info, compiler_arch):
	build_a_project("libvorbis", "External/libvorbis", compiler_info, compiler_arch)

def build_freetype(compiler_info, compiler_arch):
	build_a_project("freetype", "External/freetype", compiler_info, compiler_arch)

def build_7z(compiler_info, compiler_arch):
	build_a_project("7z", "External/7z", compiler_info, compiler_arch)

def build_external_libs(compiler_info):
	import glob

	if "win" == compiler_info.platform:
		bat_suffix = "bat"
		dll_suffix = "dll"
	elif "linux" == compiler_info.platform:
		bat_suffix = "sh"
		dll_suffix = "so"

	for arch in compiler_info.arch_list:
		platform_dir = "%s_%s" % (compiler_info.platform, arch[0])
		dst_dir = "KlayGE/bin/%s/" % platform_dir

		if not arch[3]:
			print("\nBuilding boost...\n")
			build_Boost(compiler_info, arch)

			for fname in glob.iglob("External/boost/lib/%s_%s/lib/*.%s" % (compiler_info.platform, arch[0], dll_suffix)):
				copy_to_dst(fname, dst_dir)

		if not arch[3]:
			print("\nBuilding Python...\n")
			build_Python(compiler_info, arch)

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
			build_libogg(compiler_info, arch)

		if not arch[3]:
			print("\nBuilding libvorbis...\n")
			build_libvorbis(compiler_info, arch)

		if not arch[3]:
			print("\nBuilding freetype...\n")
			build_freetype(compiler_info, arch)

		print("\nBuilding 7z...\n")
		build_7z(compiler_info, arch)

		for fname in glob.iglob("External/7z/lib/%s/*.%s" % (platform_dir, dll_suffix)):
			copy_to_dst(fname, dst_dir)

		if not arch[3]:
			if "win" == compiler_info.platform:
				print("\nSeting up DXSDK...\n")

				copy_to_dst("External/DXSDK/Redist/%s/d3dcompiler_46.%s" % (arch[0], dll_suffix), dst_dir)

		if not arch[3]:
			if "win" == compiler_info.platform:
				print("\nSeting up OpenAL SDK...\n")

				copy_to_dst("External/OpenALSDK/redist/%s/OpenAL32.%s" % (arch[0], dll_suffix), dst_dir)

		if not arch[3]:
			print("\nSeting up Cg...\n")

			if "x64" == arch[0]:
				subdir = ".x64"
			else:
				subdir = ""
			copy_to_dst("External/Cg/bin%s/cg.%s" % (subdir, dll_suffix), dst_dir)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	ci = compiler_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(ci)
