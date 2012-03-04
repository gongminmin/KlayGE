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
		cfg = sys.argv[1]
	else:
		cfg = ""

	compiler_info = get_compiler_info(cfg)

	if 0 == len(compiler_info):
		print("Wrong configuration\n")
		sys.exit(1)

	print("Building external libs...")
	build_external_libs(cfg)

	print("Building glloader...")
	for arch in compiler_info[2]:
		build_glloader(compiler_info[0], compiler_info[1], arch[0], arch[1])

	print("Building kfont...")
	for arch in compiler_info[2]:
		build_kfont(compiler_info[0], compiler_info[1], arch[0], arch[1])

	print("Building KlayGE...")
	for arch in compiler_info[2]:
		build_KlayGE(compiler_info[0], compiler_info[1], arch[0], arch[1])

	print("Building KlayGE Samples...")
	for arch in compiler_info[2]:
		build_Samples(compiler_info[0], compiler_info[1], arch[0], arch[1])

	print("Building KlayGE Tools...")
	for arch in compiler_info[2]:
		build_Tools(compiler_info[0], compiler_info[1], arch[0], arch[1])

