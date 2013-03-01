#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def copy_to_dst(src_name, dst_dir):
	print("Copy %s to %s" % (src_name, dst_dir))
	import shutil
	shutil.copy(src_name, dst_dir)

def build_Boost(compiler_name, compiler_version, compiler_arch, config_list, platform):
	os.chdir("External/boost")
	if "win32" == platform:
		if not os.path.exists("bjam.exe"):
			os.system("bootstrap.bat")
	elif "linux" == platform:
		if not os.path.exists("./bjam"):
			os.system("bootstrap.sh")

	if "vc" == compiler_name:
		boost_toolset = "ms%s-%d.0" % (compiler_name, compiler_version)
	else:
		boost_toolset = "gcc"
		
	address_model = 32
	if "x64" == compiler_arch:
		address_model = 64
	
	options = ""
	if "vc" == compiler_name:
		if compiler_version >= 10:
			options += " --without-regex"
		if compiler_version >= 11:
			options += " --without-atomic --without-chrono --without-date_time --without-filesystem --without-system --without-thread"
	else:
		options += " --without-atomic --without-date_time --without-regex"

	if ("x86" == compiler_arch) or ("x64" == compiler_arch):
		options += " --disable-filesystem2"
	elif ("x86_app" == compiler_arch):
		options += " architecture=x86 --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	elif ("arm_app" == compiler_arch):
		options += " architecture=arm --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	if "vc" == compiler_name:
		options += " cxxflags=-wd4819 cxxflags=-wd4910 define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE"

	config = ""
	if "Debug" in config_list:
		config += " debug"
	if ("Release" in config_list) or ("RelWithDebInfo" in config_list) or ("MinSizeRel" in config_list):
		config += " release"
		
	if "vc" == compiler_name:
		compiler_version_str = "%d" % compiler_version
	else:
		compiler_version_str = ""

	build_cmd = batch_command()
	build_cmd.add_command('bjam.exe --toolset=%s --stagedir=./lib_%s%s_%s --builddir=./ address-model=%d %s link=shared runtime-link=shared threading=multi stage %s' % (boost_toolset, compiler_name, compiler_version_str, compiler_arch, address_model, options, config))
	build_cmd.execute()

	os.chdir("../../")

def build_Python(compiler_name, compiler_version, compiler_arch, config_list, platform):
	if "win32" == platform:
		if ("vc" == compiler_name) and (compiler_version >= 11):
			os.chdir("External/Python/vc-11_0")
		else:
			os.chdir("External/Python/PCbuild")
		if (compiler_version >= 10):
			sln_suffix = "%d" % compiler_version
		else:
			sln_suffix = ""

		if "x64" == compiler_arch:
			arch = "x64"
			subdir = "amd64\\"
			compiler_arch = "x86_amd64"
		else:
			arch = "Win32"
			subdir = ""
		configs = []
		if "Debug" in config_list:
			configs.append("Debug")
		if ("Release" in config_list) or ("RelWithDebInfo" in config_list) or ("MinSizeRel" in config_list):
			configs.append("Release")

		build_cmd = batch_command()
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
		for cfg in configs:
			build_cmd.add_command('devenv pcbuild%s.sln /Build "%s|%s"' % (sln_suffix, cfg, arch))
			build_cmd.add_command('move /Y %s*.pyd ..\\DLLs\\%s' % (subdir, subdir))
			build_cmd.add_command('move /Y %s*.dll ..\\DLLs\\%s' % (subdir, subdir))
			build_cmd.add_command('move /Y %s*.lib ..\\libs\\%s' % (subdir, subdir))
			if "Debug" == cfg:
				suffix = "_d"
			else:
				suffix = ""
			build_cmd.add_command('move /Y %spython%s.exe ..\\%s' % (subdir, suffix, subdir))
		build_cmd.execute()
		os.chdir("../../../")
	elif "linux" == platform:
		os.chdir("External/Python")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")

def build_libogg(compiler_name, compiler_version, compiler_arch, config_list, platform, ide_name, ide_version):
	if "win32" == platform:
		if "vc" == compiler_name:
			if "x64" == compiler_arch:
				arch = "x64"
				compiler_arch = "x86_amd64"
			else:
				arch = "Win32"
			configs = []
			if "Debug" in config_list:
				configs.append("Debug")
			if ("Release" in config_list) or ("RelWithDebInfo" in config_list) or ("MinSizeRel" in config_list):
				configs.append("Release")
				
			os.chdir("External/libogg/win32/%s%d" % (ide_name, ide_version))
			build_cmd = batch_command()
			build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
			for cfg in configs:
				build_cmd.add_command('devenv libogg_static.sln /Build "%s|%s"' % (cfg, arch))
				if "Debug" == cfg:
					suffix = "_d"
				else:
					suffix = ""
				build_cmd.add_command('move /Y %s\\%s\\libogg_static.lib ..\\..\\lib\\%s\\libogg_static%s.lib' % (arch, cfg, arch, suffix))
			build_cmd.execute()
			os.chdir("../../../../")
		elif "mgw" == compiler_name:
			os.chdir("External/libogg")
			os.system("sh ./configure --disable-shared")
			os.system("mingw32-make.exe")
			copy_to_dst("src\\.libs\\libogg.a", "lib\\libogg.a")
			os.system("mingw32-make.exe clean")
			os.chdir("../../")
	elif "linux" == platform:
		os.chdir("External/libogg")
		os.system("./configure --disable-shared")
		os.system("make")
		copy_to_dst("src/.libs/libogg.a", "lib/libogg.a")
		os.system("make clean")
		os.chdir("../../")

