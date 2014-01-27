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

# all at 10/10
./makefeatures.py --highest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highest2_filtered.arff

# ratio2Inverse_8 at 5/10, upTrend and ratio3Inverse_8 at 10/10, curving_1 and curving_2 at 1/10
./makefeatures.py --highest --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/highest3_filtered.arff

# all at 10/10
./makefeatures.py --highest --all-features upTrend ratio3Inverse_8 bracket > results/highestall_filtered.arff

# all at 10/10
./makefeatures.py --nearest ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/nearest2_filtered.arff

# all at 10/10
./makefeatures.py --nearest --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/nearest3_filtered.arff

# all at 10/10
./makefeatures.py --nearest --all-features upTrend ratio3Inverse_8 bracket > results/nearestall_filtered.arff

# all at 10/10
./makefeatures.py --high-and-near ratio2_8 ratio2Inverse_8 logRatio2Inverse_8 > results/highnear2_filtered.arff

# all at 10/10
./makefeatures.py --high-and-near --three-measures ratio2Inverse_8 upTrend ratio3Inverse_8 > results/highnear3_filtered.arff

# all at 10/10
./makefeatures.py --high-and-near --all-features upTrend ratio3Inverse_8 bracket > results/highnearall_filtered.arff