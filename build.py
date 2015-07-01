#! /usr/bin/python
import os
import subprocess

os.chdir("build")
subprocess.call("make", shell=True)
os.chdir("..")
subprocess.call("build/LLVMLang test.src", shell=True)
subprocess.call("clang++ -c lib.cpp -o lib.o", shell=True)
subprocess.call("llc -filetype=obj test.bc -o test.o", shell=True)
subprocess.call("clang++ lib.o test.o -o app", shell=True)
