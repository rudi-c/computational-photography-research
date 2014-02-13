# Generate decision trees with Weka and evaluate their effectiveness.

PARSER="../../weka-json-parser/parse-j48.py"
LEFT_RIGHT_ARFF=results/nearestall_filtered.arff
ACTION_ARFF=results/action.arff

CP="$CLASSPATH:/usr/share/java/weka-3.6.6.jar"

# ./simulate.py > $ACTION_ARFF

# Train trees with Weka
echo "Training left-right tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $LEFT_RIGHT_ARFF -C 0.25 -M 6 | ./$PARSER > /tmp/tree_leftright.json
echo "Training action tree..."
java -cp $CP weka.classifiers.trees.J48 \
    -t $ACTION_ARFF -C 0.25 -M 128 | ./$PARSER > /tmp/tree_action.json

cat /tmp/tree_leftright.json
cat /tmp/tree_action.json

# Evaluate tree effectiveness.
echo "Evaluating..."
./benchmark.py --left-right-tree=/tmp/tree_leftright.json \
            --action-tree=/tmp/tree_action.json

