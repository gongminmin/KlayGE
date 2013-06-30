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
	os.chdir("External/boost")
	if "win32" == compiler_info.platform:
		if not os.path.exists("bjam.exe"):
			os.system("bootstrap.bat")
	elif "linux" == compiler_info.platform:
		if not os.path.exists("./bjam"):
			os.system("bootstrap.sh")

	if "vc" == compiler_info.name:
		boost_toolset = "ms%s-%d.0" % (compiler_info.name, compiler_info.version)
	else:
		boost_toolset = "gcc"
		
	address_model = 32
	if "x64" == compiler_arch:
		address_model = 64
	
	options = ""
	if "vc" == compiler_info.name:
		if compiler_info.version >= 10:
			options += " --without-regex"
		if compiler_info.version >= 11:
			options += " --without-atomic --without-chrono --without-date_time --without-filesystem --without-system --without-thread"
	else:
		options += " --without-atomic --without-date_time --without-regex"

	if ("x86" == compiler_arch) or ("x64" == compiler_arch):
		options += " --disable-filesystem2"
	elif ("x86_app" == compiler_arch):
		options += " architecture=x86 --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	elif ("arm_app" == compiler_arch):
		options += " architecture=arm --without-filesystem --without-program_options define=\"WINAPI_FAMILY=WINAPI_FAMILY_APP\" define=BOOST_NO_ANSI_APIS cxxflags=\"/ZW /EHsc\""
	if "vc" == compiler_info.name:
		options += " cxxflags=-wd4819 cxxflags=-wd4910 define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE"

	config = ""
	if "Debug" in compiler_info.cfg:
		config += " debug"
	if ("Release" in compiler_info.cfg) or ("RelWithDebInfo" in compiler_info.cfg) or ("MinSizeRel" in compiler_info.cfg):
		config += " release"
		
	if "vc" == compiler_info.name:
		compiler_version_str = "%d" % compiler_info.version
	else:
		compiler_version_str = ""

	build_cmd = batch_command()
	build_cmd.add_command('bjam.exe --toolset=%s --stagedir=./lib_%s%s_%s --builddir=./ address-model=%d %s link=shared runtime-link=shared threading=multi stage %s' % (boost_toolset, compiler_info.name, compiler_version_str, compiler_arch, address_model, options, config))
	build_cmd.execute()

	os.chdir("../../")

def build_Python(compiler_info, compiler_arch):
	if "win32" == compiler_info.platform:
		if "vc" == compiler_info.name:
			if compiler_info.version >= 11:
				os.chdir("External/Python/vc-11_0")
			else:
				os.chdir("External/Python/PCbuild")
			if compiler_info.version >= 10:
				sln_suffix = "%d" % compiler_info.version
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
			if "Debug" in compiler_info.cfg:
				configs.append("Debug")
			if ("Release" in compiler_info.cfg) or ("RelWithDebInfo" in compiler_info.cfg) or ("MinSizeRel" in compiler_info.cfg):
				configs.append("Release")

			build_cmd = batch_command()
			build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch))
			for cfg in configs:
				compiler_info.msvc_add_build_command(build_cmd, "pcbuild%s" % sln_suffix, "", cfg, arch)
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
		elif "mgw" == compiler_info.name:
			print("Currently Python can't be build by MinGW directly. Please build it by MSVC first\n");
			os.chdir("External/Python/DLLs")
			os.system("dlltool -D python32.dll -d python32.def -l libpython32.a")
			os.system("move /Y libpython32.a ..\libs\"")
			os.system("dlltool -D python32_d.dll -d python32_d.def -l libpython32_d.a")
			os.system("move /Y libpython32_d.a ..\libs\"")
			os.chdir("../../../")
	elif "linux" == compiler_info.platform:
		os.chdir("External/Python")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")

