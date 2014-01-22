# Should replace with a Makefile if we ever have more than
# one file.
g++ localMax.cpp -o localMax.exe

maximaFile=maxima.txt

# Empty what's in the file previously (we need to do that because
# we will be appending to the file multiple times)
cat /dev/null > $maximaFile

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
	vase.txt
do
	echo $file >> $maximaFile
	./localMax.exe < focusmeasures/$file >> $maximaFile
done

./makestatistics.py