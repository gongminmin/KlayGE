#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import sys
from blib_util import *

def build_KFL(build_info):
	for arch in build_info.arch_list:
		build_a_project("KFL", "KFL", build_info, arch)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	print("Building KFL...")
	build_KFL(bi)
