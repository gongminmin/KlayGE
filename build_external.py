#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_Boost(build_info, compiler_info):
	with_filesystem = True
	with_system = True
	if compiler_info.is_dev_platform:
		with_program_options = True
		with_regex = True
		with_test = True
	else:
		with_program_options = False
		with_regex = False
		with_test = False
	if "vc" == build_info.compiler_name:
		with_filesystem = False
		with_regex = False
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
	additional_options += " -DWITH_REGEX:BOOL="
	if with_regex:
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
	build_a_project("boost", "External/boost", build_info, compiler_info, compiler_info.is_windows and need_install, additional_options)

def build_Python(build_info, compiler_info):
	additional_options = "-D BUILTIN_COLLECTIONS:BOOL=\"ON\" -D BUILTIN_FUNCTOOLS:BOOL=\"ON\" -D BUILTIN_IO:BOOL=\"ON\" \
		-D BUILTIN_ITERTOOLS:BOOL=\"ON\" -D BUILTIN_MATH:BOOL=\"ON\" -D BUILTIN_MSI:BOOL=\"OFF\" \
		-D BUILTIN_OPERATOR:BOOL=\"ON\" -D BUILTIN_UNICODEDATA:BOOL=\"ON\" -D BUILTIN_MSVCRT:BOOL=\"OFF\" \
		-D BUILTIN_WINREG:BOOL=\"ON\" -D BUILTIN_SUBPROCESS:BOOL=\"OFF\" \
		-D BUILTIN_SELECT:BOOL=\"OFF\" -D BUILTIN_SOCKET:BOOL=\"OFF\" -D BUILTIN_MMAP:BOOL=\"OFF\" \
		-D ENABLE_AUDIOOP:BOOL=\"OFF\" -D ENABLE_CODECS_CN:BOOL=\"OFF\" -D ENABLE_CODECS_HK:BOOL=\"OFF\" -D ENABLE_CODECS_ISO2022:BOOL=\"OFF\" \
		-D ENABLE_CODECS_JP:BOOL=\"OFF\" -D ENABLE_CODECS_KR:BOOL=\"OFF\" -D ENABLE_CODECS_TW:BOOL=\"OFF\" \
		-D ENABLE_LOCALE:BOOL=\"OFF\" -D ENABLE_MULTIBYTECODEC:BOOL=\"OFF\" -D ENABLE_MULTIPROCESSING:BOOL=\"OFF\" \
		-D ENABLE_OSSAUDIODEV:BOOL=\"OFF\" -D ENABLE_SCPROXY:BOOL=\"OFF\" \
		-D USE_SYSTEM_Curses:BOOL=\"OFF\" -D USE_SYSTEM_EXPAT:BOOL=\"OFF\" -D USE_SYSTEM_DB:BOOL=\"OFF\" -D USE_SYSTEM_GDBM:BOOL=\"OFF\" \
		-D USE_SYSTEM_OpenSSL:BOOL=\"OFF\" -D USE_SYSTEM_READLINE:BOOL=\"OFF\" -D USE_SYSTEM_SQLITE3:BOOL=\"OFF\" -D USE_SYSTEM_TCL:BOOL=\"OFF\" \
		-D USE_SYSTEM_ZLIB:BOOL=\"OFF\""
	build_a_project("Python", "External/Python", build_info, compiler_info, False, additional_options)

def build_libogg(build_info, compiler_info):
	build_a_project("libogg", "External/libogg", build_info, compiler_info)

def build_libvorbis(build_info, compiler_info):
	build_a_project("libvorbis", "External/libvorbis", build_info, compiler_info)

def build_freetype(build_info, compiler_info):
	build_a_project("freetype", "External/freetype", build_info, compiler_info)

def build_7z(build_info, compiler_info):
	build_a_project("7z", "External/7z", build_info, compiler_info, compiler_info.is_windows)

def setup_DXSDK(build_info, compiler_info):
	build_a_project("DXSDK", "External/DXSDK", build_info, compiler_info)

def setup_OpenALSDK(build_info, compiler_info):
	build_a_project("OpenALSDK", "External/OpenALSDK", build_info, compiler_info)

def setup_rapidxml(build_info, compiler_info):
	build_a_project("rapidxml", "External/rapidxml", build_info, compiler_info)

def setup_wpftoolkit(build_info, compiler_info):
	build_a_project("wpftoolkit", "External/wpftoolkit", build_info, compiler_info)

def setup_android_native_app_glue(build_info, compiler_info):
	build_a_project("android_native_app_glue", "External/android_native_app_glue", build_info, compiler_info)

def build_assimp(build_info, compiler_info):
	build_a_project("assimp", "External/assimp", build_info, compiler_info)

def build_external_libs(build_info):
	for compiler_info in build_info.compilers:
		platform_dir = "%s_%s" % (build_info.target_platform, compiler_info.arch)
		dst_dir = "KlayGE/bin/%s/" % platform_dir

		print("\nBuilding boost...\n")
		build_Boost(build_info, compiler_info)

		print("\nBuilding Python...\n")
		build_Python(build_info, compiler_info)

		print("\nBuilding 7z...\n")
		build_7z(build_info, compiler_info)

		print("\nSeting up rapidxml...\n")
		setup_rapidxml(build_info, compiler_info)

		print("\nSeting up android_native_app_glue...\n")
		setup_android_native_app_glue(build_info, compiler_info)

		if not compiler_info.is_windows_runtime:
			print("\nBuilding libogg...\n")
			build_libogg(build_info, compiler_info)

		if not compiler_info.is_windows_runtime:
			print("\nBuilding libvorbis...\n")
			build_libvorbis(build_info, compiler_info)

		if compiler_info.is_dev_platform:
			print("\nBuilding freetype...\n")
			build_freetype(build_info, compiler_info)
			
			print("\nSeting up DXSDK...\n")
			setup_DXSDK(build_info, compiler_info)

			if ("win" == build_info.target_platform) and (compiler_info.arch != "arm"):
				print("\nSeting up OpenAL SDK...\n")
				setup_OpenALSDK(build_info, compiler_info)

			print("\nBuilding assimp...\n")
			build_assimp(build_info, compiler_info)

		if compiler_info.is_windows_desktop and ("x64" == compiler_info.arch) and ("vc" == build_info.compiler_name):
			print("\nSeting up wpftoolkit...\n")
			setup_wpftoolkit(build_info, compiler_info)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(bi)
