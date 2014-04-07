#!/bin/bash
# Generate decision trees with Weka to evaluate whether the first step
# should go left or right, and coarse or fine.

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"

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
        -t $filtered -C 0.25 -M 6 > results/${training_data}_weka.txt
done

# # After running the output of the above in weka's feature selection
# # Selected with CfsSubsetEval, BestFirst -D 1 -N 5

# ##############################################################################
# # NEW FEATURES 2, NEW DATASET, RAW (NON-NORMALIZED), WITH STEP SIZE 1
# ##############################################################################

# # ratio2_4, diffRatioMin2_8, diffRatioMax2_3, diffRatioMax2_6, 
# # diffRatioMax2_7 at 2/10, diffRatioAvg2_3 at 5/10, diffRatioMin2_7 at 6/10,
# # ratio2_5 at 7/10, diffRatioAvg2_7, diffRatioMin2_6 at 9/10, rest at 10/10
# ./makefeatures.py --dup-edges --highest \
#     ratio2_5 diffRatioAvg2_3 diffRatioAvg2_7 diffRatioMin2_6 diffRatioMin2_7 \
#     ratio2_6 ratio2_7 ratio2_8 logRatio2_7 \
#     > results/highest2_filtered.arff

# # diffRatioAvg3_7 at 1/10, ratio3_6 at 3/10, ratio2_7 at 5/10
# # logRatio3_7 at 9/10, rest at 10/10
# ./makefeatures.py --dup-edges --highest --three-measures \
#     ratio2_7 logRatio3_7 \
#     downTrend ratio3_7 ratio3_8 diffRatioMax3_6 \
#     > results/highest3_filtered.arff

# # upTrend at 1/10, ratio3_6 at 5/10, ratio3_8, ratio2_7 at 9/10, rest at 1/10
# ./makefeatures.py --dup-edges --highest --all-features \
#     ratio2_7 ratio3_6 ratio3_8 \
#     downTrend ratio3_7 logRatio3_7 diffRatioMax3_6 bracket \
#     > results/highestall_filtered.arff

# # ratio2_8 at 2/10, diffRatioMax2_8 at 8/10, rest at 10/10
# ./makefeatures.py --dup-edges --nearest \
#     diffRatioMax2_8 ratio2_4 ratio2_6 ratio2_7 logRatio2_7 \
#     > results/nearest2_filtered.arff

# # all at 10/10
# ./makefeatures.py --dup-edges --nearest --three-measures \
#     downTrend upTrend ratio3_7 ratio3_8 logRatio3_7 diffRatioMax3_3 \
#     diffRatioMax3_6 > results/nearest3_filtered.arff 

# # all at 10/10
# ./makefeatures.py --dup-edges --nearest --all-features \
#     downTrend upTrend ratio3_7 ratio3_8 logRatio3_7 \
#     diffRatioMax3_3 diffRatioMax3_6 bracket > results/nearestall_filtered.arff

# # all at 10/10
# ./makefeatures.py --dup-edges --high-and-near \
#     ratio2_4 ratio2_6 ratio2_7 ratio2_8 logRatio2_7 \
#     > results/highnear2_filtered.arff

# # curving_3 at 3/10, upTrend, logRatio3_7, diffRatioMax3_3 at 6/10
# # rest at 10/10
# ./makefeatures.py --dup-edges --high-and-near --three-measures \
#     upTrend logRatio3_7 diffRatioMax3_3 \
#     downTrend ratio3_7 ratio3_8 diffRatioMax3_6 \
#     > results/highnear3_filtered.arff

# # ratio3_3 at 1/10, diffRatioMax3_3 at 9/10, rest at 10/10
# ./makefeatures.py --dup-edges --high-and-near --all-features  \
#     diffRatioMax3_3 \
#     downTrend upTrend ratio3_7 ratio3_8 logRatio3_7 diffRatioMax3_6 bracket \
#     > results/highnearall_filtered.arff

# ##############################################################################
# # NEW FEATURES 2, NEW DATASET, RAW (NON-NORMALIZED), WITH STEP SIZE 2
# ##############################################################################
# ./makefeatures.py --dup-edges --double-step --highest --three-measures > results/highest3.arff
# ./makefeatures.py --dup-edges --double-step --highest --all-features > results/highestall.arff
# ./makefeatures.py --dup-edges --double-step --nearest --three-measures > results/nearest3.arff
# ./makefeatures.py --dup-edges --double-step --nearest --all-features > results/nearestall.arff
# ./makefeatures.py --dup-edges --double-step --high-and-near --three-measures > results/highnear3.arff
# ./makefeatures.py --dup-edges --double-step --high-and-near --all-features > results/highnearall.arff

