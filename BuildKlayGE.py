#!/usr/bin/env python
#-*- coding: ascii -*-

import sys
from BLibUtil import *

def BuildKlayGE(build_info):
	additional_options = " -DKLAYGE_SHADER_PLATFORM_NAME:STRING=\"%s\"" % build_info.shader_platform_name
	if not build_info.is_windows_store:
		if build_info.gles_include_dir != "auto":
			additional_options += " -DKLAYGE_GLES_INCLUDE_DIR:STRING=\"%s\"" % build_info.gles_include_dir
	if build_info.is_windows_desktop:
		if build_info.max_path != "auto":
			additional_options += " -DKLAYGE_3DSMAX_PATH:STRING=\"%s\"" % build_info.max_path
		if build_info.max_sdk_path != "auto":
			additional_options += " -DKLAYGE_3DSMAX_SDK_PATH:STRING=\"%s\"" % build_info.max_sdk_path
		if build_info.maya_path != "auto":
			additional_options += " -DKLAYGE_MAYA_PATH:STRING=\"%s\"" % build_info.maya_path
		if build_info.libovr_path != "auto":
			additional_options += " -DKLAYGE_LibOVR_PATH:STRING=\"%s\"" % build_info.libovr_path
	for compiler_info in build_info.compilers:
		BuildAProject("KlayGE", "KlayGE", build_info, compiler_info, False, additional_options)

if __name__ == "__main__":
	BuildKlayGE(BuildInfo.FromArgv(sys.argv))
