#!/bin/bash
set -x

VERSION=3
SUFFIX="_fixed_st"
SS2=0.0001
SS05=0.0001

python3 process.py run_4condi $VERSION $SUFFIX
python3 process.py run_2 $VERSION $SUFFIX
python3 process.py run_05 $VERSION $SUFFIX
python3 num_step_sizes.py $VERSION $SUFFIX $SS2 $SS05
python3 compare_ratios.py d $VERSION $SUFFIX
python3 compare_ratios.py sigma $VERSION $SUFFIX
python3 compare_ratios.py theta $VERSION $SUFFIX
python3 param_scatter.py $VERSION $SUFFIX