# # logRatio3_7 at 7/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --highest --three-measures \
#     downTrend upTrend ratio3_6 ratio3_7 curving_2 \
#     > results/highest3_filtered.arff
# # all at 10/10
# ./makefeatures.py --dup-edges --double-step --highest --all-features \
#     downTrend upTrend ratio3_6 ratio3_7 logRatio3_7 curving_2 bracket \
#     diffRatioMin3_6 bracket > results/highestall_filtered.arff
# # ratio3_2 at 1/10, ratio3_8, diffRatioAvg3_3 at 2/10, 
# # diffRatioMax3_3 at 4/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --nearest --three-measures \
#     downTrend upTrend ratio3_6 ratio3_7 logRatio3_7 curving_2 \
#     > results/nearest3_filtered.arff
# # diffRatioAvg3_3 at 3/10, diffRatioMax3_3 at 7/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --nearest --all-features \
#     diffRatioMax3_3 \
#     downTrend upTrend ratio3_6 ratio3_7 ratio3_8 logRatio3_7 \
#     curving_2 bracket > results/nearestall_filtered.arff
# # diffRatioMax3_3 at 3/10, logRatio3_7 at 5/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --nearest --three-measures \
#     logRatio3_7 downTrend upTrend ratio3_6 ratio3_7 curving_2 \
#     > results/highnear3_filtered.arff
# # ratio3_8, diffRatioMax3_3 at 7/10, logRatio3_7 at 8/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --nearest --all-features \
#     ratio3_8 logRatio3_7 diffRatioMax3_3 \
#     downTrend upTrend ratio3_6 ratio3_7 curving_2 bracket \
#     curving_2 bracket > results/highneartall_filtered.arff

##############################################################################
# NEW FEATURES, NEW DATASET, WITH STEP SIZE 1
##############################################################################

# # all at 10/10
# ./makefeatures.py --dup-edges --highest \
#     ratio2_10 diffRatioAvg2_5 diffRatioMin2_5 \
#     > results/highest2_filtered.arff

# # upTrend at 5/10, rest at 10/10
# ./makefeatures.py --dup-edges --highest --three-measures \
#     diffRatioMin2_5 downTrend upTrend ratio3_19 logRatio3_9 diffRatioAvg3_5 \
#     diffRatioMin3_5 > results/highest3_filtered.arff

# # upTrend at 6/10, rest at 10/10
# ./makefeatures.py --dup-edges --highest --all-features \
#     diffRatioMin2_5 downTrend upTrend ratio3_19 logRatio3_9 diffRatioAvg3_5 \
#     diffRatioMin3_5 bracket > results/highestall_filtered.arff

# # logRatio2_9 at 1/10, ratio2_20 at 8/10, ratio2_9 at 9/10, rest at 10/10
# ./makefeatures.py --dup-edges --nearest \
#     ratio2_20 ratio2_9 ratio2_10 ratio2_19 diffRatioAvg2_5 diffRatioMin2_5 \
#     diffRatioMax2_5 > results/nearest2_filtered.arff

# # all at 10/10
# ./makefeatures.py --dup-edges --nearest --three-measures \
#     diffRatioMin2_5 downTrend upTrend ratio3_9 ratio3_19 diffRatioAvg3_5 \
#     diffRatioMin3_5 > results/nearest3_filtered.arff

# # all at 10/10
# ./makefeatures.py --dup-edges --nearest --all-features \
#     diffRatioMin2_5 downTrend upTrend ratio3_9 ratio3_19 diffRatioAvg3_5 \
#     diffRatioMin3_5 bracket > results/nearestall_filtered.arff

# # ratio2_19, logRatio2_9 at 1/10, ratio2_20 at 4/10,
# # ratio2_9 at 7/10, diffRatioMax2_5 at 8/10
# # rest at 10/10
# ./makefeatures.py --dup-edges --high-and-near \
#     ratio2_9 diffRatioMax2_5 ratio2_10 diffRatioAvg2_5 diffRatioMin2_5 \
#     > results/highnear2_filtered.arff

