#!/bin/sh
std=--std=c99
flags='-Wall -Wextra -pedantic -Wshadow -Wstrict-prototypes -Wmissing-prototypes'
args="$std $flags"
main='
rgrep.c
'
scripts_dir=$HOME/.local/bin/scripts
if [ -f /usr/bin/gcc ]; then
	compiler=gcc
elif [ -f /usr/bin/clang ]; then
	compiler=clang
else
	echo 'gcc/clang not available'
	return 1
fi
mkdir -p bin
cd src || {
	echo "Can't cd to ./src"
	exit
}
for cfile in $(echo *.c); do
	{
		case "$main rfind.c" in
		*$cfile*) [ "$cfile" != grep.c ] && exit ;;
		esac
		base=${cfile%.*}
		if [ ! -f "$base.o" ] || test "$base.o" -ot "$cfile"; then
			$compiler $@ "$cfile" -c -o "$base.o" $args
			echo $compiler $@ "$cfile" -c -o "$base.o" $args
		fi
	} &
done
wait
for m in $main; do
	{
		$compiler $@ "$m" -o "../bin/${m%.*}" ./*.o $args
		echo $compiler $@ $m -o ${m%.*} ./*.o $args
	} &
done
{
	$compiler $@ rfind.c -o ../bin/rfind g_memmem.o librgrep.o $args
	echo $compiler $@ rfind.c -o ../bin/rfind $args
} &
wait
if [ ! -d "$scripts_dir" ]; then
	echo "$scripts_dir does not exist!"
	echo 'Set a directory in which to store the executable'
	exit
fi
cp ../bin/* "$scripts_dir"
echo
echo "Copied to $scripts_dir"
