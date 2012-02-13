#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys

def copy_to_dst(src_name, dst_dir):
	print("Copy %s to %s" % (src_name, dst_dir))
	import shutil
	shutil.copy(src_name, dst_dir)

def build_external_libs(cfg):
	import os
	import glob

	env = os.environ
	platform = sys.platform
	if 0 == platform.find("linux"):
		platform = "linux"

	if "" == cfg:
		if "win32" == platform:
			if "VS100COMNTOOLS" in env:
				cfg = "vc10"
			elif "VS90COMNTOOLS" in env:
				cfg = "vc9"
			elif "VS80COMNTOOLS" in env:
				cfg = "vc8"
			elif os.path.exists("C:\MinGW\bin\gcc.exe"):
				cfg = "mingw"
		elif "linux" == platform:
			cfg = "gcc"
		else:
			print("Unsupported platform\n")
			sys.exit(1)

	if "vc10" == cfg:
		compiler_name = "vc"
		compiler_version = 10
		ide_name = "VS"
		ide_version = 2010
	elif "vc9" == cfg:
		compiler_name = "vc"
		compiler_version = 9
		ide_name = "VS"
		ide_version = 2008
	elif "vc8" == cfg:
		compiler_name = "vc"
		compiler_version = 8
		ide_name = "VS"
		ide_version = 2005
	elif "mingw" == cfg:
		compiler_name = "gcc"
		compiler_version = 0
		ide_name = "mingw"
		ide_version = 0
	elif "gcc" == cfg:
		compiler_name = "gcc"
		compiler_version = 0
		ide_name = "gcc"
		ide_version = 0
	else:
		print("Wrong configuration\n")
		sys.exit(1)

	if "win32" == platform:
		dst_dir_x86 = "KlayGE/bin/win_x86/"
		dst_dir_x64 = "KlayGE/bin/win_x64/"
		bat_suffix = "bat"
		dll_suffix = "dll"
	elif "linux" == platform:
		dst_dir_x86 = "KlayGE/bin/linux_x86/"
		dst_dir_x64 = "KlayGE/bin/linux_x64/"	
		bat_suffix = "sh"
		dll_suffix = "so"


	print("\nBuilding boost...\n")

	os.chdir("External/boost")
	if "win32" == platform:
		if not os.path.exists("bjam.exe"):
			os.system("bootstrap.bat")
	elif "linux" == platform:
		if not os.path.exists("./bjam"):
			os.system("bootstrap.sh")

	if "vc" == compiler_name:
		boost_toolset = "%s%d" % (compiler_name, compiler_version)
	else:
		boost_toolset = "gcc"
	os.system("build_%s.%s" % (boost_toolset, bat_suffix))
	os.chdir("../../")

	for fname in glob.iglob("External/boost/lib_%s_x86/lib/*.%s" % (boost_toolset, dll_suffix)):
		copy_to_dst(fname, dst_dir_x86)
	for fname in glob.iglob("External/boost/lib_%s_x64/lib/*.%s" % (boost_toolset, dll_suffix)):
		copy_to_dst(fname, dst_dir_x64)


	print("\nBuilding Python...\n")

	if "win32" == platform:
		os.chdir("External/Python/PCbuild")
		os.system("build_%s%d_all.bat" % (compiler_name, compiler_version))
		os.chdir("../../../")
	elif "linux" == platform:
		os.chdir("External/Python")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")

	copy_to_dst("External/Python/DLLs/python32.%s" % dll_suffix, dst_dir_x86)
	copy_to_dst("External/Python/DLLs/python32_d.%s" % dll_suffix, dst_dir_x86)
	copy_to_dst("External/Python/DLLs/amd64/python32.%s" % dll_suffix, dst_dir_x64)
	copy_to_dst("External/Python/DLLs/amd64/python32_d.%s" % dll_suffix, dst_dir_x64)


	print("\nBuilding libogg...\n")

	if "win32" == platform:
		os.chdir("External/libogg/win32/%s%d" % (ide_name, ide_version))
		os.system("build_all.bat")
		os.chdir("../../../../")
	elif "linux" == platform:
		os.chdir("External/libogg")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")


	print("\nBuilding libvorbis...\n")

	if "win32" == platform:
		os.chdir("External/libvorbis/win32/%s%d" % (ide_name, ide_version))
		os.system("build_all.bat")
		os.chdir("../../../../")
	elif "linux" == platform:
		os.chdir("External/libvorbis")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")


	print("\nBuilding freetype...\n")

	if "win32" == platform:
		os.chdir("External/freetype/builds/win32/vc%s" % ide_version)
		os.system("build_all.bat")
		os.chdir("../../../../../")
	elif "linux" == platform:
		os.chdir("External/freetype")
		os.system("./configure")
		os.system("make")
		os.chdir("../../")
		
		
	print("\nBuilding 7z...\n")

	if "win32" == platform:
		os.chdir("External/7z/build/%s-%d_0" % (compiler_name, compiler_version))
		os.system("build_all.bat")
		os.chdir("../../../../")

	copy_to_dst("External/7z/build/%s-%d_0/Release/7zxa.%s" % (compiler_name, compiler_version, dll_suffix), dst_dir_x86)
	copy_to_dst("External/7z/build/%s-%d_0/x64/Release/7zxa.%s" % (compiler_name, compiler_version, dll_suffix), dst_dir_x64)
	copy_to_dst("External/7z/build/%s-%d_0/Release/LZMA.%s" % (compiler_name, compiler_version, dll_suffix), dst_dir_x86)
	copy_to_dst("External/7z/build/%s-%d_0/x64/Release/LZMA.%s" % (compiler_name, compiler_version, dll_suffix), dst_dir_x64)


	if "win32" == platform:
		print("\nSeting up DXSDK...\n")

		os.chdir("External/DXSDK/Redist")
		os.system("dxwebsetup.exe")
		os.chdir("../../../")


	if "win32" == platform:
		print("\nSeting up OpenAL SDK...\n")

		os.chdir("External/OpenALSDK/redist")
		os.system("oalinst.exe")
		os.chdir("../../../")


	print("\nSeting up Cg...\n")

	copy_to_dst("External/Cg/bin/cg.%s" % dll_suffix, dst_dir_x86)
	copy_to_dst("External/Cg/bin.x64/cg.%s" % dll_suffix, dst_dir_x64)

if __name__ == "__main__":
	if len(sys.argv) > 1:
		cfg = sys.argv[1]
	else:
		cfg = ""

	build_external_libs(cfg)
