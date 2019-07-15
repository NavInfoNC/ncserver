#!/usr/bin/python
import platform, os

def system(cmd):
	if os.system(cmd) != 0:
		raise "Failed to execute cmd:", cmd

if platform.system() == "Windows":
	system("mbmake.py build -p x64")
	system("mbmake.py build -p x64 --release")
elif platform.system() == "Linux":

	if not os.path.isdir("tmp"):
		os.mkdir("tmp")
	os.chdir("tmp")

	system("cmake -G \"Unix Makefiles\" ..")
	system("make")