def build_libvorbis(compiler_name, compiler_version, compiler_arch, config_list, platform, ide_name, ide_version):
	if "win32" == platform:
		if "vc" == compiler_name:
			if "x64" == compiler_arch:
				arch = "x64"
				compiler_arch = "x86_amd64"
			else:
				arch = "Win32"
			configs = []
			if "Debug" in config_list:
				configs.append("Debug")
			if ("Release" in config_list) or ("RelWithDebInfo" in config_list) or ("MinSizeRel" in config_list):
				configs.append("Release")

			os.chdir("External/libvorbis/win32/%s%d" % (ide_name, ide_version))
			build_cmd = batch_command()
			build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
			for cfg in configs:
				build_cmd.add_command('devenv vorbis_static.sln /Build "%s|%s"' % (cfg, arch))
				if "Debug" == cfg:
					suffix = "_d"
				else:
					suffix = ""
				build_cmd.add_command('move /Y %s\\%s\\libvorbis_static%s.lib ..\\..\\libs\\%s\\libvorbis_static%s.lib' % (arch, cfg, suffix, arch, suffix))
				build_cmd.add_command('move /Y %s\\%s\\libvorbisfile_static%s.lib ..\\..\\libs\\%s\\libvorbisfile_static%s.lib' % (arch, cfg, suffix, arch, suffix))
			build_cmd.execute()
			os.chdir("../../../../")
		elif "mgw" == compiler_name:
			os.chdir("External/libvorbis")
			os.system("sh ./configure --disable-shared --with-ogg=\"%s\"" % (os.path.realpath(os.path.abspath(".") + "\\../libogg")).replace('\\', '/'))
			os.system("mingw32-make.exe")
			copy_to_dst("lib\\.libs\\libvorbis.a", "libs\\libvorbis.a")
			copy_to_dst("lib\\.libs\\libvorbisfile.a", "libs\\libvorbisfile.a")
			os.system("mingw32-make.exe clean")
			os.chdir("../../")
	elif "linux" == platform:
		os.chdir("External/libvorbis")
		os.system("./configure --disable-shared --with-ogg=\"%s\"" % (os.path.realpath(os.path.abspath(".") + "/../libogg")))
		os.system("make")
		copy_to_dst("lib/.libs/libvorbis.a", "libs/libvorbis.a")
		copy_to_dst("lib/.libs/libvorbisfile.a", "libs/libvorbisfile.a")
		os.system("make clean")
		os.chdir("../../")

def build_freetype(compiler_name, compiler_version, compiler_arch, config_list, platform, ide_version):
	if "win32" == platform:
		if "vc" == compiler_name:
			if "x64" == compiler_arch:
				arch = "x64"
				compiler_arch = "x86_amd64"
			else:
				arch = "Win32"
			configs = []
			if "Debug" in config_list:
				configs.append("Debug")
			if ("Release" in config_list) or ("RelWithDebInfo" in config_list) or ("MinSizeRel" in config_list):
				configs.append("Release")

			os.chdir("External/freetype/builds/win32/vc%s" % ide_version)
			build_cmd = batch_command()
			build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
			for cfg in configs:
				build_cmd.add_command('devenv freetype.sln /Build "%s|%s"' % (cfg, arch))
			build_cmd.execute()
			os.chdir("../../../../../")
		elif "mgw" == compiler_name:
			os.chdir("External/freetype")
			os.system("mingw32-make.exe")
			os.system("mingw32-make.exe")
			copy_to_dst("objs\\freetype.a", "objs\\libfreetype.a")
			os.chdir("../../")
	elif "linux" == platform:
		os.chdir("External/freetype")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")

def build_7z(compiler_name, compiler_version, compiler_arch, config_list, platform):
	if "win32" == platform:
		if "x64" == compiler_arch:
			arch = "x64"
			compiler_arch = "x86_amd64"
			folder_suffix = ""
		elif "arm_app" == compiler_arch:
			arch = "ARM"
			compiler_arch = "x86_arm"
			folder_suffix = "_app"
		elif "x86_app" == compiler_arch:
			arch = "Win32"
			compiler_arch = "x86"
			folder_suffix = "_app"
		else:
			arch = "Win32"
			folder_suffix = ""
		configs = []
		if "Debug" in config_list:
			configs.append("Debug")
		if ("Release" in config_list) or ("RelWithDebInfo" in config_list) or ("MinSizeRel" in config_list):
			configs.append("Release")

		os.chdir("External/7z/build/%s-%d_0%s" % (compiler_name, compiler_version, folder_suffix))
		build_cmd = batch_command()
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
		for cfg in configs:
			build_cmd.add_command('devenv Format7zExtract.sln /Build "%s|%s"' % (cfg, arch))
			build_cmd.add_command('devenv LzmaLib.sln /Build "%s|%s"' % (cfg, arch))
		build_cmd.execute()
		os.chdir("../../../../")
			
