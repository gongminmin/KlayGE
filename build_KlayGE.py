#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_KlayGE(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "KlayGE/Build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "KlayGE/Build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = ""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""
	if compiler_arch[4]:
		additional_options += "-D KLAYGE_WITH_XP_TOOLSET:BOOL=\"TRUE\""

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
			compiler_info.msvc_add_build_command(build_cmd, "KlayGE", "ALL_BUILD", config)
			compiler_info.msvc_add_build_command(build_cmd, "KlayGE", "INSTALL", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe install')
	else:
		build_cmd.add_command('make install')
	build_cmd.execute()

	os.chdir(curdir)

def build_Samples(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "KlayGE/Samples/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "KlayGE/Samples/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]
	
	additional_options = ""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""

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
			compiler_info.msvc_add_build_command(build_cmd, "Samples", "ALL_BUILD", config)
			compiler_info.msvc_add_build_command(build_cmd, "Samples", "INSTALL", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe install')
	else:
		build_cmd.add_command('make install')
	build_cmd.execute()

	os.chdir(curdir)

def build_Tools(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "KlayGE/Tools/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "KlayGE/Tools/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s' % (compiler_arch[1], toolset_name, "../cmake"))
	cmake_cmd.execute()

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch[0]))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "Tools", "ALL_BUILD", config)
			compiler_info.msvc_add_build_command(build_cmd, "Tools", "INSTALL", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe install')
	else:
		build_cmd.add_command('make install')
	build_cmd.execute()

	os.chdir(curdir)

def build_Tutorials(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "KlayGE/Tutorials/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "KlayGE/Tutorials/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	additional_options = ""
	if compiler_arch[3]:
		additional_options += "-D KLAYGE_WITH_WINRT:BOOL=\"TRUE\""

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
			compiler_info.msvc_add_build_command(build_cmd, "Tutorials", "ALL_BUILD", config)
			compiler_info.msvc_add_build_command(build_cmd, "Tutorials", "INSTALL", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe install')
	else:
		build_cmd.add_command('make install')
	build_cmd.execute()

	os.chdir(curdir)

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

	print("Building KlayGE...")
	for arch in ci.arch_list:
		build_KlayGE(ci, arch)

	print("Building Samples...")
	for arch in ci.arch_list:
		if not arch[3]:
			build_Samples(ci, arch)

	print("Building Tools...")
	for arch in ci.arch_list:
		if not arch[3]:
			build_Tools(ci, arch)

	print("Building Tutorials...")
	for arch in ci.arch_list:
		if not arch[3]:
			build_Tutorials(ci, arch)
