# ImageSegmentor
# WARN: This package is OBSOLETE and no longer maintained.

segment remotely sensed images into meaningful regions.

This software has realized the following methods for segmentating images: graph based segmentation, quad tree segmentation, multiresolution segmentation, and two-stage segmentation, watershed, mean shift; also, mrf classification, primitive building extraction, change detection based on gradient correlation, histogram matching, intensity correlation and rosin automatic thresholding.

The release version has been tested to run successfully over windows XP, windows 7 operating systems. It is written by C++ language, compiled in Visual C++ 6.0.

# Dependencies

OpenCV 1.0 libraries are used to support some routines, such as region geometrical properties computation, morphological operations, k-means clustering. The GDAL library is used to read and write a host of image formats, *.tif, *.img, etc., and to produce vector files, such as *.shp. Both of OpenCV and GDAL are included in the package for easy compilation.

# Installation

(1) Clone this package onto your computer
(2) Open the image.dsw file in the root directory in Visual C++6.0 compiler
(3) Compile. If there are linking problems, you may have to change the project settings to link libraries for GDAL and OpenCV.
(4) Click the built executable, and open an image, try menu options.

# Licence

ImageSegmentor is released under a GPLv3 license (see License-gpl.txt). Please note in this package we also provide OpenCV 1.0 released under BSD and GDAL released under an X/MIT style Open Source license by the Open Source Geospatial Foundation.

For a closed-source version of ImageSegmentor for commercial purposes, please contact the authors.

If you use ImageSegmentor in an academic work, please cite the following publication:
	@article{
	year={2010},
	issn={0255-660X},
	journal={Journal of the Indian Society of Remote Sensing},
	volume={38},
	number={4},
	doi={10.1007/s12524-011-0085-3},
	title={An Improved Hierarchical Segmentation Method for Remote Sensing Images},
	url={http://dx.doi.org/10.1007/s12524-011-0085-3},
	publisher={Springer-Verlag},
	keywords={Inversed quad-tree; Image segmentation; Remote sensing; eCognition; ENVI},
	author={Tan, Yumin and Huai, Jianzhu and Tang, Zhongshi and Xi, Weiwei},
	pages={686-695},
	language={English}
	}
Link to pdf: http://link.springer.com/article/10.1007%2Fs12524-011-0085-3

More information about this package can be found at http://blog.csdn.net/pobudeyi/article/details/6074118
