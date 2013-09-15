#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_glloader(compiler_info, compiler_arch):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_info.name:
		build_dir = "glloader/build/%s-%d_0-%s" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "glloader/build/%s-%s" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	toolset_name = ""
	if "vc" == compiler_info.name:
		toolset_name = "-T %s" % compiler_arch[2]

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s -D GLLOADER_USE_GLES:BOOL="FALSE" -D PYTHON_EXE:STRING="%s" %s' % (compiler_arch[1], toolset_name, sys.executable, "../cmake"))
	cmake_cmd.execute()

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch[0]))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "glloader", "ALL_BUILD", config)
			compiler_info.msvc_add_build_command(build_cmd, "glloader", "INSTALL", config)
	elif "mgw" == compiler_info.name:
		build_cmd.add_command('mingw32-make.exe install')
	else:
		build_cmd.add_command('make install')
	build_cmd.execute()

	os.chdir(curdir)

	if "vc" == compiler_info.name:
		build_dir = "glloader/build/%s-%d_0-%s-es" % (compiler_info.name, compiler_info.version, compiler_arch[0])
	else:
		build_dir = "glloader/build/%s-%s-es" % (compiler_info.name, compiler_arch[0])
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s -D GLLOADER_USE_GLES:BOOL="TRUE" -D PYTHON_EXE:STRING="%s" %s' % (compiler_arch[1], toolset_name, sys.executable, "../cmake"))
	cmake_cmd.execute()

	build_cmd = batch_command()
	if "vc" == compiler_info.name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_info.version, compiler_arch[0]))
		for config in compiler_info.cfg:
			compiler_info.msvc_add_build_command(build_cmd, "glloader", "ALL_BUILD", config)
			compiler_info.msvc_add_build_command(build_cmd, "glloader", "INSTALL", config)
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

	print("Building glloader...")
	for arch in ci.arch_list:
		if not arch[3]:
			build_glloader(ci, arch)
