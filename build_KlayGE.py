#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_KlayGE(build_info):
	for compiler_info in build_info.compilers:
		build_a_project("KlayGE", "KlayGE", build_info, compiler_info)

def build_Samples(build_info):
	for compiler_info in build_info.compilers:
		build_a_project("Samples", "KlayGE/Samples", build_info, compiler_info)

def build_Tools(build_info):
	for compiler_info in build_info.compilers:
		if (not compiler_info.is_windows_runtime) and (not compiler_info.is_android) and (not compiler_info.is_ios):
			build_a_project("Tools", "KlayGE/Tools", build_info, compiler_info)

def build_Tutorials(build_info):
	for compiler_info in build_info.compilers:
		build_a_project("Tutorials", "KlayGE/Tutorials", build_info, compiler_info)

def build_Exporters(build_info):
	for compiler_info in build_info.compilers:
		if (not compiler_info.is_windows_runtime) and (not compiler_info.is_android) and (not compiler_info.is_ios):
			build_a_project("Exporters", "KlayGE/Exporters", build_info, compiler_info)

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
