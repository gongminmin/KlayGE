#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from build_external import build_external_libs
from blib_util import *
from build_glloader import *
from build_kfont import *
from build_KlayGE import *

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

	print("Building external libs...")
	for arch in compiler_info[2]:
		build_external_libs(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3], compiler_info[4])

	print("Building glloader...")
	for arch in compiler_info[2]:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_glloader(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])

	print("Building kfont...")
	for arch in compiler_info[2]:
		build_kfont(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])

	print("Building KlayGE...")
	for arch in compiler_info[2]:
		build_KlayGE(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])

	print("Building KlayGE Samples...")
	for arch in compiler_info[2]:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_Samples(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])

	print("Building KlayGE Tools...")
	for arch in compiler_info[2]:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_Tools(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])

	print("Building KlayGE Tutorials...")
	for arch in compiler_info[2]:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_Tutorials(compiler_info[0], compiler_info[1], arch[0], arch[1], compiler_info[3])