def build_external_libs(compiler_name, compiler_version, compiler_arch, generator_name, config_list, platform):
	import glob

	if "win32" == platform:
		dst_dir = "KlayGE/bin/win_%s/" % compiler_arch
		bat_suffix = "bat"
		dll_suffix = "dll"
	elif "linux" == platform:
		dst_dir = "KlayGE/bin/linux_%s/" % compiler_arch
		bat_suffix = "sh"
		dll_suffix = "so"
		
	if "vc" == compiler_name:
		ide_name = "VS"
		if 11 == compiler_version:
			ide_version = 2012
		elif 10 == compiler_version:
			ide_version = 2010
		elif 9 == compiler_version:
			ide_version = 2008
		elif 8 == compiler_version:
			ide_version = 2005
	elif "mgw" == compiler_name:
		ide_name = "mingw"
		ide_version = 0
	elif "gcc" == compiler_name:
		ide_name = "gcc"
		ide_version = 0
	else:
		print("Wrong configuration\n")
		sys.exit(1)


	print("\nBuilding boost...\n")
	build_Boost(compiler_name, compiler_version, compiler_arch, config_list, platform)

	if "vc" == compiler_name:
		compiler_version_str = "%d" % compiler_version
	else:
		compiler_version_str = ""
	for fname in glob.iglob("External/boost/lib_%s%s_%s/lib/*.%s" % (compiler_name, compiler_version_str, compiler_arch, dll_suffix)):
		copy_to_dst(fname, dst_dir)

	if ("vc" == compiler_name) and (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding Python...\n")
		build_Python(compiler_name, compiler_version, compiler_arch, config_list, platform)

		if "x64" == compiler_arch:
			subdir = "amd64/"
		else:
			subdir = ""
		for fname in glob.iglob("External/Python/Dlls/%spython32*.%s" % (subdir, dll_suffix)):
			copy_to_dst(fname, dst_dir)

		if not os.path.exists("%sLib" % dst_dir):
			os.mkdir("%sLib" % dst_dir)
		for fname in glob.iglob("External/Python/Lib/*.py"):
			copy_to_dst(fname, "%sLib/" % dst_dir)
		if not os.path.exists("%sLib/encodings" % dst_dir):
			os.mkdir("%sLib/encodings" % dst_dir)
		for fname in glob.iglob("External/Python/Lib/encodings/*.py"):
			copy_to_dst(fname, "%sLib/encodings/" % dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding libogg...\n")
		build_libogg(compiler_name, compiler_version, compiler_arch, config_list, platform, ide_name, ide_version)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding libvorbis...\n")
		build_libvorbis(compiler_name, compiler_version, compiler_arch, config_list, platform, ide_name, ide_version)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding freetype...\n")
		build_freetype(compiler_name, compiler_version, compiler_arch, config_list, platform, ide_version)

	if ("vc" == compiler_name):
		print("\nBuilding 7z...\n")
		build_7z(compiler_name, compiler_version, compiler_arch, config_list, platform)

		if "x64" == compiler_arch:
			subdir = "x64/"
			folder_suffix = ""
		elif "arm_app" == compiler_arch:
			subdir = "ARM/"
			folder_suffix = "_app"
		elif "x86_app" == compiler_arch:
			subdir = ""
			folder_suffix = "_app"
		else:
			subdir = ""
			folder_suffix = ""
		copy_to_dst("External/7z/build/%s-%d_0%s/%sRelease/7zxa.%s" % (compiler_name, compiler_version, folder_suffix, subdir, dll_suffix), dst_dir)
		copy_to_dst("External/7z/build/%s-%d_0%s/%sRelease/LZMA.%s" % (compiler_name, compiler_version, folder_suffix, subdir, dll_suffix), dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		if "win32" == platform:
			print("\nSeting up DXSDK...\n")

			copy_to_dst("External/DXSDK/Redist/%s/d3dcompiler_46.%s" % (compiler_arch, dll_suffix), dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		if "win32" == platform:
			print("\nSeting up OpenAL SDK...\n")

			copy_to_dst("External/OpenALSDK/redist/%s/OpenAL32.%s" % (compiler_arch, dll_suffix), dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nSeting up Cg...\n")

		if "x64" == compiler_arch:
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

	compiler_info = get_compiler_info(compiler, arch, cfg)

	if 0 == len(compiler_info):
		print("Wrong configuration\n")
		sys.exit(1)

	for arch in compiler_info[2]:
		build_external_libs(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3], compiler_info[4])
