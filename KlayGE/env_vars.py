#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os
if ('nt' == os.name):
	try:
		from winreg import *
	except:
		from _winreg import *

klayge_path = os.path.split(os.path.abspath(__file__))[0]
klayge_bin_path = "%s\bin" % klayge_path

klayge_home = 'KLAYGE_HOME'

env = os.environ

if ('nt' == os.name):
	env_key = r'SYSTEM\CurrentControlSet\Control\Session Manager\Environment'

	if klayge_home not in env:
		CreateKey(HKEY_LOCAL_MACHINE, env_key)

	k = OpenKey(HKEY_LOCAL_MACHINE, env_key, 0, KEY_SET_VALUE)
	SetValueEx(k, klayge_home, None, REG_SZ, klayge_path)
	FlushKey(k)
	CloseKey(k)

	if klayge_bin_path.lower() not in env["path"].lower():
		k = OpenKey(HKEY_LOCAL_MACHINE, env_key, 0, KEY_SET_VALUE)
		cur_path = env["path"]
		if (cur_path[-1] != ";"):
			cur_path += ";"
		SetValueEx(k, "path", None, REG_SZ, "%s%%%s%%\\bin" % (cur_path, klayge_home))
		FlushKey(k)
		CloseKey(k)

else:
	if klayge_home not in env:
		f = open('/etc/profile', 'a')
		f.seek(0, os.SEEK_END)
		f.write('export %s=%s' % (klayge_home, klayge_path))
		f.close()

print('Set %%KLAYGE_HOME%% to %s' % klayge_path)
print('Add %KLAYGE_HOME%\\bin to %PATH%')
