#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildGlloader(build_info):
	if not build_info.is_windows_store:
		additional_options = ""
		if build_info.gles_include_dir != "auto":
			additional_options += " -DKLAYGE_GLES_INCLUDE_DIR:STRING=\"%s\"" % build_info.gles_include_dir
		for compiler_info in build_info.compilers:
			BuildAProject("glloader", "glloader", build_info, compiler_info, build_info.is_windows, additional_options)

if __name__ == "__main__":
	BuildGlloader(BuildInfo.FromArgv(sys.argv))
