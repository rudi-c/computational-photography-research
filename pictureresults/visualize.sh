PARSER="../../weka-json-parser/parsej48.py"

left_right_classifier=nearest

left_right_tree=/tmp/left_right_tree.json
first_step_tree=/tmp/first_step_tree.json

./$PARSER results/nearestall_weka.txt > $left_right_tree
./$PARSER results/firstsize_weka.txt  > $first_step_tree

./makegroundtruthcomparison.py -t $left_right_tree \
                               -c $left_right_classifier > ground.R

# Empty what's in the file previously (we need to do that because
# we will be appending to the file multiple times)
cat /dev/null > left_right_classification.R
cat /dev/null > first_step_classification.R
cat /dev/null > stepsizes.R

for file in \
    backyard.txt \
    bench.txt \
    book.txt \
    bridge.txt \
    building1.txt \
    building2.txt \
    building3.txt \
    cat.txt \
    cup1.txt \
    cup2.txt \
    cup3.txt \
    cup4.txt \
    fabric.txt \
    flower.txt \
    interior1.txt \
    interior2.txt \
    lamp.txt \
    landscape1.txt \
    landscape2.txt \
    landscape3.txt \
    moon.txt \
    screen.txt \
    snails.txt \
    stillLife.txt \
    vase.txt \
    books1.txt \
    books2.txt \
    books3.txt \
    books4.txt \
    gametree.txt \
    gorillapod.txt \
    granola.txt \
    timbuk.txt \
    ubuntu.txt
do
    ./makeclassifierplot.py -s $file -t $left_right_tree \
        -c $left_right_classifier >> left_right_classification.R
    ./makefirstsizeplot.py -s $file -t $first_step_tree \
        >> first_step_classification.R
    ./plotfullsweep.py -s $file >> stepsizes.R
done