# # diffRatioMin2_5, ratio3_19, diffRatioMin3_5 at 9/10
# # rest at 10/10
# ./makefeatures.py --dup-edges --high-and-near --three-measures \
#     diffRatioMin2_5 ratio3_19 diffRatioMin3_5 upTrend downTrend ratio3_9 \
#     diffRatioAvg3_5 > results/highnear3_filtered.arff

# # all at 10/10
# ./makefeatures.py --dup-edges --high-and-near --all-features  \
#     diffRatioMin2_5 downTrend upTrend ratio3_9 ratio3_19 diffRatioAvg3_5 \
#     diffRatioMin3_5 bracket > results/highnearall_filtered.arff

##############################################################################
# NEW FEATURES, NEW DATASET, WITH STEP SIZE 2
##############################################################################

# ./makefeatures.py --dup-edges --double-step --highest --three-measures > results/highest3.arff
# ./makefeatures.py --dup-edges --double-step --highest --all-features > results/highestall.arff
# ./makefeatures.py --dup-edges --double-step --nearest --three-measures > results/nearest3.arff
# ./makefeatures.py --dup-edges --double-step --nearest --all-features > results/nearestall.arff

# # ratio3_19 at 1/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --highest --three-measures \
#     downTrend upTrend logRatio3_9 diffRatioAvg3_5 diffRatioMin3_6 \
#     > results/highest3_filtered.arff
# # all at 10/10
# ./makefeatures.py --dup-edges --double-step --highest --all-features \
#     downTrend upTrend ratio3_19 logRatio3_9 diffRatioAvg3_5 \
#     diffRatioMin3_6 bracket > results/highestall_filtered.arff
# # all at 10/10
# ./makefeatures.py --dup-edges --double-step --nearest --three-measures \
#     downTrend upTrend logRatio3_9 diffRatioAvg3_5 diffRatioMin3_6 \
#     > results/nearest3_filtered.arff
# # diffRatioMin3_4, diffRatioMin3_5 at 1/10
# # ratio3_19 at 9/10, rest at 10/10
# ./makefeatures.py --dup-edges --double-step --nearest --all-features \
#     ratio3_19 downTrend upTrend logRatio3_9 diffRatioAvg3_5 diffRatioMin3_6 \
#     bracket > results/nearestall_filtered.arff



##############################################################################
# NEW FEATURES, OLD DATASET, WITH STEP SIZE 1
##############################################################################

# # diffRatioMin2_5, ratio2_19 at 10/10, diffRatioAvg2_5 at 7/10, ratio2_10 at 6/10
# ./makefeatures.py --highest diffRatioMin2_5 ratio2_19 diffRatioAvg2_5 ratio2_10 > results/highest2_filtered.arff

# # curving_2, diffRatioMin3_6 at 1/10,  diffRatioMin2_5 at 5/10, diffRatioMin3_5, ratio3_19 at 9/10, downTrend at 10/10
# ./makefeatures.py --highest --three-measures diffRatioMin2_5 downTrend ratio3_19 diffRatioMin3_5 > results/highest3_filtered.arff

# # all at 10/10
# ./makefeatures.py --highest --all-features diffRatioMin2_5 downTrend ratio3_19 downTrend diffRatioMin3_5 bracket > results/highestall_filtered.arff

# # all at 10/10
# ./makefeatures.py --nearest ratio2_10 ratio2_19 diffRatioMin2_5  > results/nearest2_filtered.arff

# # curving_2, diffRatioMin3_6, diffRatioMin2_5, logRatio3_19 at 1/10, ratio3_19 at 9/10, diffRatioMin3_5, downTrend at 10/10
# ./makefeatures.py --nearest --three-measures downTrend ratio3_19 diffRatioMin3_5 > results/nearest3_filtered.arff

# # all at 10/10
# ./makefeatures.py --nearest --all-features diffRatioMin2_5 downTrend ratio3_19 diffRatioMin3_5 diffRatioMin3_6 bracket > results/nearestall_filtered.arff

# # diffRatioAvg2_5 at 8/10, rest at 10/10
# ./makefeatures.py --high-and-near ratio2_10 ratio2_19 diffRatioAvg2_5 diffRatioMin2_5 > results/highnear2_filtered.arff

# # curving_1 at 2/10, ratio3_19 at 8/10, diffRatioMin2_5 at 2/10, rest at 10/10
# ./makefeatures.py --high-and-near --three-measures downTrend ratio3_19 diffRatioMin3_5 > results/highnear3_filtered.arff

