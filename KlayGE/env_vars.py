#!/usr/bin/env python
#-*- coding: ascii -*-

from __future__ import print_function
from winreg import *
import os

klayge_path = os.path.split(os.path.abspath(__file__))[0]
print("Set %%KLAYGEHOME%% to %s" % klayge_path)

env = os.environ
env_key = r"SYSTEM\CurrentControlSet\Control\Session Manager\Environment"

def regkey(env_key, key_name, value):
	if key_name not in env:
		CreateKey(HKEY_LOCAL_MACHINE, env_key)

	k = OpenKey(HKEY_LOCAL_MACHINE, env_key, 0, KEY_SET_VALUE)
	SetValueEx(k, key_name, None, REG_SZ, value)
	FlushKey(k)
	CloseKey(k)

regkey(env_key, 'KLAYGEHOME', klayge_path)
