#!/bin/tcsh -f
# unix install script

echo compiling libmini...
(cd mini; cmake . && make -j 4)
if ($? != 0) exit($?)
echo compiling libvolren...
cmake . && make -j 4
if ($? != 0) exit($?)
echo compiling qtv3...
(cd qtv3; cmake . && make -j 4)
if ($? != 0) exit($?)
echo installing qtv3...
echo enter root password when prompted
(cd qtv3; sudo make install)
if ($? != 0) exit($?)
