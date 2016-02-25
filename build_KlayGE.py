#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_KlayGE(build_info):
	for compiler_info in build_info.compilers:
		build_a_project("KlayGE", "KlayGE", build_info, compiler_info)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building KlayGE...")
	build_KlayGE(bi)
