#!/bin/sh
cd navier_stokes_solver
cd src
./compile_forms
cd ..
cmake .
make -j 2
cd ../cloudnaca
./runme.sh 0 30 10 200 3
