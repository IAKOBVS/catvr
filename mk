#!/bin/sh
scripts_dir=$HOME/.local/bin/scripts
if [ ! -d "$scripts_dir" ]; then
	echo "$scripts_dir does not exist!"
	echo 'Set a directory in which to store the executable'
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
for file in $(echo *.c); do
	{
		bin=../bin/${file%.*}
		headers=$(sed -n 's/^[[:space:]]*#[[:space:]]*include[[:space:]]\{0,\}"\([-\._A-Za-z]\{1,\}\)"$/\1/p' "$file")
		if test "$file" -nt "$bin"; then
			update=1
			grep -q -F config.h && ./src/max_fork
			grep -q -F pthread.h && set -- $@ -pthread
			grep -q -F omp.h && set -- $@ -fopenmp
		else
			for h in $headers; do
				if test "$h" -nt "$bin"; then
					update=1
					break
				fi
			done
			case $headers in *config.h*) ./src/max_fork ;; esac
			case $headers in *pthread.h*) set -- $@ -pthread ;; esac
			case $headers in *omp.h*) set -- $@ -fopenmp ;; esac
		fi
		if [ "$update" ]; then
			$compiler $@ "$file" -o "$bin" &&
				echo $compiler -Wall -Wextra $@ "$file" -o "$bin" &&
				/bin/cp -rf "$bin" "$scripts_dir/${bin##*/}"
		fi
	} &
done
wait
