#!/bin/bash
# Generate decision trees with Weka and evaluate their effectiveness.

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"
PARSER="../../weka-json-parser/parsej48.py"
ACTION_ARFF=results/action.arff

attribute_select=false
discretize=false

# Loop until all parameters are used up
while [ "$1" != "" ]; do
    echo "parameter $1 set"

    case $1 in
        # -f | --file )           shift
        #                         filename=$1
        #                         ;;
        -as | --attribute-select ) attribute_select=true
                                ;;
        -ds | --discretize ) discretize=true
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
./$PARSER results/nearestall_weka.txt > /tmp/tree_leftright.json
./$PARSER results/firstsize_weka.txt > /tmp/tree_firstsize.json
echo "Training action tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_DATA -C 0.25 -M 128 | ./$PARSER > /tmp/tree_action.json


# Evaluate tree effectiveness.
echo "Evaluating..."
./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
            --first-size-tree=/tmp/tree_firstsize.json \
            --action-tree=/tmp/tree_action.json

# Evaluate the effectiveness given perfect classifications
# ./makeperfectaction.py > perfect.json
# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --first-size-tree=/tmp/tree_firstsize.json \
#     --action-tree=/tmp/tree_action.json --perfect-file=perfect.json

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --action-tree=/tmp/tree_action.json --perfect-file=perfect.json \
#     --first-size-tree=/tmp/tree_firstsize.json \
#     --specific-scene=landscape3.txt > test.R

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --action-tree=/tmp/tree_action.json \
#     --specific-scene=backyard.txt > test.R

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#             --action-tree=results/manual_tree.json

# ./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
#     --action-tree=results/manual_tree.json\
#     --specific-scene=landscape3.txt > test.R