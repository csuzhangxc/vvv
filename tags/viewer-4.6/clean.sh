#!/bin/tcsh -f
# unix clean script

echo cleaning libmini...
(cd mini; make clean)
echo cleaning viewer...
make clean
echo cleaning qtv3...
(cd qtv3; make clean)