#!/usr/bin/bash

echo $1
echo $2

for i in `ls $1`
do
	echo $i
	pygmentize -f html -O full -O style=fruity -o $2'/'$i.html $1'/'$i
done
