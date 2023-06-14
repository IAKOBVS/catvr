#!/bin/sh
scripts_dir=$HOME/.local/bin/scripts
if [ ! -d "$scripts_dir" ]; then
	echo "$scripts_dir does not exist!"
fi
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
	{
	bin=../bin/${file%.*}
	if test "$file" -nt "$bin"; then
		$compiler $@ "$file" -o "$bin" &&
		echo "$file successfuly compiled!" &&
		cp "$bin" "$scripts_dir/${bin##*/}" &&
		echo "$bin successfuly copied to $scripts_dir!"
	fi
	} &
done
wait
