#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_glloader(build_info):
	additional_options = '-D PYTHON_EXE:STRING="%s"' % sys.executable
	for compiler_info in build_info.compilers:
		if not compiler_info.is_windows_runtime:
			build_a_project("glloader", "glloader", build_info, compiler_info, True, additional_options)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building glloader...")
	build_glloader(bi)
