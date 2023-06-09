#!/bin/sh
std=--std=c99
flags='-Wall -Wextra -pedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes'
args="$std $flags"
main=rgrep.c
scripts_dir=$HOME/.local/bin/scripts
if [ -f /usr/bin/gcc ]; then
	compiler=gcc
elif [ -f /usr/bin/clang ]; then
	compiler=clang
else
	echo 'gcc/clang not available'
	exit 1
fi
mkdir -p ./bin
if ! cd src; then
	echo "Can't cd to ./src"
	exit
fi
compile_echo()
{
	$*
	echo "$@"
}
for cfile in $(find . -type f -name '*.c' ! -name "$main"); do
	{
		base=${cfile%.*}
		if [ ! -f "$base.o" ] || test "$base.o" -ot "$cfile"; then
			compile_echo "$compiler $* $args $cfile -c"
		fi
	} &
done
wait
compile_echo "$compiler $* $args $main -o ../bin/${main%.*}" ./*.o
if [ ! -d "$scripts_dir" ]; then
	echo "$scripts_dir does not exist!"
	echo 'Set a directory in which to store the executable'
	exit
fi
cp ../bin/* "$scripts_dir"
echo
echo "Copied to $scripts_dir"
