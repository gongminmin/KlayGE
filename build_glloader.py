#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_glloader(build_info):
	for compiler_info in build_info.compilers:
		if not compiler_info.is_windows_runtime:
			build_a_project("glloader", "glloader", build_info, compiler_info, compiler_info.is_windows)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_glloader(bi)