# # ratio3_10 at 1/10, diffRatioMin2_5 at 7/10, ratio3_19 at 9/10, rest at 10/10
# ./makefeatures.py --high-and-near --all-features diffRatioMin2_5 downTrend ratio3_19 diffRatioMin3_5 bracket > results/highnearall_filtered.arff


##############################################################################
# NEW FEATURES, OLD DATASET, WITH STEP SIZE 2
##############################################################################

# # WITH STEP SIZE 2
# ./makefeatures.py --double-step --highest --three-measures > results/highest3.arff
# ./makefeatures.py --double-step --highest --all-features > results/highestall.arff
# ./makefeatures.py --double-step --nearest --three-measures > results/nearest3.arff
# ./makefeatures.py --double-step --nearest --all-features > results/nearestall.arff

# # diffRatioAvg3_5 at 1/10, upTrend at 2/10, diffRatioMin3_7, 
# # ratio3_17 at 4/10, diffRatioMin3_5 at 9/10, diffRatioMin3_6, downTrend at 10/10
# ./makefeatures.py --double-step --highest --three-measures diffRatioMin3_5 diffRatioMin3_6 downTrend > results/highest3_filtered.arff
# # diffRatioAvg3_5, ratio2_19 at 1/10, 
# # upTrend at 4/10, ratio3_17, diffRatioMin2_5, diffRatioMin3_7 at 5/10, 
# # diffRatioMin3_5 at 9/10, diffRatioMin3_6, downTrend, bracket at 10/10
# ./makefeatures.py --double-step --highest --all-features diffRatioMin3_5 diffRatioMin3_6 downTrend bracket > results/highestall_filtered.arff
# # diffRatioMax3_6 at 1/10, ratio3_17 at 2/10,
# # diffRatioMin3_6 at 7/10, diffRatioMin3_7, diffRatioMin3_5 at 8/10
# # upTrend, downTrend at 10/10
# ./makefeatures.py --double-step --nearest --three-measures diffRatioMin3_6 diffRatioMin3_7 diffRatioMin3_5 upTrend downTrend > results/nearest3_filtered.arff
# # diffRatioMax3_6 at 1/10, ratio3_19 at 1/10, ratio2_19 at 2/10, ratio3_10 at 3/10
# # ratio3_17, diffRatioMin2_6 at 5/10, diffRatioMin3_6 at 8/10, diffRatioMin3_5 at 9/10
# # upTrend, downTrend, diffRatioMin3_7, bracket at 10/10
# ./makefeatures.py --double-step --nearest --all-features upTrend downTrend diffRatioMin3_7 bracket diffRatioMin3_7 diffRatioMin3_5 > results/nearestall_filtered.arff

##############################################################################
# OLD FEATURES, OLD DATASET
##############################################################################

# # all at 10/10
# ./makefeatures.py --highest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highest2_filtered.arff

# # ratio2Inverse_8 at 5/10, downTrend and ratio3Inverse_8 at 10/10, curving_1 and curving_2 at 1/10
# ./makefeatures.py --highest --three-measures ratio2Inverse_8 downTrend ratio3Inverse_8 > results/highest3_filtered.arff

# # all at 10/10 except ratio3_8 at 6/10 ratio3_9 at 8/10
# ./makefeatures.py --highest --all-features ratio2Inverse_8 ratio3_8 ratio3_9 downTrend ratio3Inverse_8 bracket > results/highestall_filtered.arff

# # all at 10/10
# ./makefeatures.py --nearest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/nearest2_filtered.arff

# # all at 10/10
# ./makefeatures.py --nearest --three-measures ratio2Inverse_8 downTrend ratio3Inverse_8 > results/nearest3_filtered.arff

# # all at 10/10 except curving_1 at 3/10 and curving_2 at 1/10
# ./makefeatures.py --nearest --all-features downTrend ratio3Inverse_8 bracket > results/nearestall_filtered.arff

# # all at 10/10
# ./makefeatures.py --high-and-near ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highnear2_filtered.arff

# # all at 10/10
# ./makefeatures.py --high-and-near --three-measures ratio2Inverse_8 downTrend ratio3Inverse_8 > results/highnear3_filtered.arff

# # all at 10/10 except curving_1 at 2/10
# ./makefeatures.py --high-and-near --all-features downTrend ratio3Inverse_8 bracket > results/highnearall_filtered.arff