#!/usr/bin/env python
#-*- coding: ascii -*-

# call 'winegcc' directly in XCode build script may fail, since '/usr/local/bin' is not added to PATH when running.
import sys, subprocess, os

input = sys.argv[1]
output = sys.argv[2]
params = sys.argv[3:]

my_env = os.environ.copy()
my_env["PATH"] = "/usr/local/bin:opt/local/sbin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:" + my_env["PATH"]
ret = subprocess.call(["wineg++", input, "-o", output] + params, stderr = subprocess.STDOUT, env = my_env)
sys.exit(ret)
