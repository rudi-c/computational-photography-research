#!/bin/bash
# Generate decision trees with Weka and evaluate their effectiveness.

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"
PARSER="../../weka-json-parser/parsej48.py"
ACTION_ARFF_UP=results/action_up.arff
ACTION_ARFF_DOWN=results/action_down.arff

attribute_select=false
discretize=false
redirect=false
backlash=false
leaveout=""
useonly=""
plotfile=""

# Loop until all parameters are used up
while [ "$1" != "" ]; do
    echo "parameter $1 set"

    case $1 in
        # -f | --file )           shift
        #                         filename=$1
        #                         ;;
        -bl | --backlash ) backlash=true ;;
        -lv | --leaveout ) shift
              redirect=true
              leaveout="--leave-out="$1
              useonly="--use-only="$1
              plotfile=$1
              echo "leaving out "$1 ;;
        # -h | --help )           usage
        #                         exit
        #                         ;;
        * )                     usage
                                exit 1
    esac
    # Shift all the parameters down by one
    shift
done

# Generate instances to train with.
echo "Simulating..."
./simulate.py $leaveout --downslope-only > $ACTION_ARFF_DOWN
./simulate.py $leaveout --upslope-only > $ACTION_ARFF_UP

# Train trees with Weka
echo "Parsing first step trees..."
./$PARSER results/nearest3_weka.txt > /tmp/tree_leftright.json
./$PARSER results/firstsize_weka.txt > /tmp/tree_firstsize.json
echo "Training action tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_ARFF_UP -C 0.25 -M 256 | ./$PARSER > /tmp/tree_action_up.json
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_ARFF_DOWN -C 0.25 -M 256 | ./$PARSER > /tmp/tree_action_down.json

up=`cat /tmp/tree_action_up.json`
down=`cat /tmp/tree_action_down.json`
echo "[\"regression_slope_up\", [[\"<=\", 0, $down], [\">\", 0, $up]]]" > /tmp/tree_action.json

arg1="--left-right-tree=/tmp/tree_leftright.json "
arg2="--first-size-tree=/tmp/tree_firstsize.json "
arg3="--action-tree=/tmp/tree_action.json "
treeargs=$arg1$arg2$arg3

if [ "$backlash" = true ]; then
    arg=" --backlash"
    treeargs=$treeargs$arg
fi

# Evaluate tree effectiveness.
echo "Evaluating..."
mkdir -p simulations
if [ "$redirect" = true ]; then
    # This is for leave-on-out cross validation.
    ./benchmark.py $treeargs $useonly >> results.txt
    ./benchmark.py $treeargs \
                --specific-scene=$plotfile > simulations/$plotfile.R
else
    ./benchmark.py $treeargs
fi

# Evaluate the effectiveness given perfect classifications
# ./makeperfectaction.py > perfect.json
# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --first-size-tree=/tmp/tree_firstsize.json \
#     --action-tree=/tmp/tree_action.json --perfect-file=perfect.json

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --first-size-tree=/tmp/tree_firstsize.json \
#     --action-tree=/tmp/tree_action.json --perfect-file=perfect.json \
#     --specific-scene=landscape3.txt > test.R

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --first-size-tree=/tmp/tree_firstsize.json \
#     --action-tree=/tmp/tree_action.json \
#     --specific-scene=backyard.txt > test.R

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --first-size-tree=/tmp/tree_firstsize.json \
#     --action-tree=/tmp/tree_action.json
