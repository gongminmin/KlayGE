#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os, sys
from blib_util import *

def build_Boost(build_info, compiler_info):
	with_atomic = True
	with_chrono = True
	with_date_time = True
	with_filesystem = True
	with_program_options = True
	with_regex = True
	with_system = True
	with_thread = True
	if "vc" == build_info.compiler_name:
		if build_info.compiler_version >= 100:
			with_regex = False
		if build_info.compiler_version >= 110:
			with_atomic = False
			with_chrono = False
			with_date_time = False
			with_filesystem = False
			with_system = False
			with_thread = False
	if compiler_info.is_windows_runtime:
		with_filesystem = False
		with_program_options = False

	additional_options = ""
	if with_atomic:
		additional_options += " -DWITH_ATOMIC:BOOL=\"ON\""
	if with_chrono:
		additional_options += " -DWITH_CHRONO:BOOL=\"ON\""
	if with_date_time:
		additional_options += " -DWITH_DATE_TIME:BOOL=\"ON\""
	if with_filesystem:
		additional_options += " -DWITH_FILESYSTEM:BOOL=\"ON\""
	if with_program_options:
		additional_options += " -DWITH_PROGRAM_OPTIONS:BOOL=\"ON\""
	if with_regex:
		additional_options += " -DWITH_REGEX:BOOL=\"ON\""
	if with_system:
		additional_options += " -DWITH_SYSTEM:BOOL=\"ON\""
	if with_thread:
		additional_options += " -DWITH_THREAD:BOOL=\"ON\""
	build_a_project("boost", "External/boost", build_info, compiler_info, True, additional_options)

def build_Python(build_info, compiler_info):
	additional_options = "-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_HK:BOOL=\"ON\" -D BUILTIN_CODECS_ISO2022:BOOL=\"ON\" \
		-D BUILTIN_CODECS_CN:BOOL=\"ON\" -D BUILTIN_CODECS_JP:BOOL=\"ON\" -D BUILTIN_CODECS_KR:BOOL=\"ON\" -D BUILTIN_CODECS_TW:BOOL=\"ON\" \
		-D BUILTIN_COLLECTIONS:BOOL=\"ON\" -D BUILTIN_FUNCTOOLS:BOOL=\"ON\" -D BUILTIN_IO:BOOL=\"ON\" -D BUILTIN_ITERTOOLS:BOOL=\"ON\" \
		-D BUILTIN_LOCALE:BOOL=\"ON\" -D BUILTIN_MATH:BOOL=\"ON\" -D BUILTIN_MSI:BOOL=\"OFF\" -D BUILTIN_MULTIBYTECODEC:BOOL=\"ON\" \
		-D BUILTIN_OPERATOR:BOOL=\"ON\" -D BUILTIN_UNICODEDATA:BOOL=\"ON\" -D BUILTIN_MSVCRT:BOOL=\"OFF\" \
		-D BUILTIN_WINREG:BOOL=\"ON\" -D BUILTIN_SUBPROCESS:BOOL=\"OFF\" -D BUILTIN_MULTIPROCESSING:BOOL=\"OFF\" \
		-D BUILTIN_SELECT:BOOL=\"OFF\" -D BUILTIN_SOCKET:BOOL=\"OFF\" -D BUILTIN_MMAP:BOOL=\"OFF\" \
		-D ENABLE_AUDIOOP:BOOL=\"OFF\" -D ENABLE_OSSAUDIODEV:BOOL=\"OFF\" \
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
	build_a_project("7z", "External/7z", build_info, compiler_info, True)

def setup_DXSDK(build_info, compiler_info):
	build_a_project("DXSDK", "External/DXSDK", build_info, compiler_info)

def setup_OpenALSDK(build_info, compiler_info):
	build_a_project("OpenALSDK", "External/OpenALSDK", build_info, compiler_info)

def setup_Cg(build_info, compiler_info):
	build_a_project("Cg", "External/Cg", build_info, compiler_info)

def build_external_libs(build_info):
	import glob

	for compiler_info in build_info.compilers:
		platform_dir = "%s_%s" % (build_info.target_platform, compiler_info.arch)
		dst_dir = "KlayGE/bin/%s/" % platform_dir

		if not compiler_info.is_windows_runtime:
			print("\nBuilding boost...\n")
			build_Boost(build_info, compiler_info)

		if not compiler_info.is_windows_runtime:
			print("\nBuilding Python...\n")
			build_Python(build_info, compiler_info)

			if not build_info.prefer_static:
				if not os.path.exists("%sLib" % dst_dir):
					os.mkdir("%sLib" % dst_dir)
				copy_to_dst("External/Python/Lib/_collections_abc.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/_weakrefset.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/abc.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/codecs.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/copyreg.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/genericpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/io.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/macpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/ntpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/os.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/pathlib.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/posixpath.py", "%sLib/" % dst_dir)
				copy_to_dst("External/Python/Lib/stat.py", "%sLib/" % dst_dir)
				if not os.path.exists("%sLib/encodings" % dst_dir):
					os.mkdir("%sLib/encodings" % dst_dir)
				for fname in glob.iglob("External/Python/Lib/encodings/*.py"):
					copy_to_dst(fname, "%sLib/encodings/" % dst_dir)

		if not compiler_info.is_windows_runtime:
			print("\nBuilding libogg...\n")
			build_libogg(build_info, compiler_info)

		if not compiler_info.is_windows_runtime:
			print("\nBuilding libvorbis...\n")
			build_libvorbis(build_info, compiler_info)

		if (not compiler_info.is_windows_runtime) and (not compiler_info.is_android):
			print("\nBuilding freetype...\n")
			build_freetype(build_info, compiler_info)

		print("\nBuilding 7z...\n")
		build_7z(build_info, compiler_info)

		if (not compiler_info.is_windows_runtime) and (not compiler_info.is_android):
			if "win" == build_info.target_platform:
				print("\nSeting up DXSDK...\n")
				setup_DXSDK(build_info, compiler_info)

		if (not compiler_info.is_windows_runtime) and (not compiler_info.is_android):
			if ("win" == build_info.target_platform) and (compiler_info.arch != "arm"):
				print("\nSeting up OpenAL SDK...\n")
				setup_OpenALSDK(build_info, compiler_info)

		if (not compiler_info.is_windows_runtime) and (compiler_info.arch != "arm") and (not compiler_info.is_android):
			print("\nSeting up Cg...\n")
			setup_Cg(build_info, compiler_info)

if __name__ == "__main__":
	cfg = cfg_from_argv(sys.argv)
	bi = build_info(cfg.compiler, cfg.archs, cfg.cfg)

	build_external_libs(bi)
