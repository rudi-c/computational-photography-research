#!/bin/bash
# Generate training data and induce decision trees with Weka to
# obtain a classifier which determines whether to search left or right
# (near focus or far focus).

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"
MIN_INSTANCES_PER_LEAF=512

function feature_select {
    local input=$1
    local output=$2
    java -cp $CP weka.filters.supervised.attribute.AttributeSelection \
        -E "weka.attributeSelection.CfsSubsetEval -M" \
        -S "weka.attributeSelection.BestFirst -D 1 -N 5" \
        -b \
        -i $input \
        -o $output \
        -r $input \
        -s $output
}

# There is actually nine sets of data that we generate and train, for every
# combination of [ two-measures, three-measures, all-features ] X
#                [ highest, nearest, high-and-near ]
#
# two-measures : Only features which use only two focus measures.
# three-measures : Only features which use only three focus measures.
# all-features : All features, including those who depend on knowledge of
#                the current lens position (we ended up finding that this
#                is not a safe assumption to make)
#
# highest : The correct classification is the direction of the highest peak.
# nearest : The correct classification is the direction of the nearest peak.
# high-and-near : The correct classification is some combination of height
#                 and distance.
#
# In the paper, we use the options three-measures, nearest

mkdir -p results
echo "Making features for classifier \"highest\""
./makeleftrightfeatures.py --dup-edges --highest > results/highest2.arff
./makeleftrightfeatures.py --dup-edges --highest --three-measures > results/highest3.arff
./makeleftrightfeatures.py --dup-edges --highest --all-features > results/highestall.arff

echo "Making features for classifier \"nearest\""
./makeleftrightfeatures.py --dup-edges --nearest > results/nearest2.arff
./makeleftrightfeatures.py --dup-edges --nearest --three-measures > results/nearest3.arff
./makeleftrightfeatures.py --dup-edges --nearest --all-features > results/nearestall.arff

echo "Making features for classifier \"highnear\""
./makeleftrightfeatures.py --dup-edges --high-and-near > results/highnear2.arff
./makeleftrightfeatures.py --dup-edges --high-and-near --three-measures > results/highnear3.arff
./makeleftrightfeatures.py --dup-edges --high-and-near --all-features > results/highnearall.arff

echo "Training left-right trees..."
for training_data in highest2 highest3 highestall \
                     nearest2 nearest3 nearestall \
                     highnear2 highnear3 highnearall
do
    echo $training_data
    data=results/${training_data}.arff
    filtered=results/${training_data}_filtered.arff
    feature_select $data $filtered
    java -cp $CP weka.classifiers.trees.J48 \
        -t $filtered -C 0.25 -M $MIN_INSTANCES_PER_LEAF > results/${training_data}_weka.txt
done
