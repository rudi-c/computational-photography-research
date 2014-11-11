#!/bin/bash
# Generate decision trees with Weka and evaluate their effectiveness.

JAR="/usr/share/java/weka-3.6.10.jar"

if [ ! -f $JAR ]; then
    echo "ERROR: Weka not found at "$JAR
    exit
fi

PARSER="parsej48.py"

WEKA_LEFTRIGHT="results/nearest3_weka.txt"
WEKA_ACTION="results/weka_out.txt"

# Parameters.
redirect=false
simulateerrors=false
lowlight=false
lowlightgauss=false
leaveout=""
useonly=""
plotfile=""

# Loop until all parameters are used up
while [ "$1" != "" ]; do
    echo "parameter $1 set"

    case $1 in
        -se | --simulateerrors | --simulate-errors ) simulateerrors=true ;;
        -ll | --lowlight | --low-light ) lowlight=true ;;
        -llg | --lowlightgauss | --low-light-gauss ) lowlightgauss=true ;;
        -lv | --leaveout ) shift
              redirect=true
              leaveout="--leave-out="$1
              useonly="--use-only="$1
              plotfile=$1
              echo "leaving out "$1 ;;
        * )                     usage
                                exit 1
    esac
    # Shift all the parameters down by one
    shift
done

# Parse weka output from txt to json.
echo "Parsing leftright tree..."
cat $WEKA_LEFTRIGHT | ./$PARSER > /tmp/tree_leftright.json
echo "Parsing action tree..."
cat $WEKA_ACTION | ./$PARSER > /tmp/tree_action.json

# Build arguments list.
arg1="--left-right-tree=/tmp/tree_leftright.json "
arg2="--action-tree=/tmp/tree_action.json "
treeargs=$arg1$arg2

# Optional arguments.
if [ "$simulateerrors" = true ]; then
    arg=" --backlash --noise"
    treeargs=$treeargs$arg
fi

if [ "$lowlight" = true ]; then
    arg=" --lowlight"
    treeargs=$treeargs$arg
fi

if [ "$lowlightgauss" = true ]; then
    if [ "$lowlight" = true ]; then
        echo "Warning : only one of (-ll/--lowlight/--low-light)"
        echo "                  and (-llg/--lowlightgauss/--low-light-gauss)"
        echo "          should be set."
    fi
    arg=" --lowlightgauss"
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
