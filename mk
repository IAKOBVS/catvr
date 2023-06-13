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
mkdir -p bin
cd src || return
./max_fork
for file in $(echo *.c); do
	$compiler -O3 -flto $file -o ../bin/${file%.*} && echo "$file successfuly compiled!" &
done
wait
