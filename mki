#!/bin/sh
scripts_dir=$HOME/.local/bin/scripts
if [ ! -d $scripts_dir ]; then
	echo "$scripts_dir does not exist!"
	return
fi
./mk
cp bin/* $scripts_dir && echo "Successfuly copied to $scripts_dir"
