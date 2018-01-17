#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildKFL(build_info):
	for compiler_info in build_info.compilers:
		BuildAProject("KFL", "KFL", build_info, compiler_info)

if __name__ == "__main__":
	BuildKFL(BuildInfo.FromArgv(sys.argv))
