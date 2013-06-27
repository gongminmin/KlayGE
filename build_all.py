#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from build_external import build_external_libs
from blib_util import *
from build_KFL import *
from build_glloader import *
from build_kfont import *
from build_MeshMLLib import *
from build_KlayGE import *

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

	print("Building external libs...")
	for arch in ci.arch_list:
		build_external_libs(ci, arch[0], arch[1])

	print("Building KFL...")
	for arch in ci.arch_list:
		build_KFL(ci, arch[0], arch[1])

	print("Building glloader...")
	for arch in ci.arch_list:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_glloader(ci, arch[0], arch[1])

	print("Building kfont...")
	for arch in ci.arch_list:
		build_kfont(ci, arch[0], arch[1])

	print("Building MeshMLLib...")
	for arch in ci.arch_list:
		build_MeshMLLib(ci, arch[0], arch[1])

	print("Building KlayGE...")
	for arch in ci.arch_list:
		build_KlayGE(ci, arch[0], arch[1])

	print("Building KlayGE Samples...")
	for arch in ci.arch_list:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_Samples(ci, arch[0], arch[1])

	print("Building KlayGE Tools...")
	for arch in ci.arch_list:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_Tools(ci, arch[0], arch[1])

	print("Building KlayGE Tutorials...")
	for arch in ci.arch_list:
		if (arch[0] != "x86_app") and (arch[0] != "arm_app"):
			build_Tutorials(ci, arch[0], arch[1])
