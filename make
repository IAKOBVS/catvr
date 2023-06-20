#!/bin/sh
main='
rgrep.c
rfind.c
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
		case $main in
		*$cfile*) exit ;;
		esac
		base=${cfile%.*}
		if [ ! -f "$base.o" ] || test "$base.o" -ot "$cfile"; then
			$compiler -Wall -Wextra $@ "$cfile" -c -o "$base.o"
			echo $compiler $@ "$cfile" -c -o "$base.o"
		fi
	} &
done
wait
for m in $main; do
	{
		$compiler -Wall -Wextra $@ "$m" -o "../bin/${m%.*}" ./*.o
		echo "$compiler $@ $m -o ${m%.*} -Wall -Wextra" ./*.o
	} &
done
wait
if [ ! -d "$scripts_dir" ]; then
	echo "$scripts_dir does not exist!"
	echo 'Set a directory in which to store the executable'
	exit
fi
cp ../bin/* "$scripts_dir"
