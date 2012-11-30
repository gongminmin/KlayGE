#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_kfont(compiler_name, compiler_version, compiler_arch, generator_name, config_list):
	curdir = os.path.abspath(os.curdir)

	build_dir = "kfont/build/%s-%d_0-%s" % (compiler_name, compiler_version, compiler_arch)
	if not os.path.exists(build_dir):
		os.makedirs(build_dir)

	os.chdir(build_dir)
	
	additional_options = ""
	if (compiler_arch.find("_app") > 0):
		additional_options += "-D KFONT_WITH_WINRT:BOOL=\"TRUE\""

	cmake_cmd = batch_command()
	cmake_cmd.add_command('cmake -G "%s" %s %s' % (generator_name, additional_options, "../cmake"))
	cmake_cmd.execute()

	build_cmd = batch_command()
	build_cmd.add_command('CALL "%%VS%d0COMNTOOLS%%..\\..\\VC\\vcvarsall.bat" %s' % (compiler_version, compiler_arch))
	for config in config_list:
		build_cmd.add_command('devenv kfont.sln /Build %s /project ALL_BUILD' % config)
		build_cmd.add_command('devenv kfont.sln /Build %s /project Install' % config)
	build_cmd.execute()

	os.chdir(curdir)

if __name__ == "__main__":
	if len(sys.argv) > 1:
		compiler = sys.argv[1]
	else:
		compiler = ""
	if len(sys.argv) > 2:
		arch = sys.argv[2]
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

	print("Building kfont...")
	for arch in compiler_info[2]:
		build_kfont(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])
