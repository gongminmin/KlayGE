#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildKFont(build_info):
	for compiler_info in build_info.compilers:
		BuildAProject("kfont", "kfont", build_info, compiler_info, build_info.is_windows)

if __name__ == "__main__":
	BuildKFont(BuildInfo.FromArgv(sys.argv))
