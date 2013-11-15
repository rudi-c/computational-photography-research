
in="../Benchmarks/Gray"

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
    # highly ranked first deriv
    ./apply.exe  0 $in/$b/*.gray.median > Results/$b/outgray_firstorder3x3.txt
    ./apply.exe  1 $in/$b/*.gray.median > Results/$b/outgray_roberts3x3.txt
    ./apply.exe  2 $in/$b/*.gray.median > Results/$b/outgray_prewitt3x3.txt
    ./apply.exe  3 $in/$b/*.gray.median > Results/$b/outgray_scharr3x3.txt
    ./apply.exe  4 $in/$b/*.gray.median > Results/$b/outgray_sobel3x3.txt
    ./apply.exe 10 $in/$b/*.gray.median > Results/$b/outgray_brenner.txt
    ./apply.exe 12 $in/$b/*.gray.median > Results/$b/outgray_squaredGradient.txt
    ./apply.exe 27 $in/$b/*.gray.median > Results/$b/outgray_firstDerivGaussian.txt
    ./apply.exe 28 $in/$b/*.gray.median > Results/$b/outgray_LoG.txt

    # highly ranked second deriv
    ./apply.exe  7 $in/$b/*.gray.median > Results/$b/outgray_laplacian5x5.txt
    ./apply.exe  9 $in/$b/*.gray.median > Results/$b/outgray_sobel5x5so.txt
    ./apply.exe 26 $in/$b/*.gray.median > Results/$b/outgray_sobel5x5soCross.txt
done

