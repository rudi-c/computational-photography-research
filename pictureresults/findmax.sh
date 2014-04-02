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
	vase.txt \
	books1.txt \
	books2.txt \
	books3.txt \
	books4.txt \
	gorillapod.txt \
	granola.txt \
	timbuk.txt \
	ubuntu.txt
do
	echo $file >> $maximaFile
	./localMax.exe < focusmeasures/$file >> $maximaFile
done

# For gametree.txt, localmax misses a maxima.
echo gametree.txt >> $maximaFile
echo " "`./localMax.exe < focusmeasures/gametree.txt`" 56" >> $maximaFile

./makestatistics.py > stats.R

for file in \
	blackboard1.txt \
	blackboard2.txt \
	pillow1.txt \
	pillow2.txt \
	pillow3.txt \
	projector1.txt \
	projector2.txt \
	projector3.txt \
	screws1.txt \
	screws2.txt \
	whiteboard1.txt \
	whiteboard2.txt \
	whiteboard3.txt
do
	echo $file >> $maximaFile
	./localMax.exe < lowlightgaussnorm/$file >> $maximaFile
done