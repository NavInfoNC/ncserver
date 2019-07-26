#!/usr/bin/python
import platform, os

def system(cmd):
	if os.system(cmd) != 0:
		raise "Failed to execute cmd:", cmd

if platform.system() == "Linux":

	if not os.path.isdir("tmp"):
		os.mkdir("tmp")
	os.chdir("tmp")

	system("cmake -G \"Unix Makefiles\" ..")
	system("make")
else:
	print "Only support Linux"
