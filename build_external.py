#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os
import sys

env = os.environ

if len(sys.argv) > 1:
	cfg = sys.argv[1]
else:
	if "VS100COMNTOOLS" in env:
		cfg = "vc10"
	elif "VS90COMNTOOLS" in env:
		cfg = "vc9"
	elif "VS80COMNTOOLS" in env:
		cfg = "vc8"

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
else:
	print("Wrong configuration\n")
	sys.exit(1)

print("\nBuilding boost...\n")

if not os.path.exists("External/boost/bjam.exe"):
	os.system("External/boost/bootstrap.bat")

if "vc" == compiler_name:
	boost_toolset_str = "ms%s-%d.0" % (compiler_name, compiler_version)
boost_define_str = "define=_CRT_SECURE_NO_DEPRECATE define=_SCL_SECURE_NO_DEPRECATE"
if ("vc" == compiler_name) and ((8 == compiler_version) or (9 == compiler_version)):
	boost_define_str += " define=_SECURE_SCL=0"
os.chdir("External/boost")
os.system("build_%s%d.bat" % (compiler_name, compiler_version))
os.chdir("../../")


print("\nBuilding Python...\n")

os.chdir("External/Python/PCbuild")
os.system("build_%s%d_all.bat" % (compiler_name, compiler_version))
os.chdir("../../../")


print("\nBuilding libogg...\n")

os.chdir("External/libogg/win32/%s%d" % (ide_name, ide_version))
os.system("build_all.bat")
os.chdir("../../../../")


print("\nBuilding libvorbis...\n")

os.chdir("External/libvorbis/win32/%s%d" % (ide_name, ide_version))
os.system("build_all.bat")
os.chdir("../../../../")


print("\nBuilding freetype...\n")

os.chdir("External/freetype/builds/win32/vc%s" % ide_version)
os.system("build_all.bat")
os.chdir("../../../../../")
