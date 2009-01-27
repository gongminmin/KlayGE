#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
import os
if ('nt' == os.name):
	from winreg import *

klayge_path = os.path.split(os.path.abspath(__file__))[0]
print('Set %%KLAYGEHOME%% to %s' % klayge_path)

klaygehome = 'KLAYGEHOME'

env = os.environ

if ('nt' == os.name):
	env_key = r'SYSTEM\CurrentControlSet\Control\Session Manager\Environment'

	def regkey(env_key, key_name, value):
		if key_name not in env:
			CreateKey(HKEY_LOCAL_MACHINE, env_key)

		k = OpenKey(HKEY_LOCAL_MACHINE, env_key, 0, KEY_SET_VALUE)
		SetValueEx(k, key_name, None, REG_SZ, value)
		FlushKey(k)
		CloseKey(k)

	regkey(env_key, klaygehome, klayge_path)
else:
	if klaygehome not in env:
		f = open('/etc/profile', 'a')
		f.seek(0, os.SEEK_END)
		f.write('export %s=%s' % (klaygehome, klayge_path))
		f.close()
