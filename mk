#!/bin/sh
file=jcatv
if [ -f $file ] && [ -f $file.c ]; then
	test $file.c -nt $file || return
fi
if [ -f /usr/bin/gcc ]; then
	compiler=gcc
elif [ -f /usr/bin/clang ]; then
	compiler=clang
else
	echo 'gcc/clang not available'
	return 1
fi
$compiler -O3 -flto $file.c -o $file
