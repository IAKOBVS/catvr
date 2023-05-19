#!/bin/sh
file=jcatv
if [ -f /usr/bin/gcc ]; then
	compiler=gcc
elif [ -f /usr/bin/clang ]; then
	compiler=clang
else
	echo 'gcc/clang not available'
	return 1
fi
$compiler -O3 -flto bin/$file.c -o bin/$file && echo "$file successfuly compiled!"
