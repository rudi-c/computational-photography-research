#!/bin/bash
# Generate training data and induce decision trees with Weka to
# obtain a classifier which determines state transition (continue,
# backtrack, turn to peak/success).

# Optional parameters :
# -as / --attribute-select  to perform attribute/feature selection
# -ds / --discretize        to discretize the numerical attributes

JAR="/usr/share/java/weka-3.6.10.jar"

if [ ! -f $JAR ]; then
    echo "ERROR: Weka not found at "$JAR
    exit
fi

CP="$CLASSPATH:"$JAR

WEKA_OUT="results/weka_out.txt"
ACTION_ARFF="results/action.arff"
MIN_INSTANCES_PER_LEAF=512

# Parameters.
attribute_select=false
discretize=false
redirect=false
leaveout=""

# Loop until all parameters are used up
while [ "$1" != "" ]; do
    echo "parameter $1 set"

    case $1 in
        -as | --attribute-select ) attribute_select=true ;;
        -ds | --discretize ) discretize=true ;;
        -lv | --leaveout ) shift
              redirect=true
              leaveout="--leave-out="$1
              echo "leaving out "$1 ;;
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

# Optional parameters.
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
echo "Training action tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_DATA -C 0.25 -M $MIN_INSTANCES_PER_LEAF > $WEKA_OUT
