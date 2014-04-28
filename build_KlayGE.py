#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_KlayGE(build_info):
	for arch in build_info.arch_list:
		build_a_project("KlayGE", "KlayGE", build_info, arch)

def build_Samples(build_info):
	for arch in build_info.arch_list:
		build_a_project("Samples", "KlayGE/Samples", build_info, arch)

def build_Tools(build_info):
	for arch in build_info.arch_list:
		if (not arch[3]) and (build_info.target_platform != "android"):
			build_a_project("Tools", "KlayGE/Tools", build_info, arch)

def build_Tutorials(build_info):
	for arch in build_info.arch_list:
		build_a_project("Tutorials", "KlayGE/Tutorials", build_info, arch)

def build_Exporters(build_info):
	for arch in build_info.arch_list:
		if (not arch[3]) and (build_info.target_platform != "android"):
			build_a_project("Exporters", "KlayGE/Exporters", build_info, arch)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building KlayGE...")
	build_KlayGE(bi)

	print("Building KlayGE Samples...")
	build_Samples(bi)

	print("Building KlayGE Tools...")
	build_Tools(bi)

	print("Building KlayGE Tutorials...")
	build_Tutorials(bi)

	print("Building KlayGE Exporters...")
	build_Exporters(bi)
