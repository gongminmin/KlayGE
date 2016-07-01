#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_DXBC2GLSL(build_info):
	if (build_info.target_platform != "android") and (build_info.target_platform != "ios") and (not build_info.is_windows_runtime):
		for compiler_info in build_info.compilers:
			build_a_project("DXBC2GLSL", "DXBC2GLSL", build_info, compiler_info)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_DXBC2GLSL(bi)
