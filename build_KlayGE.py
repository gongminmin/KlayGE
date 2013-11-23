#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_KlayGE(compiler_info):
	for arch in compiler_info.arch_list:
		build_a_project("KlayGE", "KlayGE", compiler_info, arch)

def build_Samples(compiler_info):
	for arch in compiler_info.arch_list:
		build_a_project("Samples", "KlayGE/Samples", compiler_info, arch)

def build_Tools(compiler_info):
	for arch in compiler_info.arch_list:
		if not arch[3]:
			build_a_project("Tools", "KlayGE/Tools", compiler_info, arch)

def build_Tutorials(compiler_info):
	for arch in compiler_info.arch_list:
		if not arch[3]:
			build_a_project("Tutorials", "KlayGE/Tutorials", compiler_info, arch)

def build_Exporters(compiler_info):
	for arch in compiler_info.arch_list:
		if not arch[3]:
			build_a_project("Exporters", "KlayGE/Exporters", compiler_info, arch)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	ci = compiler_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building KlayGE...")
	build_KlayGE(ci)

	print("Building KlayGE Samples...")
	build_Samples(ci)

	print("Building KlayGE Tools...")
	build_Tools(ci)

	print("Building KlayGE Tutorials...")
	build_Tutorials(ci)

	print("Building KlayGE Exporters...")
	build_Exporters(ci)
