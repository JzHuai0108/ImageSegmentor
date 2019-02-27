
/***************************************************************/
/*                                                             */
/*           Mathematical Morphological Operations             */
/*refer to IMPLEMENTATION  OF  MATHEMATICAL MORPHOLOGICAL  OPERATIONS  FOR spatial data by dong pinliang                                                         */
/***************************************************************/
/* The following is a list of  functions in the  C programming */
/* language for (1) maximum/minimum calculation, (2) binary or */
/* gray scale image  dilation, (3) binary or gray  scale image */
/* erosion,  (4) binary  image  hit/miss transform, (5) binary */
/* image thinning, and (6) trimming of thinned image.          */
/*-------------------------------------------------------------*/
/* Description of arguments in the functions:                  */
/* arry  - pointer to a one-dimensional array;                 */
/* num   - size of the one-dimensional array;                  */
/* img   - input image (defined as pointer to pointer);        */
/* xsize - size of input image in X;                           */
/* ysize - size of input image in Y;                           */
/* st    - pointer to a structuring element defined as         */
/*         a 1-D array;                                        */
/* wdsz  - size of the structuring element defined as an       */
/*         n by n window where n is an odd. For thinning and   */
/*         trimming, wdsz=n=3; For other operations, wdsz can  */
/*         be 3, 5, 7, etc. The structuring element may have   */
/*         different shapes such as box, plus and cross. For   */
/*         gray scale image operations, box shape is used in   */
/*         relevant functions for simplicity.                  */
/* nitr  - number of iterations for a specific operation;      */
/* opt   - option for gray scale or binary operations; opt=1   */
/*         for gray scale, opt=0 for binary, and opt=-1 for    */
/*         erosion in thinning.                                */
/* th    - threshold for gray scale dilation and erosion       */
/*         operations. th=0 can be used to dilate/erode all    */
/*         the image pixels without setting non-zero pixel to  */
/*         zero.                                               */
/*                                                             */ 
/* Note: functions are indicated if they are called by others. */
/*                                                             */ 
/* Written by:                                                 */
/*                                                             */
/* Pinliang Dong                                               */
/* Department of Geology                                       */
/* University of New Brunswick                                 */
/* Fredericton, N.B.                                           */
/* Canada E3B 5A3                                              */
/*                                                             */
/* October 1995.                                               */
/*-------------------------------------------------------------*/
/*-------------------------------------------------------------*/
/*  (1) Calculate maximum and minimum values.                  */
/*      Called by dilation () and erosion ().                  */
/*-------------------------------------------------------------*/
#include<malloc.h>
void max_min(int *arry, int num); /* data array, and size of array */

/*-------------------------------------------------------------*/
/*  (2) Dilation of image (binary or gray scale)               */
/*-------------------------------------------------------------*/
int **dilation (int **img, int xsize, int ysize,int *st,int nitr,int wdsz,int opt,int th);

/*-------------------------------------------------------------*/
/*  (3) Erosion of image (binary or gray scale).               */
/*      Called by hitmiss ().                                  */
/*-------------------------------------------------------------*/
int **erosion(int **img,int xsize,int ysize,int *st,int nitr,int wdsz,int opt,int th);

/*-------------------------------------------------------------*/
/*  (4) Hit/miss transformation of image.                      */
/*      Called by thinning ().                                 */
/*-------------------------------------------------------------*/
int **hitmiss(int **img, int xsize, int ysize, int *st, int wdsz);

/*-------------------------------------------------------------*/
/*  (5) Thinning of image.                                     */
/*-------------------------------------------------------------*/
int **thinning(int **img,int xsize,int ysize,int nitr,int wdsz);

/*-------------------------------------------------------------*/
/*  (6) Trimming of 1-pixel thick skeleton legs & short lines. */
/*-------------------------------------------------------------*/
int **trimming(int **img, int xsize, int ysize, int nitr);

