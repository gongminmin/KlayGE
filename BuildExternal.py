#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildExternalLibs(build_info):
	for compiler_info in build_info.compilers:
		BuildAProject("External", "External", build_info, compiler_info)

if __name__ == "__main__":
	BuildExternalLibs(BuildInfo.FromArgv(sys.argv))

	if sys.argv[1].lower() == "clean":
		import shutil
		clean_dir_list = [ "assimp", "cxxopts", "libogg", "nanosvg", "rapidjson" ]
		for dir in clean_dir_list:
			dir_name = "External/" + dir
			if os.path.isdir(dir_name):
				shutil.rmtree(dir_name)
