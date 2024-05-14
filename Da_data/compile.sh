#!/bin/bash
set -x

g++ run_2.cpp -laddm -o run2
g++ run_4condi.cpp -laddm -o run4condi
g++ run_05.cpp -laddm -o run05
g++ run_best_fit.cpp -laddm -o run_best_fit_d_adj