#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os
if ('nt' == os.name):
	try:
		from winreg import *
	except:
		from _winreg import *

	try:
		import win32gui
		import win32con
		has_pywin32 = True
	except:
		has_pywin32 = False

klayge_path = os.path.split(os.path.abspath(__file__))[0]
if ('nt' == os.name):
	klayge_bin_path = "%s\\bin" % klayge_path
else:
	klayge_bin_path = "%s/bin" % klayge_path

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

		if has_pywin32:
			win32gui.SendMessageTimeout(win32con.HWND_BROADCAST, win32con.WM_SETTINGCHANGE, 0,
				"Environment", win32con.SMTO_ABORTIFHUNG, 5000);

else:
	f = open('/etc/profile', 'a')
	f.seek(0, os.SEEK_END)

	if klayge_home not in env:
		f.write('\n')
		f.write('export %s=%s' % (klayge_home, klayge_path))
	if klayge_bin_path.lower() not in env["PATH"].lower():
		f.write('\n')
		f.write('export PATH=$PATH:$%s/bin' % klayge_home)
	if (os.getenv("LD_LIBRARY_PATH") == None) or (klayge_bin_path.lower() not in env["LD_LIBRARY_PATH"].lower()):
		f.write('\n')
		f.write('export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$%s/bin' % klayge_home)

	f.close()

print('Set %%KLAYGE_HOME%% to %s' % klayge_path)
print('Add %KLAYGE_HOME%\\bin to %PATH%')

if not has_pywin32:
	print("WM_SETTINGCHANGE can't be delivered because PyWin32 is not installed. Please reboot your computer to update the environment variables.")

print("Please press any key to exit...")

try:
	raw_input()
except:
	input()
