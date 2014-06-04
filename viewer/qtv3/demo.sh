#!/bin/tcsh -f

set volume=Bucky.pvm
if ($1 != "") set volume=$1

set qtv3=./qtv3
if ($HOSTTYPE == "intel-pc") set qtv3=qtv3.app/Contents/MacOS/qtv3

$qtv3 --demo --fullscreen --gradmag --zoom=40 --maxidle=60 --maxvol=1024 --maxiso=256 $volume