def build_libogg(compiler_info, compiler_arch, generator_name):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/libogg/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch)
	else:
		build_dir = "External/libogg/build/%s-%s" % (compiler_info.name, compiler_arch)
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_info.toolset

	additional_options = ""
	if (compiler_arch.find("_app") > 0):
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (generator_name, toolset_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	if ("x86_app" == compiler_arch):
		compiler_arch = "x86"
	elif ("arm_app" == compiler_arch):
		compiler_arch = "x86_arm"
	elif ("x64" == compiler_arch):
		compiler_arch = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "libogg", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)

def build_libvorbis(compiler_info, compiler_arch, ide_name, ide_version):
	if "win32" == compiler_info.platform:
		if "vc" == compiler_info.name:
			if "x64" == compiler_arch:
				arch = "x64"
				compiler_arch = "x86_amd64"
			else:
				arch = "Win32"
			configs = []
			if "Debug" in compiler_info.cfg:
				configs.append("Debug")
			if ("Release" in compiler_info.cfg) or ("RelWithDebInfo" in compiler_info.cfg) or ("MinSizeRel" in compiler_info.cfg):
				configs.append("Release")

			os.chdir("External/libvorbis/win32/%s%d" % (ide_name, ide_version))
			build_cmd = batch_command()
			build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch))
			for cfg in configs:
				compiler_info.msvc_add_build_command(build_cmd, "vorbis_static", "", cfg, arch)
				if "Debug" == cfg:
					suffix = "_d"
				else:
					suffix = ""
				build_cmd.add_command('move /Y %s\\%s\\libvorbis_static.lib ..\\..\\libs\\%s\\libvorbis_static%s.lib' % (arch, cfg, arch, suffix))
				build_cmd.add_command('move /Y %s\\%s\\libvorbisfile_static.lib ..\\..\\libs\\%s\\libvorbisfile_static%s.lib' % (arch, cfg, arch, suffix))
			build_cmd.execute()
			os.chdir("../../../../")
		elif "mgw" == compiler_info.name:
			os.chdir("External/libvorbis")
			build_cmd = batch_command()
			build_cmd.add_command("@set PATH=%PATH%;C:\\MinGW\\msys\\1.0\\bin")
			build_cmd.add_command("sh ./configure --disable-shared --with-ogg=\"%s\"" % (os.path.realpath(os.path.abspath(".") + "\\../libogg")).replace('\\', '/'))
			build_cmd.add_command("mingw32-make.exe")
			build_cmd.add_command("copy /Y lib\\.libs\\libvorbis.a libs\\libvorbis.a")
			build_cmd.add_command("copy /Y lib\\.libs\\libvorbisfile.a libs\\libvorbisfile.a")
			build_cmd.add_command("mingw32-make.exe clean")
			build_cmd.execute()
			os.chdir("../../")
	elif "linux" == compiler_info.platform:
		os.chdir("External/libvorbis")
		os.system("./configure --disable-shared --with-ogg=\"%s\"" % (os.path.realpath(os.path.abspath(".") + "/../libogg")))
		os.system("make")
		copy_to_dst("lib/.libs/libvorbis.a", "libs/libvorbis.a")
		copy_to_dst("lib/.libs/libvorbisfile.a", "libs/libvorbisfile.a")
		os.system("make clean")
		os.chdir("../../")

def build_freetype(compiler_info, compiler_arch, ide_version):
	if "win32" == compiler_info.platform:
		if "vc" == compiler_info.name:
			if "x64" == compiler_arch:
				arch = "x64"
				compiler_arch = "x86_amd64"
			else:
				arch = "Win32"
			configs = []
			if "Debug" in compiler_info.cfg:
				configs.append("Debug")
			if ("Release" in compiler_info.cfg) or ("RelWithDebInfo" in compiler_info.cfg) or ("MinSizeRel" in compiler_info.cfg):
				configs.append("Release")

			os.chdir("External/freetype/builds/win32/vc%s" % ide_version)
			build_cmd = batch_command()
			build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch))
			for cfg in configs:
				compiler_info.msvc_add_build_command(build_cmd, "freetype", "", cfg, arch)
			build_cmd.execute()
			os.chdir("../../../../../")
		elif "mgw" == compiler_info.name:
			os.chdir("External/freetype")
			os.system("mingw32-make.exe")
			os.system("mingw32-make.exe")
			copy_to_dst("objs\\freetype.a", "objs\\libfreetype.a")
			os.chdir("../../")
	elif "linux" == compiler_info.platform:
		os.chdir("External/freetype")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")

