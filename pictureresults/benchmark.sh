#!/bin/bash
# Generate decision trees with Weka and evaluate their effectiveness.

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"
PARSER="../../weka-json-parser/parse-j48.py"
LEFT_RIGHT_ARFF=results/nearestall_filtered.arff
ACTION_ARFF=results/action.arff

attribute_select=false

# By default, the data with filtered features is the same unless we
# pass in the command-line option --attribute-select
ACTION_ARFF_FILTERED=$ACTION_ARFF


# Loop until all parameters are used up
while [ "$1" != "" ]; do
    echo "parameter $1 set"

    case $1 in
        # -f | --file )           shift
        #                         filename=$1
        #                         ;;
        -as | --attribute-select ) attribute_select=true
                                ;;
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
./simulate.py > $ACTION_ARFF

# Perform attribute selection if needed.
if [ "$attribute_select" = true ]; then
    echo "Performing attribute selection..."
    ACTION_ARFF_FILTERED=/tmp/filtered_action.arff

    java -cp $CP weka.filters.supervised.attribute.AttributeSelection \
        -E "weka.attributeSelection.CfsSubsetEval -M" \
        -S "weka.attributeSelection.BestFirst -D 1 -N 5" \
        -b \
        -i $ACTION_ARFF \
        -o $ACTION_ARFF_FILTERED \
        -r $ACTION_ARFF \
        -s $ACTION_ARFF_FILTERED
fi


# Train trees with Weka
echo "Training left-right tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $LEFT_RIGHT_ARFF -C 0.25 -M 6 | ./$PARSER > /tmp/tree_leftright.json
echo "Training action tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_ARFF_FILTERED -C 0.25 -M 128 | ./$PARSER > /tmp/tree_action.json


# Evaluate tree effectiveness.
echo "Evaluating..."
./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
            --action-tree=/tmp/tree_action.json

# Evaluate the effectiveness given perfect classifications
# ./makeperfectaction.py > perfect.json
# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --action-tree=/tmp/tree_action.json --perfect-file=perfect.json

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --action-tree=/tmp/tree_action.json --perfect-file=perfect.json \
#     --specific-scene=landscape3.txt > test.R

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --action-tree=/tmp/tree_action.json \
#     --specific-scene=backyard.txt > test.R