#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_glloader(build_info):
	for arch in build_info.arch_list:
		if not arch[3]:
			build_a_project("glloader", "glloader", build_info, arch, True)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building glloader...")
	build_glloader(bi)
