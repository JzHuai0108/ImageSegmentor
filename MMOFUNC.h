
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
int max, min; 
void max_min(int *arry, int num) /* data array, and size of array */
{
  int *p, *array_end;
  array_end = arry + num;
  max = min = *arry;
  for (p=arry+1; p<array_end; p++)
    if (*p > max)
      max = *p;
    else if (*p < min)
      min = *p;
  return; /* return max and min */
}
/*-------------------------------------------------------------*/
/*  (2) Dilation of image (binary or gray scale)               */
/*-------------------------------------------------------------*/
int **dilation (int **img, int xsize, int ysize,int *st,int nitr,int wdsz,int opt,int th)
{
  int **dltn, *sx, *px, i,j,k,ok,n,dx,dy,ws,ww,
      nt0=0, /*number of 0's in st */
      nt1=0; /*number of 1's in st */

  ws = (wdsz-1)/2;
  ww = wdsz*wdsz;
  /* allocate memory */
  sx   = (int *) calloc( (size_t)(ww), sizeof(int));
  dltn  = (int **) calloc( (size_t)(ysize), sizeof(int));
  for (i=0; i<ysize; i++)
    dltn[i] = (int *) calloc( (size_t)(xsize), sizeof(int));
  
  for (k=0; k<nitr; k++) {/*iterations for dilation operation */
    for (i=ws; i<(ysize-ws); i++)
      for (j=ws; j<(xsize-ws); j++) {
        if (opt == 1) { /* for gray scale dilation */
          for (dy=-ws,n=0; dy<=ws; dy++)
            for (dx=-ws; dx<=ws && n<ww;
                 dx++,n++)
              sx[n] = *(*(img+i+dy)+j+dx);
          px = sx;
          max_min (px,ww); /* calculate max and min */
        }
        ok = 0;
        for (dy=-ws,n=0; dy<=ws && ok != 1; dy++)
          for (dx=-ws; dx<=ws && n<ww &&
                 ok != 1; dx++,n++) {
            if ( opt == 0 && *(st+n) == 1 &&
                 *(*(img+i+dy)+j+dx) == 1 ) {
              dltn[i][j] = 1; /* binary dilation */
              ok = 1;
            }
            if ( opt == 1 && *(st+n) == 1 &&
                 *(*(img+i+dy)+j+dx) >= th ) {
              dltn[i][j] = max; /* gray scale dilation */
              ok = 1;
            }
          }
      }
    if (nitr > 1) {  /* more than one iterations for dilation */
      for (i=0; i<ysize; i++)
        for (j=0; j<xsize; j++)
          img[i][j] = dltn[i][j];
    }
  }
  free(sx);
  return (dltn);
}
/*-------------------------------------------------------------*/
/*  (3) Erosion of image (binary or gray scale).               */
/*      Called by hitmiss ().                                  */
/*-------------------------------------------------------------*/
int **erosion(int **img,int xsize,int ysize,int *st,int nitr,int wdsz,int opt,int th)
{
  int **eros,*sx,*px,i,j,k,k0,k1,n,dx,dy,ws,ww,
      nt0=0, /*number of 0's in st */
      nt1=0; /*number of 1's in st */

  ws = (wdsz-1)/2;
  ww = wdsz*wdsz;
  /* allocate memory */
  sx    = (int *) calloc( (size_t)(ww), sizeof(int));
  eros  = (int **) calloc( (size_t)(ysize), sizeof(int));
  for (i=0; i<ysize; i++)
    eros[i]  = (int *) calloc( (size_t)(xsize), sizeof(int));
  for (n=0; n < ww; n++) {/*get # of 0's and 1's in st */
    if ( st[n] == 0)
      nt0 ++;
    if ( st[n] == 1)
      nt1 ++;
  }
  if (opt == -1) { /* for erosion operations in thinning */
    for (i=ws; i<(ysize-ws); i++)
      for (j=ws; j<(xsize-ws); j++) {
        k0 = 0;
        k1 = 0;
        /* comparison of the input image and st */
        for (dy=-ws,n=0; dy<=ws; dy++)
          for (dx=-ws; dx<=ws && n<ww;
               dx++,n++) {
            if ( st[n] == 0 && *(*(img+i+dy)+j+dx) == 0)
              k0++;
            if ( st[n] == 1 && *(*(img+i+dy)+j+dx) == 1)
              k1++;
          }
        if (k0 == nt0 && k1 == nt1)
          eros[i][j] = 1; /* keep necessary pixels */
      }
  }
  else { /*for general erosion operations */
    for (k=0; k<nitr; k++) {/*iterations for erosion operation */
      for (i=ws; i<(ysize-ws); i++)
        for (j=ws; j<(xsize-ws); j++) {
          k1 = 0;
          if (opt == 1) { /* for gray scale erosion */
            for (dy=-ws,n=0; dy<=ws; dy++)
              for (dx=-ws; dx<=ws && n<ww;
                   dx++,n++)
                sx[n] = *(*(img+i+dy)+j+dx);
            px = sx;
            max_min (px,ww); /* calculate max & min */
          }
          for (dy=-ws,n=0; dy<=ws; dy++)
            for (dx=-ws; dx<=ws && n<ww;
                 dx++,n++) {
              if ( opt == 0 && *(st+n) == 1 &&
                   *(*(img+i+dy)+j+dx) == 1 )
                k1++;
              if ( opt == 1 && *(st+n) == 1 &&
                   *(*(img+i+dy)+j+dx) >= th)
                k1++;
            }
          if (opt == 0)  /* binary erosion */
            eros[i][j] = (k1 == nt1)? 1 : 0;
          if (opt == 1)  /* gray scale erosion */
            eros[i][j] = (k1 == nt1)? min : 0;
        }
      if (nitr > 1) { /* more than one interations for erosion */
        for (i=0; i<ysize; i++)
          for (j=0; j<xsize; j++)
            img[i][j] = eros[i][j];
      }
    }
  }
  free(sx);
    return (eros);
}
/*-------------------------------------------------------------*/
/*  (4) Hit/miss transformation of image.                      */
/*      Called by thinning ().                                 */
/*-------------------------------------------------------------*/
int **hitmiss(int **img, int xsize, int ysize, int *st, int wdsz)
{
  int **cimg,**hit,**chit,**thn,*cst,i,j,n,ws,ww,
      **p1,**p2;
  
  ws = (wdsz-1)/2;
  ww = wdsz*wdsz;
  /* allocate memory */
  cst   = (int  *) calloc( (size_t)(ww), sizeof(int));
  cimg    = (int **) calloc( (size_t)(ysize), sizeof(int));
  hit   = (int **) calloc( (size_t)(ysize), sizeof(int));
  chit  = (int **) calloc( (size_t)(ysize), sizeof(int));
  thn   = (int **) calloc( (size_t)(ysize), sizeof(int));
  for (i=0; i<ysize; i++) {
    cimg[i]    = (int *) calloc( (size_t)(xsize), sizeof(int));
    hit[i]   = (int *) calloc( (size_t)(xsize), sizeof(int));
    chit[i]  = (int *) calloc( (size_t)(xsize), sizeof(int));
    thn[i]   = (int *) calloc( (size_t)(xsize), sizeof(int));
  }
  for (i=0; i < ysize; i++) 
    for (j=0; j < xsize; j++)  /* complement of img[i][j] */
      cimg[i][j] = (img[i][j] == 0)? 1 : 0;
  for (n=0; n < ww; n++) { /* complement of st */
    if (st[n] == 0)
      cst[n] = 1;
    if (st[n] == 1)
      cst[n] = 0;
    if (st[n] == -1) /* x is represented by -1 */
      cst[n] = -1;   /* x keeps unchanged  */
  }
  p1 = erosion (img,xsize,ysize,st,1,wdsz,-1,-1);
  p2 = erosion (cimg,xsize,ysize,cst,1,wdsz,-1,-1);  
  for (i=ws; i<ysize-ws; i++) 
    for (j=ws; j<xsize-ws; j++) 
      hit[i][j] = ( *(*(p1+i)+j)==1 && *(*(p2+i)+j)==1)? 1 : 0;
  for (i=ws; i<ysize-ws; i++)
    for (j=ws; j<xsize-ws; j++) 
      /* complement of hit[i][j] */
      chit[i][j] = (hit[i][j] == 1)? 0 : 1;
  for (i=ws; i<ysize-ws; i++)
    for (j=ws; j<xsize-ws; j++) 
      /* thinning of img[i][j] */
      thn[i][j] = (img[i][j]==1 && img[i][j]==chit[i][j])? 1 : 0;
	free(cst);
	 for (i=0; i<ysize; i++) {
    free(cimg[i]);
    free(hit[i]);
    free(chit[i]);
  }
	 free(cimg);
    free(hit);
    free(chit);
  return (thn); /* return hit/miss results */
}
/*-------------------------------------------------------------*/
/*  (5) Thinning of image.                                     */
/*-------------------------------------------------------------*/
int **thinning(int **img,int xsize,int ysize,int nitr,int wdsz)
{
  int **thin,**p1,**p2,**p3,*pt1,*pt2,*pt3,i,j,k,pk,ok=0;
  /* structuring elements for thinning */
  static int d1[9] = { 0, 0,-1, 0, 1, 1,-1, 1,-1 };
  static int d2[9] = {-1, 0, 0, 1, 1, 0,-1, 1,-1 };
  static int d3[9] = {-1, 1,-1, 1, 1, 0,-1, 0, 0 };
  static int d4[9] = {-1, 1,-1, 0, 1, 1, 0, 0,-1 };
  static int e1[9] = {-1, 0,-1, 1, 1, 1,-1, 1,-1 };
  static int e2[9] = {-1, 1,-1, 1, 1, 0,-1, 1,-1 };
  static int e3[9] = {-1, 1,-1, 1, 1, 1,-1, 0,-1 };
  static int e4[9] = {-1, 1,-1, 0, 1, 1,-1, 1,-1 };
  /* allocate memory */
  thin    = (int **) calloc( (size_t)(ysize), sizeof(int));
  for (i=0; i<ysize; i++)
    thin[i] = (int *) calloc( (size_t)(xsize), sizeof(int));
  
  for (k=0; k<nitr; k++) /* iterations for thinning */
    for (pk=1; pk<=4; pk++) { /*four passes for each iteration */
      ok = 0;
      if (pk==1) {
        pt1 = d1;
        pt2 = d2;
        pt3 = e1;
      }
      if (pk==2) {
        pt1 = d2;
        pt2 = d3;
        pt3 = e2;
      }
      if (pk==3) {
        pt1 = d3;
        pt2 = d4;
        pt3 = e3;
      }
      if (pk==4) {
        pt1 = d4;
        pt2 = d1;
        pt3 = e4;
      }
      p1 = hitmiss (img,xsize,ysize,pt1,3);
      p2 = hitmiss (img,xsize,ysize,pt2,3);
      p3 = hitmiss (img,xsize,ysize,pt3,3);
      for (i=0; i<ysize; i++)
        for (j=0; j<xsize; j++) {/*comparion of hitmiss results*/
          if ( *(*(p1+i)+j) == 1 && *(*(p2+i)+j) == 1 &&
               *(*(p3+i)+j) == 1)
            thin[i][j] = 1; /* keep the pixel */
          else
            thin[i][j] = 0; /* delete the pixel */
        }
      if (nitr > 1) { /* more than one iterations for thinning */
        for (i=0; i<ysize; i++)
          for (j=0; j<xsize; j++)
            img[i][j] = thin[i][j];
      }
    }
	 for (i=0; i<ysize; i++)
    for (j=0; j<xsize; j++)
       img[i][j]=thin[i][j];
  	for (i=0; i<ysize; i++)
		{
			
			free(thin[i]);
		}	
		free(thin);
  return (img); 
}
/*-------------------------------------------------------------*/
/*  (6) Trimming of 1-pixel thick skeleton legs & short lines. */
/*-------------------------------------------------------------*/
int **trimming(int **img, int xsize, int ysize, int nitr)
{
  int **trim,i,j,k,k1,k2,dx,dy,t0,t1,
      nt0[9],/* # of 0's in trimming templates 1-9 listed below*/
      nt1[9];/* # of 1's in trimming templates 1-9 listed below*/
  
  /* trimming matrix from Figure 2 where x is replaced by -1.  */
  static int g0[9][9] = { { 0, 0, 0, 0, 1, 0, 0, 0, 0},
                          {-1, 1,-1, 0, 1, 0, 0, 0, 0},
                          { 0, 0, 1, 0, 1, 0, 0, 0, 0},
                          { 0, 0,-1, 0, 1, 1, 0, 0,-1},
                          { 0, 0, 0, 0, 1, 0, 0, 0, 1},
                          { 0, 0, 0, 0, 1, 0,-1, 1,-1},
                          { 0, 0, 0, 0, 1, 0, 1, 0, 0},
                          {-1, 0, 0, 1, 1, 0,-1, 0, 0},
                          { 1, 0, 0, 0, 1, 0, 0, 0, 0} };
  /*allocate memory */
  trim  = (int **) calloc( (size_t)(ysize), sizeof(int));
  for (i=0; i<ysize; i++)
    trim[i]  = (int *) calloc( (size_t)(xsize), sizeof(int));
  for (i=0; i<ysize; i++)
    for (j=0; j<xsize; j++)
      trim[i][j] = *(*(img+i)+j);
  nt0[0] = 8;
  nt0[1] = nt0[3] = nt0[5] = nt0[7] = 5;
  nt0[2] = nt0[4] = nt0[6] = nt0[8] = 7;
  nt1[0] = 1;
  nt1[1] = nt1[2] = nt1[3] = nt1[4] = nt1[5] = nt1[6] =
    nt1[7] = nt1[8] = 2;
  /* do trimmimg using the nine structuring templates */
  for (k=0; k<nitr; k++) {/*iterations for trimming operation */
    for (i=1; i<ysize-1; i++)
      for (j=1; j<xsize-1; j++) {
        for (k1=0; k1<9; k1++) {
          /* apply the nine structuring templates */
          t0 = 0;  /* number of 0's in the input image window */
          t1 = 0;  /* number of 1's in the input image window */
          for (dy=-1,k2=0; dy<=1; dy++)
            for (dx=-1; dx<=1 && k2<9; dx++,k2++) {
              /*comparison of the input image and the template*/
              if ( *(*(g0+k1)+k2)==0 && *(*(img+i+dy)+j+dx)==0)
                t0++;
              if ( *(*(g0+k1)+k2)==1 && *(*(img+i+dy)+j+dx)==1)
                t1++;
            }

          if ( *(*(img+i)+j) == 1 && t0 == nt0[k1] &&
               t1 == nt1[k1])
            trim[i][j] = 0;/*do trimming by setting pixel to 0*/
        }
      }
    if (nitr > 1) { /* more than one iterations for trimming */
      for (i=0; i<ysize; i++)
        for (j=0; j<xsize; j++)
          img[i][j] = trim[i][j];
    }
  }
  for (i=0; i<ysize; i++)
    for (j=0; j<xsize; j++)
       img[i][j]=trim[i][j];
  	for (i=0; i<ysize; i++)
		{
			
			free(trim[i]);
		}	
		free(trim);
  return (img); /* return trimmed image */ 
}
