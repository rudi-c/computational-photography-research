#!/bin/bash
# Generate decision trees with Weka and evaluate their effectiveness.

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"
PARSER="../../weka-json-parser/parsej48.py"
WEKA_OUT="results/weka_out.txt"
ACTION_ARFF=results/action.arff

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
        -as | --attribute-select ) attribute_select=true ;;
        -ds | --discretize ) discretize=true ;;
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
./simulate.py $leaveout > $ACTION_ARFF

# Make a copy of the data, which we will modify if filters used.
ACTION_DATA=/tmp/action_data.arff
cat $ACTION_ARFF > $ACTION_DATA

if [ "$discretize" = true ]; then
    echo "Performing discretization..."
    ACTION_FILTERED=/tmp/filtered_action.arff

    java -cp $CP weka.filters.supervised.attribute.Discretize \
        -R "first-last" \
        -c "last" \
        -b \
        -i $ACTION_DATA \
        -o $ACTION_FILTERED \
        -r $ACTION_DATA \
        -s $ACTION_FILTERED
    cat $ACTION_FILTERED > $ACTION_DATA
fi

if [ "$attribute_select" = true ]; then
    echo "Performing attribute selection..."
    ACTION_FILTERED=/tmp/filtered_action.arff

    java -cp $CP weka.filters.supervised.attribute.AttributeSelection \
        -E "weka.attributeSelection.CfsSubsetEval -M" \
        -S "weka.attributeSelection.BestFirst -D 1 -N 5" \
        -b \
        -i $ACTION_DATA \
        -o $ACTION_FILTERED \
        -r $ACTION_DATA \
        -s $ACTION_FILTERED
    cat $ACTION_FILTERED > $ACTION_DATA
fi


# Train trees with Weka
echo "Parsing first step trees..."
./$PARSER results/nearest3_weka.txt > /tmp/tree_leftright.json
./$PARSER results/firstsize_weka.txt > /tmp/tree_firstsize.json
echo "Training action tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_DATA -C 0.25 -M 512 > $WEKA_OUT
cat $WEKA_OUT | ./$PARSER > /tmp/tree_action.json

# Assign higher cost to certain classications. Doesn't work as hoped.
# java -cp $CP weka.classifiers.meta.CostSensitiveClassifier \
#     -t $ACTION_DATA \
#     -cost-matrix "[0.0 1.0 10.0; 1.0 0.0 5.0; 1.0 1.0 0.0]" \
#     -S 1 -W weka.classifiers.trees.J48 -- -U -M 512 -A  \
#     | ./$PARSER > /tmp/tree_action.json

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
