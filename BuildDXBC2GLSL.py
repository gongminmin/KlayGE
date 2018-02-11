#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildDXBC2GLSL(build_info):
	if build_info.is_dev_platform:
		for compiler_info in build_info.compilers:
			BuildAProject("DXBC2GLSL", "DXBC2GLSL", build_info, compiler_info)

if __name__ == "__main__":
	BuildDXBC2GLSL(BuildInfo.FromArgv(sys.argv))
