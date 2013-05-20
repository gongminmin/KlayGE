#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_glloader(compiler_name, compiler_version, compiler_arch, generator_name, config_list, toolset):
	curdir = os.path.abspath(os.curdir)

	if "vc" == compiler_name:
		build_dir = "glloader/build/%s-%d_0-%s" % (compiler_name, compiler_version, compiler_arch)
	else:
		build_dir = "glloader/build/%s-%s" % (compiler_name, compiler_arch)
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	toolset_name = ""
	if "vc" == compiler_name:
		toolset_name = "-T %s" % toolset

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s -D GLLOADER_USE_GLES:BOOL="FALSE" -D PYTHON_EXE:STRING="%s" %s' % (generator_name, toolset_name, sys.executable, "../cmake"))
	cmake_cmd.execute()

	build_cmd = batch_command()
	if "vc" == compiler_name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
		for config in config_list:
			build_cmd.add_command('devenv glloader.sln /Build %s /project ALL_BUILD' % config)
			build_cmd.add_command('devenv glloader.sln /Build %s /project Install' % config)
	elif "mgw" == compiler_name:
		build_cmd.add_command('mingw32-make.exe install')
	else:
		build_cmd.add_command('make install')
	build_cmd.execute()

	os.chdir(curdir)

	if "vc" == compiler_name:
		build_dir = "glloader/build/%s-%d_0-%s-es" % (compiler_name, compiler_version, compiler_arch)
	else:
		build_dir = "glloader/build/%s-%s-es" % (compiler_name, compiler_arch)
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s -D GLLOADER_USE_GLES:BOOL="TRUE" -D PYTHON_EXE:STRING="%s" %s' % (generator_name, toolset_name, sys.executable, "../cmake"))
	cmake_cmd.execute()

	build_cmd = batch_command()
	if "vc" == compiler_name:
		build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
		for config in config_list:
			build_cmd.add_command('devenv glloader.sln /Build %s /project ALL_BUILD' % config)
			build_cmd.add_command('devenv glloader.sln /Build %s /project Install' % config)
	elif "mgw" == compiler_name:
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

	compiler_info = get_compiler_info(compiler, arch, cfg)

	if 0 == len(compiler_info):
		print("Wrong configuration\n")
		sys.exit(1)

	print("Building glloader...")
	for arch in compiler_info[2]:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_glloader(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3], compiler_info[5])
