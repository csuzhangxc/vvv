#!/bin/csh -f

set arg=$1
if ($arg == "") exit
if ($arg:e != "pvm") exit

set dir = $arg:h
if ($dir == $arg) set dir=.
set file = $arg:t

./pvm2web $dir/$file

cd $dir/*$file:r
foreach image (*.pgm)
   convert $image $image:r.png
   rm -f $image
end
