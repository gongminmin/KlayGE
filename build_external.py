#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_Boost(build_info, compiler_info):
	with_filesystem = True
	with_system = True
	if build_info.is_dev_platform:
		with_program_options = True
		with_test = True
	else:
		with_program_options = False
		with_test = False
	if ("vc" == build_info.compiler_name) or ((("gcc" == build_info.compiler_name) or ("mgw" == build_info.compiler_name)) and (build_info.compiler_version > 60)):
		with_filesystem = False
		with_system = False

	need_install = False
	additional_options = " -DWITH_FILESYSTEM:BOOL="
	if with_filesystem:
		additional_options += "\"ON\""
		need_install = True
	else:
		additional_options += "\"OFF\""
	additional_options += " -DWITH_PROGRAM_OPTIONS:BOOL="
	if with_program_options:
		additional_options += "\"ON\""
		need_install = True
	else:
		additional_options += "\"OFF\""
	additional_options += " -DWITH_SYSTEM:BOOL="
	if with_system:
		additional_options += "\"ON\""
		need_install = True
	else:
		additional_options += "\"OFF\""
	additional_options += " -DWITH_TEST:BOOL="
	if with_test:
		additional_options += "\"ON\""
		need_install = True
	else:
		additional_options += "\"OFF\""
	build_a_project("boost", "External/boost", build_info, compiler_info, build_info.is_windows and need_install, additional_options)

def build_Python(build_info, compiler_info):
	additional_options = "-DPY_VERSION_MAJOR=\"3\" -DPY_VERSION_MINOR=\"5\" -DPY_VERSION_PATCH=\"1\" \
		-DCMAKE_CONFIGURATION_TYPES=\"Debug;Release;RelWithDebInfo;MinSizeRel\" \
		-DDOWNLOAD_SOURCES=\"OFF\" \
		-DBUILD_EXTENSIONS_AS_BUILTIN=\"ON\" \
		-DBUILD_FREEZE_IMPORTLIB=\"OFF\" \
		-DBUILD_PYTHON_DEVELOPMENT=\"OFF\" \
		-DBUILD_PYTHON_EXECUTABLE=\"OFF\" \
		-DBUILD_PGEN=\"OFF\" \
		-DBUILD_WININST=\"OFF\" \
		-DENABLE_AUDIOOP=\"OFF\" \
		-DENABLE_CODECS_CN=\"OFF\" \
		-DENABLE_CODECS_HK=\"OFF\" \
		-DENABLE_CODECS_ISO2022=\"OFF\" \
		-DENABLE_CODECS_JP=\"OFF\" \
		-DENABLE_CODECS_KR=\"OFF\" \
		-DENABLE_CODECS_TW=\"OFF\" \
		-DENABLE_CTYPES=\"OFF\" \
		-DENABLE_CTYPES_TEST=\"OFF\" \
		-DENABLE_DECIMAL=\"OFF\" \
		-DENABLE_IPV6=\"OFF\" \
		-DENABLE_LOCALE=\"OFF\" \
		-DENABLE_MMAP=\"OFF\" \
		-DENABLE_MSI=\"OFF\" \
		-DENABLE_MSVCRT=\"OFF\" \
		-DENABLE_MULTIBYTECODEC=\"OFF\" \
		-DENABLE_MULTIPROCESSING=\"OFF\" \
		-DENABLE_OSSAUDIODEV=\"OFF\" \
		-DENABLE_OVERLAPPED=\"OFF\" \
		-DENABLE_PYEXPAT=\"OFF\" \
		-DENABLE_SCPROXY=\"OFF\" \
		-DENABLE_SELECT=\"OFF\" \
		-DENABLE_SOCKET=\"OFF\" \
		-DENABLE_SUBPROCESS=\"OFF\" \
		-DENABLE_TESTCAPI=\"OFF\" \
		-DENABLE_WINAPI=\"OFF\" \
		-DINSTALL_DEVELOPMENT=\"OFF\" \
		-DINSTALL_MANUAL=\"OFF\" \
		-DINSTALL_TEST=\"OFF\" \
		-DUSE_SYSTEM_BZip2=\"OFF\" \
		-DUSE_SYSTEM_Curses=\"OFF\" \
		-DUSE_SYSTEM_EXPAT=\"OFF\" \
		-DUSE_SYSTEM_DB=\"OFF\" \
		-DUSE_SYSTEM_GDBM=\"OFF\" \
		-DUSE_SYSTEM_OpenSSL=\"OFF\" \
		-DUSE_SYSTEM_READLINE=\"OFF\" \
		-DUSE_SYSTEM_SQLITE3=\"OFF\" \
		-DUSE_SYSTEM_TCL=\"OFF\" \
		-DUSE_SYSTEM_ZLIB=\"OFF\" \
		-DWITH_COMPUTED_GOTOS=\"OFF\" \
		-DWITH_DOC_STRINGS=\"OFF\" \
		-DWITH_TSC:BOOL=\"OFF\""
	build_a_project("Python", "External/Python", build_info, compiler_info, False, additional_options)

def build_libogg(build_info, compiler_info):
	build_a_project("libogg", "External/libogg", build_info, compiler_info)

def build_libvorbis(build_info, compiler_info):
	build_a_project("libvorbis", "External/libvorbis", build_info, compiler_info)

def build_freetype(build_info, compiler_info):
	build_a_project("freetype", "External/freetype", build_info, compiler_info)

def build_7z(build_info, compiler_info):
	build_a_project("7z", "External/7z", build_info, compiler_info, build_info.is_windows)

def build_UniversalDXSDK(build_info, compiler_info):
	build_a_project("UniversalDXSDK", "External/UniversalDXSDK", build_info, compiler_info)

def build_OpenALSDK(build_info, compiler_info):
	build_a_project("OpenALSDK", "External/OpenALSDK", build_info, compiler_info)

def build_rapidxml(build_info, compiler_info):
	build_a_project("rapidxml", "External/rapidxml", build_info, compiler_info)

def build_wpftoolkit(build_info, compiler_info):
	build_a_project("wpftoolkit", "External/wpftoolkit", build_info, compiler_info)

def build_android_native_app_glue(build_info, compiler_info):
	build_a_project("android_native_app_glue", "External/android_native_app_glue", build_info, compiler_info)

def build_assimp(build_info, compiler_info):
	build_a_project("assimp", "External/assimp", build_info, compiler_info)

def build_external_libs(build_info):
	for compiler_info in build_info.compilers:
		build_Boost(build_info, compiler_info)
		build_Python(build_info, compiler_info)
		build_7z(build_info, compiler_info)
		build_rapidxml(build_info, compiler_info)
		build_android_native_app_glue(build_info, compiler_info)

		if not build_info.is_windows_store:
			build_libogg(build_info, compiler_info)
			build_libvorbis(build_info, compiler_info)

		if build_info.is_dev_platform:
			build_freetype(build_info, compiler_info)			
			build_UniversalDXSDK(build_info, compiler_info)

			if ("win" == build_info.target_platform) and (compiler_info.arch != "arm"):
				build_OpenALSDK(build_info, compiler_info)

			build_assimp(build_info, compiler_info)

		if build_info.is_windows_desktop and ("x64" == compiler_info.arch) and ("vc" == build_info.compiler_name):
			build_wpftoolkit(build_info, compiler_info)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(bi)
