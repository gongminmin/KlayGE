#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_KlayGE(compiler_info, compiler_arch):
	build_a_project("KlayGE", "KlayGE", compiler_info, compiler_arch)

def build_Samples(compiler_info, compiler_arch):
	build_a_project("Samples", "KlayGE/Samples", compiler_info, compiler_arch)

def build_Tools(compiler_info, compiler_arch):
	build_a_project("Tools", "KlayGE/Tools", compiler_info, compiler_arch)

def build_Tutorials(compiler_info, compiler_arch):
	build_a_project("Tutorials", "KlayGE/Tutorials", compiler_info, compiler_arch)

def build_Exporters(compiler_info, compiler_arch):
	build_a_project("Exporters", "KlayGE/Exporters", compiler_info, compiler_arch)

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
		log_error("Wrong configuration\n")

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

	print("Building Exporters...")
	for arch in ci.arch_list:
		if not arch[3]:
			build_Exporters(ci, arch)
