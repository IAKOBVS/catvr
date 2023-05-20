#!/bin/sh
file=catvr
if [ -f /usr/bin/gcc ]; then
	compiler=gcc
elif [ -f /usr/bin/clang ]; then
	compiler=clang
else
	echo 'gcc/clang not available'
	return 1
fi
$compiler -O3 -flto src/$file.c -o bin/$file && echo "$file successfuly compiled!"
