
# TREE="(ratio2Inverse_8 right left)"
# TREE="(downTrend right (ratio2Inverse_8 right left))"

# New features, nearest, 3 measures with brackets, step size 1
# TREE="(downTrend (diffRatioMin3_5 right 
#                                 (bracket right right right left left)) 
#                  (bracket right 
#                           (diffRatioMin2_5 right
#                                            (diffRatioMin3_6 right left))
#                           (ratio3_19 left right)
#                           left
#                           left))"

# New features and dataset, highest, 3 measures with brackets, 
# step size 1, min 10 nodes per leaf
# TREE="(diffRatioAvg3_5 (bracket right right right
#                                 (logRatio3_9 left
#                                              (upTrend left right))
#                                 (upTrend left
#                                          (logRatio3_9 left right)))
#                        (downTrend (bracket right right left left left)
#                                   (bracket (diffRatioMin2_5 right left)
#                                            left left left left)))"

# New features and dataset, nearest or near_high (same result), 
# 3 measures with brackets, step size 1, min 10 nodes per leaf
# TREE="(diffRatioAvg3_5 (upTrend (bracket right right right left left)
#                                 (bracket right right right right
#                                         (ratio3_9 left right)))
#                        (downTrend (bracket right right left left left)
#                                   (bracket (diffRatioMin2_5 right left)
#                                            (ratio3_19 left right)
#                                            left left left)))"

# New features 2 and raw data, highest, 3 measures with brackets
# TREE="(downTrend (ratio3_7 (bracket right right
#                                     (ratio3_6 right left)
#                                     left left)
#                            (bracket right right right
#                                     (ratio3_8 left right)
#                                     (ratio3_8 left right)))
#                  (bracket (ratio3_6 left right)
#                           (ratio3_6 left right)
#                           left left left))"

# New features 2 and raw data, nearest, 3 measures with brackets
# TREE="(ratio3_7 (downTrend (bracket right right 
#                                    (diffRatioMax3_6 left right)
#                                    left left)
#                            (bracket (diffRatioMax3_3 right left)
#                                     (diffRatioMax3_3 (diffRatioMax3_6 right left)
#                                                      left)
#                                     left left left))
#                 (ratio3_8 (bracket right right right left left)
#                           (upTrend (bracket right right right right left)
#                                    right)))"

# New features 2 and raw data, near-highest, 3 measures with brackets
TREE="(ratio3_7 (downTrend (bracket right right 
                                   (diffRatioMax3_6 left right)
                                   left left)
                           (diffRatioMax3_6 (bracket right right left left left)
                                            left))
                (ratio3_8 (bracket right right right left left)
                          (upTrend (bracket right right right right left)
                                   right)))"


#CLASSIFIER=highest
#CLASSIFIER=nearest
CLASSIFIER=near_high

./makegroundtruthcomparison.py -t "$TREE" -c $CLASSIFIER > ground.R

# Empty what's in the file previously (we need to do that because
# we will be appending to the file multiple times)
cat /dev/null > classification.R
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
    ./makeclassifierplot.py -s $file -t "$TREE" -c $CLASSIFIER >> classification.R
    ./evaluatestepsize.py -s $file >> stepsizes.R
done
