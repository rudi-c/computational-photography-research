
in="../Benchmarks"
cd $in
mkdir Gray
for b in \
	LowLightCandle1 \
	LowLightCandle2 \
	LowLightCup \
	LowLightPicture_1_over_30 \
	LowLightPicture_1 \
	LowLightPicture_2 \
	LowLightPicture_4 \
	LowLightPicture_8
do
    echo $b
    for f in $b/*.gray; do ../Src/median --adaptive-median $f ; done
    mkdir Gray/$b
    mv $b/*.gray.median Gray/$b/
done

