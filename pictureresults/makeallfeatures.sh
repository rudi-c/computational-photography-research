mkdir -p results
./makefeatures.py --highest > results/highest2.arff
./makefeatures.py --highest --three-measures > results/highest3.arff
./makefeatures.py --highest --all-features > results/highestall.arff
./makefeatures.py --nearest > results/nearest2.arff
./makefeatures.py --nearest --three-measures > results/nearest3.arff
./makefeatures.py --nearest --all-features > results/nearestall.arff
./makefeatures.py --high-and-near > results/highnear2.arff
./makefeatures.py --high-and-near --three-measures > results/highnear3.arff
./makefeatures.py --high-and-near --all-features > results/highnearall.arff

# After running the output of the above in weka's feature selection
# Selected with CfsSubsetEval, BestFirst -D 1 -N 5

# diffRatioMin2_5, ratio2_19 at 10/10, diffRatioAvg2_5 at 7/10, ratio2_10 at 6/10
./makefeatures.py --highest diffRatioMin2_5 ratio2_19 diffRatioAvg2_5 ratio2_10 > results/highest2_filtered.arff

# curving_2, diffRatioMin3_6 at 1/10,  diffRatioMin2_5 at 5/10, diffRatioMin3_5, ratio3_19 at 9/10, upTrend at 10/10
./makefeatures.py --highest --three-measures diffRatioMin2_5 upTrend ratio3_19 diffRatioMin3_5 > results/highest3_filtered.arff

# all at 10/10
./makefeatures.py --highest --all-features diffRatioMin2_5 upTrend ratio3_19 upTrend diffRatioMin3_5 bracket > results/highestall_filtered.arff

# all at 10/10
./makefeatures.py --nearest ratio2_10 ratio2_19 diffRatioMin2_5  > results/nearest2_filtered.arff

# curving_2, diffRatioMin3_6, diffRatioMin2_5, logRatio3_19 at 1/10, ratio3_19 at 9/10, diffRatioMin3_5, upTrend at 10/10
./makefeatures.py --nearest --three-measures upTrend ratio3_19 diffRatioMin3_5 > results/nearest3_filtered.arff

# all at 10/10
./makefeatures.py --nearest --all-features diffRatioMin2_5 upTrend ratio3_19 diffRatioMin3_5 diffRatioMin3_6 bracket > results/nearestall_filtered.arff

# diffRatioAvg2_5 at 8/10, rest at 10/10
./makefeatures.py --high-and-near ratio2_10 ratio2_19 diffRatioAvg2_5 diffRatioMin2_5 > results/highnear2_filtered.arff

# curving_1 at 2/10, ratio3_19 at 8/10, diffRatioMin2_5 at 2/10, rest at 10/10
./makefeatures.py --high-and-near --three-measures upTrend ratio3_19 diffRatioMin3_5 > results/highnear3_filtered.arff

# ratio3_10 at 1/10, diffRatioMin2_5 at 7/10, ratio3_19 at 9/10, rest at 10/10
./makefeatures.py --high-and-near --all-features diffRatioMin2_5 upTrend ratio3_19 diffRatioMin3_5 bracket > results/highnearall_filtered.arff


# WITH OLD FEATURES

# # all at 10/10
# ./makefeatures.py --highest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highest2_filtered.arff

# # ratio2Inverse_8 at 5/10, upTrend and ratio3Inverse_8 at 10/10, curving_1 and curving_2 at 1/10
# ./makefeatures.py --highest --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/highest3_filtered.arff

# # all at 10/10 except ratio3_8 at 6/10 ratio3_9 at 8/10
# ./makefeatures.py --highest --all-features ratio2Inverse_8 ratio3_8 ratio3_9 upTrend ratio3Inverse_8 bracket > results/highestall_filtered.arff

# # all at 10/10
# ./makefeatures.py --nearest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/nearest2_filtered.arff

# # all at 10/10
# ./makefeatures.py --nearest --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/nearest3_filtered.arff

# # all at 10/10 except curving_1 at 3/10 and curving_2 at 1/10
# ./makefeatures.py --nearest --all-features upTrend ratio3Inverse_8 bracket > results/nearestall_filtered.arff

# # all at 10/10
# ./makefeatures.py --high-and-near ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highnear2_filtered.arff

# # all at 10/10
# ./makefeatures.py --high-and-near --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/highnear3_filtered.arff

# # all at 10/10 except curving_1 at 2/10
# ./makefeatures.py --high-and-near --all-features upTrend ratio3Inverse_8 bracket > results/highnearall_filtered.arff