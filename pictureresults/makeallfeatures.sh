mkdir -p results
./makefeatures.py --highest > results/highest2.arff
./makefeatures.py --highest --three-measures > results/highest3.arff
./makefeatures.py --nearest > results/nearest2.arff
./makefeatures.py --nearest --three-measures > results/nearest3.arff
./makefeatures.py --high-and-near > results/highnear2.arff
./makefeatures.py --high-and-near --three-measures > results/highnear3.arff

# After running the output of the above in weka's feature selection
# Selected with CfsSubsetEval, BestFirst -D 1 -N 5

# all at 10/10
./makefeatures.py --highest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highest2_filtered.arff

# ratio2Inverse_8 at 5/10, upTrend at 10/10, ratio3Inverse_8 at 10/10, curving_1 and curving_2 at 1/10
./makefeatures.py --highest --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/highest3_filtered.arff

# ratio2_1 at 1/10, ratio2_8 at 10/10
./makefeatures.py --nearest ratio2_8 > results/nearest2_filtered.arff

# ratio2_1 at 1/10, rest at 10/10
./makefeatures.py --nearest --three-measures ratio2_8 ratio3_8 downTrend > results/nearest3_filtered.arff

# all at 10/10
./makefeatures.py --high-and-near ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highnear2_filtered.arff

# all at 10/10
./makefeatures.py --high-and-near --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/highnear3_filtered.arff