def build_7z(compiler_info, compiler_arch, generator_name):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "External/7z/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch)
	else:
		build_dir = "External/7z/build/%s-%s" % (compiler_info.name, compiler_arch)
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_info.toolset

	additional_options = ""
	if (compiler_arch.find("_app") > 0):
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s %s' % (generator_name, toolset_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	if ("x86_app" == compiler_arch):
		compiler_arch = "x86"
	elif ("arm_app" == compiler_arch):
		compiler_arch = "x86_arm"
	elif ("x64" == compiler_arch):
		compiler_arch = "x86_amd64"

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "7z", "ALL_BUILD", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe')
	else:
		build_cmd.add_command('make')
	build_cmd.execute()

	os.chdir(curdir)
			
def build_external_libs(compiler_info, compiler_arch, generator_name):
	import glob

	if "win32" == compiler_info.platform:
		platform_dir = "win_%s" % compiler_arch
		dst_dir = "KlayGE/bin/%s/" % platform_dir
		bat_suffix = "bat"
		dll_suffix = "dll"
	elif "linux" == compiler_info.platform:
		platform_dir = "linux_%s" % compiler_arch
		dst_dir = "KlayGE/bin/%s/" % platform_dir
		bat_suffix = "sh"
		dll_suffix = "so"
		
	if "vc" == compiler_info.name:
		ide_name = "VS"
		if 11 == compiler_info.version:
			ide_version = 2012
		elif 10 == compiler_info.version:
			ide_version = 2010
		elif 9 == compiler_info.version:
			ide_version = 2008
		elif 8 == compiler_info.version:
			ide_version = 2005
	elif "mgw" == compiler_info.name:
		ide_name = "mingw"
		ide_version = 0
	elif "gcc" == compiler_info.name:
		ide_name = "gcc"
		ide_version = 0
	else:
		print("Wrong configuration\n")
		sys.exit(1)


	print("\nBuilding boost...\n")
	build_Boost(compiler_info, compiler_arch)

	if "vc" == compiler_info.name:
		compiler_version_str = "%d" % compiler_info.version
	else:
		compiler_version_str = ""
	for fname in glob.iglob("External/boost/lib_%s%s_%s/lib/*.%s" % (compiler_info.name, compiler_version_str, compiler_arch, dll_suffix)):
		copy_to_dst(fname, dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding Python...\n")
		build_Python(compiler_info, compiler_arch)

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
		build_libogg(compiler_info, compiler_arch, generator_name)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding libvorbis...\n")
		build_libvorbis(compiler_info, compiler_arch, ide_name, ide_version)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		print("\nBuilding freetype...\n")
		build_freetype(compiler_info, compiler_arch, ide_version)

	if ("vc" == compiler_info.name):
		print("\nBuilding 7z...\n")
		build_7z(compiler_info, compiler_arch, generator_name)

		for fname in glob.iglob("External/7z/lib/%s/7zxa*.%s" % (platform_dir, dll_suffix)):
			copy_to_dst(fname, dst_dir)
		for fname in glob.iglob("External/7z/lib/%s/LZMA*.%s" % (platform_dir, dll_suffix)):
			copy_to_dst(fname, dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		if "win32" == compiler_info.platform:
			print("\nSeting up DXSDK...\n")

			copy_to_dst("External/DXSDK/Redist/%s/d3dcompiler_46.%s" % (compiler_arch, dll_suffix), dst_dir)

	if (compiler_arch != "x86_app") and (compiler_arch != "arm_app"):
		if "win32" == compiler_info.platform:
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

	ci = compiler_info(compiler, arch, cfg)

	if 0 == len(ci.name):
		print("Wrong configuration\n")
		sys.exit(1)

	for arch in ci.arch_list:
		build_external_libs(ci, arch[0], arch[1])
