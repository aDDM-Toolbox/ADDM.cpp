#!/bin/bash
set -x

VERSION=$1
case $VERSION in
    1)
        SUFFIX=""
        SS2=0.0001
        SS05=0.001
        LABEL_POS=0.0085
        ;;
    2)
        SUFFIX="_fixed"
        SS2=0.0005
        SS05=0.0005
        LABEL_POS=0.0085
        ;;
    3)
        SUFFIX="_fixed_st"
        SS2=0.0001
        SS05=0.0001
        LABEL_POS=0.0044
        ;;
    4) 
        SUFFIX=""
        SS2=0.0001
        SS05=0.0001
        LABEL_POS=0.0044
        ;;
    *)
        echo "Invalid version provided."
        exit 1
        ;;
esac

python3 process.py run_4condi $VERSION $SUFFIX
python3 process.py run_2 $VERSION $SUFFIX
python3 process.py run_05 $VERSION $SUFFIX
python3 num_step_sizes.py $VERSION $SS2 $SS05 $SUFFIX 
python3 compare_ratios.py d $VERSION $SUFFIX
python3 compare_ratios.py sigma $VERSION $SUFFIX
python3 compare_ratios.py theta $VERSION $SUFFIX
python3 param_scatter.py $VERSION $LABEL_POS $SUFFIX