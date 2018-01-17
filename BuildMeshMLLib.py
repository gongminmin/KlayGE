#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildMeshMLLib(build_info):
	for compiler_info in build_info.compilers:
		BuildAProject("MeshMLLib", "MeshMLLib", build_info, compiler_info)

if __name__ == "__main__":
	BuildMeshMLLib(BuildInfo.FromArgv(sys.argv))
