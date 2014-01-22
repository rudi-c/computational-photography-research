
in=$1

if [ $# -ne 1 ]; then
  echo "Expected 1 argument (benchmark folder), received" $#. 1>&2
  exit 1
fi

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

    # ''> Results/$b/*.txt' doesn't work if the 
    # directory Results/$b doesn't exist
    mkdir -p ../Results/$b

    # highly ranked first deriv
    ./apply.exe  0 $in/$b/*.gray > ../Results/$b/out_firstorder3x3.txt
    ./apply.exe  1 $in/$b/*.gray > ../Results/$b/out_roberts3x3.txt
    ./apply.exe  2 $in/$b/*.gray > ../Results/$b/out_prewitt3x3.txt
    ./apply.exe  3 $in/$b/*.gray > ../Results/$b/out_scharr3x3.txt
    ./apply.exe  4 $in/$b/*.gray > ../Results/$b/out_sobel3x3.txt
    ./apply.exe 10 $in/$b/*.gray > ../Results/$b/out_brenner.txt
    ./apply.exe 12 $in/$b/*.gray > ../Results/$b/out_squaredGradient.txt
    ./apply.exe 27 $in/$b/*.gray > ../Results/$b/out_firstDerivGaussian.txt
    ./apply.exe 28 $in/$b/*.gray > ../Results/$b/out_LoG.txt

    # These are slow - uncomment only if needed.
    # ./apply.exe 30 $in/$b/*.gray > ../Results/$b/out_firstDerivGaussian2.txt
    # ./apply.exe 31 $in/$b/*.gray > ../Results/$b/out_firstDerivGaussian3.txt
    # ./apply.exe 32 $in/$b/*.gray > ../Results/$b/out_LoG2.txt
    # ./apply.exe 33 $in/$b/*.gray > ../Results/$b/out_LoG3.txt

    # # highly ranked second deriv
    ./apply.exe  7 $in/$b/*.gray > ../Results/$b/out_laplacian5x5.txt
    ./apply.exe  9 $in/$b/*.gray > ../Results/$b/out_sobel5x5so.txt
    ./apply.exe 26 $in/$b/*.gray > ../Results/$b/out_sobel5x5soCross.txt

    # # not consistently good
    ./apply.exe  5 $in/$b/*.gray > ../Results/$b/out_sobel5x5.txt
    ./apply.exe  6 $in/$b/*.gray > ../Results/$b/out_laplacian3x3.txt
    ./apply.exe  8 $in/$b/*.gray > ../Results/$b/out_sobel3x3so.txt
    ./apply.exe 11 $in/$b/*.gray > ../Results/$b/out_thresholdGradient.txt
    ./apply.exe 13 $in/$b/*.gray > ../Results/$b/out_MMHistogram.txt
    ./apply.exe 14 $in/$b/*.gray > ../Results/$b/out_rangeHistogram.txt
    ./apply.exe 15 $in/$b/*.gray > ../Results/$b/out_MGHistogram.txt
    ./apply.exe 16 $in/$b/*.gray > ../Results/$b/out_entropyHistogram.txt
    ./apply.exe 17 $in/$b/*.gray > ../Results/$b/out_th_cont.txt
    ./apply.exe 18 $in/$b/*.gray > ../Results/$b/out_num_pix.txt
    ./apply.exe 19 $in/$b/*.gray > ../Results/$b/out_power.txt
    ./apply.exe 20 $in/$b/*.gray > ../Results/$b/out_var.txt
    ./apply.exe 21 $in/$b/*.gray > ../Results/$b/out_nor_var.txt
    ./apply.exe 22 $in/$b/*.gray > ../Results/$b/out_vollath4.txt
    ./apply.exe 23 $in/$b/*.gray > ../Results/$b/out_vollath5.txt
    ./apply.exe 24 $in/$b/*.gray > ../Results/$b/out_autoCorrelation.txt
    ./apply.exe 25 $in/$b/*.gray > ../Results/$b/out_sobel3x3soCross.txt

    #sh compress.bash -b $in/$b > Results/$b/out_bzip2.txt
    #sh compress.bash -g $in/$b > Results/$b/out_gzip.txt
    #sh jpegcompress.bash 75 $in/$b > Results/$b/out_jpeg.txt
done

