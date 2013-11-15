#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_MeshMLLib(compiler_info, compiler_arch):
	build_a_project("MeshMLLib", "MeshMLLib", compiler_info, compiler_arch)

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

	print("Building MeshMLLib...")
	for arch in ci.arch_list:
		build_MeshMLLib(ci, arch)
