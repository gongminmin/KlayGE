#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_kfont(build_info):
	for compiler_info in build_info.compilers:
		build_a_project("kfont", "kfont", build_info, compiler_info, True)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building kfont...")
	build_kfont(bi)
