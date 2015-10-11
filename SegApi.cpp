/**
* This file is part of ImageSegmentor.
*
* Copyright (C) 2012 Jianzhu Huai <huai dot 3 at osu dot edu> (The Ohio State University)
*
* ImageSegmentor is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ImageSegmentor is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ImageSegmentor. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "GlobalApi.h"

#include "susan.h"
#include "Thinner.h"
#include "MMOFUNC.h"
//#include "math.h"
//#include <direct.h>
#include <queue>
using namespace std;
//following freeman code style as introduced in opencv reference
//roadseed points a memory space of size width*height with 1 denotes roads 0 for background 
//after processing, 2 for centerline
//vector<int>terminal returns the end pixels in form of serial number in roads after pruning
/*void RoadThin(BYTE*roadseed, int Width,int Height,vector<int>&terminal)
{
	int sernum=0,i,j,L=Height*Width;
	int x,y,tsize,hsize;
	BYTE*roads=new BYTE[L];
	memcpy(roads,roadseed,sizeof(BYTE)*L);
	ThinnerRosenfeld(roads,Height,Width);
	//	int **img2,xsize,ysize;
	/*	ysize=Width; 
	xsize=Height;
	img2=new int *[ysize];
	for (i=0; i<ysize; i++)
	img2[i] =new int[xsize];
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{	      
			img2[j][i]=roads[i*Width+j];				
		}
	}
	trimming(img2,xsize,ysize,3);	*/
	//detect end nodes
/*	vector<int> edgecode;
	vector<int>::iterator vint;
	int normin[8]={0},dest[8]={0};
	int k,count,runlen,next;
//x,y coordinate for pixel, tsize the size of stack storing terminals, 
	//hsize terminals deleted during pruning
	const int dangle=10;
	bool flag;
		//do not take care border pixels
	for(i=0;i<Height;++i)
	{
		sernum=i*Width;
		roads[sernum]=0;
		roads[sernum+Width-1]=0;
	}
	sernum=(Height-1)*Width;
	for(j=0;j<Width;++j)
	{
		roads[j]=0;
		roads[sernum+j]=0;
	}
	for(i=1;i<Height-1;++i)
	{
		for(j=1;j<Width-1;++j)
		{
			sernum=i*Width+j;
			if(roads[sernum]==1)
			{
				normin[0]=sernum+1;
				normin[1]=sernum+1-Width;
				normin[2]=sernum-Width;
				normin[3]=sernum-1-Width;
				normin[4]=sernum-1;
				normin[5]=sernum-1+Width;
				normin[6]=sernum+Width;
				normin[7]=sernum+1+Width;
				count=0;
				for(k=0;k<8;++k)
				{
					if(roads[normin[k]])
					{
						dest[count]=k;
						++count;
					}
				}
				//there are spur lines on the left border of the image			

				//1 0 0  count==4	0	1	0 count==4
				//1 1 1	on the  	1	1	1 in the interior				
				//1 0 0 image border0	1	0 for cross		
			
				switch(count)
				{				
				case 1:
					terminal.push_back(sernum);
					break;
				case 2:
					if((dest[1]-dest[0]==1)||(dest[1]-dest[0]==7))
						terminal.push_back(sernum);
					break;
				case 3:
				//	terminal.push_back(sernum);
					break;
				case 4: break;
					default:
				AfxMessageBox("damn!singular point exists!");break;
				
				}
			}
		}
	}

	//prune dangling edges, note that the pruned edge may be longer than later processed spurs 
	tsize=terminal.size();
	hsize=0;
	for(i=0;i<tsize;++i)//for each end point
	{
		sernum=terminal[i];//position in image array
		assert(roads[sernum]>0);
			
		runlen=0;
		flag=true;//true for proceeding, false for halt while loop
		edgecode.clear();
		while(runlen<dangle)
		{
			assert(roads[sernum]==1);
			x=sernum%Width;
			y=sernum/Width;
			for(j=0;j<8;++j)
				normin[j]=0;
			if(x==0)
			{
				if(y==0)
				{					
					normin[0]=roads[sernum+1];
					normin[6]=roads[sernum+Width];
					normin[7]=roads[sernum+1+Width];
				}
				else if(y==Height-1)
				{
					normin[0]=roads[sernum+1];
					normin[1]=roads[sernum+1-Width];
					normin[2]=roads[sernum-Width];
				}
				else
				{
					normin[0]=roads[sernum+1];
					normin[1]=roads[sernum+1-Width];
					normin[2]=roads[sernum-Width];
					normin[6]=roads[sernum+Width];
					normin[7]=roads[sernum+1+Width];					
				}
			}
			else if(x==Width-1)
			{
				if(y==0)
				{
					normin[4]=roads[sernum-1];
					normin[5]=roads[sernum-1+Width];
					normin[6]=roads[sernum+Width];
				}
				else if(y==Height-1)
				{
					normin[2]=roads[sernum-Width];
					normin[3]=roads[sernum-1-Width];
					normin[4]=roads[sernum-1];
				}
				else
				{
					normin[2]=roads[sernum-Width];
					normin[3]=roads[sernum-1-Width];
					normin[4]=roads[sernum-1];
					normin[5]=roads[sernum-1+Width];
					normin[6]=roads[sernum+Width];
					
				}
			}
			else
			{
				if(y==0)
				{
					normin[0]=roads[sernum+1];
					normin[4]=roads[sernum-1];
					normin[5]=roads[sernum-1+Width];
					normin[6]=roads[sernum+Width];
					normin[7]=roads[sernum+1+Width];
				}
				else if(y==Height-1)
				{
					normin[0]=roads[sernum+1];
					normin[1]=roads[sernum+1-Width];
					normin[2]=roads[sernum-Width];
					normin[3]=roads[sernum-1-Width];
					normin[4]=roads[sernum-1];
				}
				else
				{
					normin[0]=roads[sernum+1];
					normin[1]=roads[sernum+1-Width];
					normin[2]=roads[sernum-Width];
					normin[3]=roads[sernum-1-Width];
					normin[4]=roads[sernum-1];
					normin[5]=roads[sernum-1+Width];
					normin[6]=roads[sernum+Width];
					normin[7]=roads[sernum+1+Width];
					
				}
			}					
			count=0;
			for(k=0;k<8;++k)
			{
				if(normin[k])
				{
					dest[count]=k;
					++count;
				}
			}
			switch(count)
			{				
			case 1:
				if(runlen>0)					
				{
					AfxMessageBox("small edge with length less than 7 exist!");
					flag=false;
				}
				next=dest[0];
				edgecode.push_back(sernum);
				++runlen;
				break;
			case 2:
				assert(runlen>0);
				next=next>3?next-4:next+4;
				next=dest[0]+dest[1]-next;
				
				edgecode.push_back(sernum);
				++runlen;
				break;
			case 3://we don't push back sernum here, bacause if that sernum is nullified in roads
				//the other spurs may not be found or fall into dead loops
			case 4://010
				   //111
				   //010
				flag=false;
				break;
			default: 
				AfxMessageBox("neighbor count illegal!");
				break;
				
			}
			if(flag==false)
				break;
			switch(next)
			{
			case 0:
				++sernum;
				break;
			case 1:
				sernum=sernum-Width+1;
				break;
			case 2:
				sernum=sernum-Width;
				break;
			case 3:
				sernum=sernum-Width-1;
				break;
			case 4:
				sernum=sernum-1;
				break;
			case 5:
				sernum=sernum+Width-1;
				break;
			case 6:
				sernum=sernum+Width;
				break;
			case 7:
				sernum=sernum+Width+1;
				break;
			default:
				AfxMessageBox("next neighbor index exceeds bound!");
			}				
			
		}
		assert(runlen==edgecode.size());
		if(runlen<dangle)//negative pixels in edgecode
		{
			for(k=0;k<runlen;++k)			
				roads[edgecode[k]]=0;
			++hsize;
		}	
	}
	edgecode.clear();
	//get really terminals
	hsize=tsize-hsize;
	vint=terminal.begin();
	while(vint!=terminal.end())
	{
		if(roads[*vint]==0)
		{			
			terminal.erase(vint);
		}
		else
		{
			++vint;
		}
	}
	assert(hsize==terminal.size());
	for(i=0;i<L;++i)
		roadseed[i]+=roads[i];

	//show result
/*	CvSize bound=cvSize(Width,Height);
	IplImage*portal=cvCreateImage(bound, IPL_DEPTH_8U , 3);
	cvNamedWindow("Portal", 0);		
	cvZero(portal);
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{				
			if(roads[i*Width+j]>0)//(img2[j][i]==1)
			{	uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*i))[j*3];
			temp_ptr[0]=0;
			temp_ptr[1]=255;
			temp_ptr[2]=0;
			}		
		}   
	}
	count=terminal.size();
	for(k=0;k<count;++k)
	{
		i=terminal[k]/Width;
		j=terminal[k]%Width;
		CvPoint center=cvPoint(j,i);		
		cvCircle( portal, center, 2, CV_RGB (255, 0, 0 ),1);
	}

	cvFlip(portal);
	cvShowImage("Portal", portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);*
	
	delete[]roads;

}*/
//process binary image data stored in EM with white background
void Morph(BYTE*EM, int Width,int Height,int opt)
{
	int i,j;
	int *img;
	int **img2;
	int xsize,ysize;
	//2 3 5 outperform others, test image has bright background
	//before processing negative or binarize might be required
	//note the order of height and width in the parameters list of each thinning operation
	switch(opt)
	{
	case 1:
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				EM[i*Width+j]=~EM[i*Width+j];
			}
		}
		
		ThinnerPavlidis(EM,Height,Width);
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{
				
				if(EM[i*Width+j]!=0)
					EM[i*Width+j]=(BYTE)255;
			}   
		}
		break;
	case 2:	
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				EM[i*Width+j]=~EM[i*Width+j];
			}
		}
		
		ThinnerHilditch(EM,Height,Width);
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{
				
				if(EM[i*Width+j]!=0)
					EM[i*Width+j]=(BYTE)255;
			}   
		}
		break;
	case 3:
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				if(EM[i*Width+j]==0)
					EM[i*Width+j]=1;
				
				else
					EM[i*Width+j]=0;
			}
		}
		ThinnerRosenfeld(EM,Height,Width);
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{
				
				if(EM[i*Width+j]==1)
					EM[i*Width+j]=(BYTE)255;
				
				else
					EM[i*Width+j]=(BYTE)0;
			}   
		}
		break;
	case 4:

		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				EM[i*Width+j]=~EM[i*Width+j];
			}
		}
		ThiningDIBSkeleton (EM,Width,Height);
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{
				
				if(EM[i*Width+j]!=0)
					EM[i*Width+j]=(BYTE)255;
			}   
		}
		break;
	case 5://from image-j1 
		img=new int[Height*Width];
	
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				if(EM[i*Width+j]==0)
					img[i*Width+j]=1;				
				else
					img[i*Width+j]=0;
			}
		}
		Thin_2(img,0, 0 , (int)Height,(int)Width);
		
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{				
				if(img[i*Width+j]!=0)
					EM[i*Width+j]=(BYTE)0;
				else EM[i*Width+j]=(BYTE)255;
			}   
		}
		delete[]img;
		break;
	case 6://from mmofunc not effective
		ysize=Width; 
		xsize=Height;
		img2=new int *[ysize];
		for (i=0; i<ysize; i++)
			img2[i] =new int[xsize];
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				if(EM[i*Width+j]==0)
					img2[j][i]=1;				
				else
					img2[j][i]=0;
			}
		}
		thinning(img2,xsize,ysize,3,3);
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{				
				if(img2[j][i]!=0)
					EM[i*Width+j]=(BYTE)255;
			}   
		}
		for (i=0; i<ysize; i++)
		{
			delete[] img2[i];
			
		}
		delete[]img2;
	
		break;
	case 7://THIN BLACK THREAD IN BINARY IMAGE
		ysize=Width; 
		xsize=Height;
		img2=new int *[ysize];
		for (i=0; i<ysize; i++)
			img2[i] =new int[xsize];
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{	      
				if(EM[i*Width+j]==0)
					img2[j][i]=1;				
				else
					img2[j][i]=0;
			}
		}
		trimming(img2,xsize,ysize,3);
		for(i=0;i<Height;i++)
		{
			for(j=0;j<Width;j++)
			{				
				if(img2[j][i]!=0)
					EM[i*Width+j]=(BYTE)0;
				else
					EM[i*Width+j]=255;
			}   
		}
		for (i=0; i<ysize; i++)
		{
			delete[] img2[i];		
		}
		delete[]img2;
	
		break;
	default: break;
	}	
}
BYTE* EdgeSusan(BYTE*EM, int w,int h,int BT,bool Trixtri)
{
	int L=h*w,y=0;
	BYTE*pPE=EM;
	susanedge(EM,w,h,BT,Trixtri);//mark edge pixels black on white background 
	//convert to 255 standing for edge and 0 for nonedge
	/*for(y=0;y<L;++y)
	{
			*pPE=255-(*pPE);						
			++pPE;			
	}*/
	//if more accurate tuning is necessary, susan is optional.
	return EM;
}
void GaussianFilter(BYTE*EM,int Width, int Height,float sigma)
{
	int L=Width*Height;
	BYTE*pUnchSmooth=new BYTE[L];

	GaussianSmooth(EM, Width, Height, sigma, pUnchSmooth);
	memcpy(EM,pUnchSmooth,sizeof(BYTE)*L);
	delete[]pUnchSmooth;
}
void EdgeMag(BYTE*EM,int w,int h)
{

	float *exEM = new float[w*h];
	float max=0.f,min=256.f;
	float *ptr=exEM;
	int i=0,j=0;
	// \B5\F7\D3\C3SOBEL FILTER compute gradient absolute value
	Gradient(EM,w,h,exEM);
	for(j=0;j<h;++j)
		for(i=0;i<w;++i)
		{
			if(*ptr>max)
				max=*ptr;
			if(*ptr<min)
				min=*ptr;
			++ptr;
		}
		ptr=exEM;
		BYTE*p2=EM;
		max-=min;
	for(j=0;j<h;++j)
		for(i=0;i<w;++i)
		{
			*p2=(*ptr-min)/max*255;
			++p2;++ptr;
		}
	delete []exEM;
	exEM=NULL;
}
//edgepoints finds edge candidate points in edge channel
//input:edge channel with arbitrary size output: edge map with 255 for edge point
//the edge channel is processed by gaussian filter and then canny filter
void EdgePoints(BYTE* EM,int w,int h,float sig, float low, float high)
{
	BYTE* input = new BYTE[w*h];
	memcpy(input,EM,sizeof(BYTE)*w*h);
	// \B5\F7\D3\C3canny\BA\AF\CA\FD\BD\F8\D0б߽\E7\CC\E1ȡ
	Canny(input,w,h, sig,low,high, EM);
	
	delete[] input;
}

MyLUV*  GetNearPixelsLUV(int xPos, int yPos
	  , MyLUV* inLUVs, int picWidth, int picHeight
	  , int inScale, int& outWidth, int& outHeight)
//\B5õ\BDLUV\C1\DA\D3\F2\A3\BB
{
	//\CA\D7\CFȼ\C6\CB\E3\BF\C9\C4ܵ\C4\C1\DA\D3\F2\B5\E3\CA\FD\A3\BB
	int templeft = xPos - inScale;
	int tempright = xPos + inScale;
	int left, right, up, down;//\C1\DA\D3\F2\B5\C4\D7\F3\D3\D2\C9\CF\CF±߽磻
	if (templeft>0 && tempright<picWidth)
	{
		left = inScale;
		right = inScale;
	}else
	{
		if (templeft<=0)
		{
			left = xPos;
		}else
		{
			left = inScale;
		}
		if (tempright>=picWidth)
		{
			right = picWidth - xPos - 1;
		}else
		{
			right = inScale;
		}
	}

	int temptop = yPos - inScale;
	int tempbottom = yPos + inScale;
	if ( temptop>0 && tempbottom<picHeight )
	{
		up = inScale;
		down = inScale;
	}else
	{
		if (temptop<=0)
		{
			up = yPos;
		}else
		{
			up = inScale;
		}
		if (tempbottom>=picHeight)
		{
			down = picHeight - yPos - 1;
		}else
		{
			down = inScale;
		}
	}

	outWidth = left + right + 1;
	outHeight = up + down + 1;

	LONG outlen = outWidth * outHeight;
	MyLUV* outarr = new MyLUV[outlen];

	LONG pos = 0;
	for (int x=0; x<outWidth; x++)
	{
		for (int y=0; y<outHeight; y++)
		{
			pos = ( y*outWidth + x );
			//\D2\D4\CF\C2\C7\F3\CA\E4\B3\F6\B5ĵ\DAx,y\B8\F6Ԫ\CB\D8\D4\DAԭ\CA\FD\D7\E9\D6е\C4X\BA\CDYλ\D6ã\BB
			int originx = xPos - left + x;
			int originy = yPos - up + y;
			//\D4\DA\CA\E4\C8\EB\B5\C4ͼ\CF\F1\CA\FD\BE\DD\D6е\C4λ\D6ã\BB
			LONG inpos = (originy * picWidth + originx); 
			outarr[pos].l = inLUVs[inpos].l;
			outarr[pos].u = inLUVs[inpos].u;
			outarr[pos].v = inLUVs[inpos].v;
		}
	}
	
	return outarr;
}

void  GetNearPixelsGreenExt(int xPos, int yPos
	    , BYTE* inPixels, int picWidth, int picHeight
	    , int radius, BYTE** outArr)
//\B5õ\BD\C1\DA\D3\F2\CF\F1\CB\D8ֵ(\D5\FD\B7\BD\D0\CE,Gͨ\B5\C0),\CA\E4\C8\EBλ\D6ô\D30\BF\AAʼ\BC\C6\CA\FD, \B1\DFԵ\B4\A6\B6Գ\C6\D1\D3\CDأ\BB
{
	int matrixwidth = (radius*2+1);
	BYTE* temparr = new BYTE[matrixwidth*matrixwidth];//\B0\FC\C0\A8ָ\B6\A8\B5\E3\D7\D4\C9\ED\A3\BB

	LONG pos = 0;
	int rposx, rposy;
	rposx = rposy = 0;//\D4\DAͼ\CF\F1\D6е\C4λ\D6ã\BB

	for (int y=-radius; y<=radius; y++)
	{
		rposy = yPos+y;
		if (rposy<0)
		{
			rposy = -rposy;
		}else if (rposy>=picHeight)
		{
			rposy = picHeight - (rposy-picHeight);
		}
		
		for (int x=-radius; x<=radius; x++)
		{
			rposx = xPos+x;
			if (rposx<0)
			{
				rposx = -rposx;
			}else if (rposx>=picWidth)
			{
				rposx = picWidth - (rposx-picWidth);
			}

			//\D4\DA\CA\E4\C8\EB\B5\C4ͼ\CF\F1\CA\FD\BE\DD\D6е\C4λ\D6ã\BB
			LONG inpos = ( rposy*picWidth + rposx ) * 3 + 1;//RGB\C8\FDɫֵ\A3\BB
			//\D4\DA\CA\E4\B3\F6\CA\FD\D7\E9\D6е\C4λ\D6ã\BB
			LONG pos = ( (y+radius)*matrixwidth + (x+radius) );
			temparr[pos] = ( inPixels[inpos] );
		}
	}

	*outArr = temparr;
}


void  GetNearPixelsExt(int xPos, int yPos
	, BYTE* inPixels, int picWidth, int picHeight
	, int radius, BYTE** outArr)
//\B5õ\BD\C1\DA\D3\F2\CF\F1\CB\D8ֵ(\D5\FD\B7\BD\D0\CE),\CA\E4\C8\EBλ\D6ô\D30\BF\AAʼ\BC\C6\CA\FD, \B1\DFԵ\B4\A6\D1\D3\CDأ\BB
{
	int matrixwidth = (radius*2+1);
	BYTE* temparr = new BYTE[matrixwidth*matrixwidth*3];//\B0\FC\C0\A8ָ\B6\A8\B5\E3\D7\D4\C9\ED\A3\BB

	LONG pos = 0;
	int rposx, rposy;
	rposx = rposy = 0;//\D4\DAͼ\CF\F1\D6е\C4λ\D6ã\BB

	for (int y=-radius; y<=radius; y++)
	{
		rposy = yPos+y;
		if (rposy<0)
		{
			rposy = -rposy;
		}else if (rposy>=picHeight)
		{
			rposy = picHeight - (rposy-picHeight);
		}
		
		for (int x=-radius; x<=radius; x++)
		{
			rposx = xPos+x;
			if (rposx<0)
			{
				rposx = -rposx;
			}else if (rposx>=picWidth)
			{
				rposx = picWidth - (rposx-picWidth);
			}

			//\D4\DA\CA\E4\C8\EB\B5\C4ͼ\CF\F1\CA\FD\BE\DD\D6е\C4λ\D6ã\BB
			LONG inpos = ( rposy*picWidth + rposx ) * 3;//RGB\C8\FDɫֵ\A3\BB
			//\D4\DA\CA\E4\B3\F6\CA\FD\D7\E9\D6е\C4λ\D6ã\BB
			LONG pos = ( (y+radius)*matrixwidth + (x+radius) ) * 3;
			temparr[pos] = inPixels[inpos];
			temparr[pos+1] = inPixels[inpos+1];
			temparr[pos+2] = inPixels[inpos+2];
		}
	}

	*outArr = temparr;
}


BYTE*  GetNearPixels(int xPos, int yPos, 
      BYTE* inPixels, int picWidth, int picHeight, int inScale, 
	  int& outWidth, int& outHeight)
//\B5õ\BD\C1\DA\D3\F2\CF\F1\CB\D8ֵ, \CA\E4\C8\EBλ\D6ô\D30\BF\AAʼ\BC\C6\CA\FD\A3\BB
{
	//\CA\D7\CFȼ\C6\CB\E3\BF\C9\C4ܵ\C4\C1\DA\D3\F2\B5\E3\CA\FD\A3\BB
	int templeft = xPos - inScale;
	int tempright = xPos + inScale;
	int left, right, up, down;//\C1\DA\D3\F2\B5\C4\D7\F3\D3\D2\C9\CF\CF±߽磻
	if (templeft>0 && tempright<picWidth)
	{
		//outWidth = inScale * 2 + 1;//\BC\D31\D4\F2\B0\FC\BA\AC\C1\CB\CF\F1\CBر\BE\C9\ED\A3\BB
		left = inScale;
		right = inScale;
	}else
	{
		if (templeft<=0)
		{
			//outWidth += xPos;//\D2\D4\D7\F3ȫ\D4\DA\C1\DA\D3\F2\C4ڣ\BB
			left = xPos;
		}else
		{
			left = inScale;
		}
		if (tempright>=picWidth)
		{
			//outWidth += picWidth - xPos - 1;//\D2\D4\D3\D2ȫ\D4\DA\C1\DA\D3\F2\C4ڣ\BB
			right = picWidth - xPos - 1;
		}else
		{
			right = inScale;
		}

/*
		if (outWidth>picWidth)
		{
			outWidth = picWidth;//\C1\DA\D3\F2\BF\ED\B6\C8\D3\EBͼ\CF\F1\BF\ED\CF\E0\B5ȣ\BB
		}
*/
	}

	int temptop = yPos - inScale;
	int tempbottom = yPos + inScale;
	if ( temptop>0 && tempbottom<picHeight )
	{
		//outWidth = inScale * 2 + 1;//\BC\D31\D4\F2\B0\FC\BA\AC\C1\CB\CF\F1\CBر\BE\C9\ED\A3\BB
		up = inScale;
		down = inScale;
	}else
	{
		if (temptop<=0)
		{
			up = yPos;
		}else
		{
			up = inScale;
		}
		if (tempbottom>=picHeight)
		{
			down = picHeight - yPos - 1;
		}else
		{
			down = inScale;
		}
/*
		if (outWidth>picWidth)
		{
			outWidth = picWidth;//\C1\DA\D3\F2\BF\ED\B6\C8\D3\EBͼ\CF\F1\BF\ED\CF\E0\B5ȣ\BB
		}
*/
	}

	outWidth = left + right + 1;
	outHeight = up + down + 1;

	LONG outlen = outWidth * outHeight;
	BYTE* outarr = new BYTE[outlen*3];

	LONG pos = 0;
	for (int x=0; x<outWidth; x++)
	{
		for (int y=0; y<outHeight; y++)
		{
			pos = ( y*outWidth + x ) * 3;//RGB\C8\FDɫֵ\A3\BB
			//\D2\D4\CF\C2\C7\F3\CA\E4\B3\F6\B5ĵ\DAx,y\B8\F6Ԫ\CB\D8\D4\DAԭ\CA\FD\D7\E9\D6е\C4X\BA\CDYλ\D6ã\BB
			int originx = xPos - left + x;
			int originy = yPos - up + y;
			//\D4\DA\CA\E4\C8\EB\B5\C4ͼ\CF\F1\CA\FD\BE\DD\D6е\C4λ\D6ã\BB
			LONG inpos = (originy * picWidth + originx) * 3; 
			outarr[pos] = inPixels[inpos];
			outarr[pos+1] = inPixels[inpos+1];
			outarr[pos+2] = inPixels[inpos+2];
		}
	}
	
	return outarr;
}
void GetGradient(BYTE* image, int width, int height
		, FLOAT* deltar, FLOAT* deltasita)
//\B5õ\BD\CA\E4\C8\EBͼ\CF\F1\B5\C4\CCݶȣ\BB
{
	//\CF\C2\C3\E6\BC\C6\CB\E3\B8\F7\CF\F1\CB\D8\D4\DAˮƽ\BAʹ\B9ֱ\B7\BD\CF\F2\C9ϵ\C4\CCݶ\C8,\B1\DFԵ\B5\E3\CCݶȼ\C6Ϊ0\A3\BB
	int* deltaxarr;
	int* deltayarr;
	int grawidth = width;
	int graheight = height;
	int deltacount = grawidth * graheight;
	deltaxarr = new int[deltacount];
	deltayarr = new int[deltacount];

    //\D4ݲ\BB\BC\C6\CB\E3\B1\DFԵ\B5㣻
	for (int y=1; y<graheight-1; y++)
	{
		for (int x=1; x<grawidth-1; x++)
		{
			int inarrpos = ((y)*width + (x))*3 + 1;//\D4\DA\CA\E4\C8\EB\BF\E9\D6е\C4λ\D6ã\BB
			int deltaarrpos = y*grawidth + x;//\D4\DA\CCݶ\C8\CA\FD\D7\E9\D6е\C4λ\D6ã\BB
			//\BE\ED\BB\FD\BC\C6\CB㣻
			deltaxarr[deltaarrpos] = (int) ( (
				image[((y-1)*width + (x+1))*3 + 1] //\D3\D2\C9\CF
				+ image[((y)*width + (x+1))*3 + 1] //\D3\D2
				+ image[((y+1)*width + (x+1))*3 + 1] //\D3\D2\CF\C2
				- image[((y-1)*width + (x-1))*3 + 1] //\D7\F3\C9\CF
				- image[((y)*width + (x-1))*3 + 1] //\D7\F3
				- image[((y+1)*width + (x-1))*3 + 1] ) / 3 );//\D7\F3\CF\C2
			deltayarr[deltaarrpos] = (int) ( ( 
				image[((y-1)*width + (x+1))*3 + 1] //\D3\D2\C9\CF
				+ image[((y-1)*width + (x))*3 + 1] //\C9\CF
				+ image[((y-1)*width + (x-1))*3 + 1] //\D7\F3\C9\CF
				- image[((y+1)*width + (x-1))*3 + 1] //\D7\F3\CF\C2
				- image[((y+1)*width + (x))*3 + 1] //\CF\C2
				- image[((y+1)*width + (x+1))*3 + 1]) / 3 );//\D3\D2\CF\C2
		}
	}

	//\B1\DFԵ\B8\B3Ϊ\C6\E4\C4ڲ\E0\B5\E3\B5\C4ֵ\A3\BB
	for (y=0; y<graheight; y++)
	{
		int x1 = 0;
		int pos1 = y*grawidth + x1;
		deltaxarr[pos1] = deltaxarr[pos1+1];
		deltayarr[pos1] = deltayarr[pos1+1];
		int x2 = grawidth-1;
		int pos2 = y*grawidth + x2;
		deltaxarr[pos2] = deltaxarr[pos2-1];
		deltayarr[pos2] = deltayarr[pos2-1];
	}
	for (int x=0; x<grawidth; x++)
	{
		int y1 = 0;
		int pos1 = x;
		int inner = x + grawidth;//\CF\C2һ\D0У\BB
		deltaxarr[pos1] = deltaxarr[inner];
		deltayarr[pos1] = deltayarr[inner];
		int y2 = graheight-1;
		int pos2 = y2*grawidth + x;
		inner = pos2 - grawidth;//\C9\CFһ\D0У\BB
		deltaxarr[pos2] = deltaxarr[inner];
		deltayarr[pos2] = deltayarr[inner];
	}


	for (y=0; y<graheight; y++)
	{
		for (int x=0; x<grawidth; x++)
		{
			int temppos = y*grawidth + x;
			if ( (deltaxarr[temppos])==0 )
			{
				if (deltayarr[temppos]!=0)
				{
					deltasita[temppos] = 0;//ˮƽ\B7\BD\CF\F2;
					deltar[temppos] = (FLOAT) abs(deltayarr[temppos]);
				}else
				{
					deltasita[temppos] = -1;//\CE\DEȷ\B6\A8\B7\BD\CF\F2;
					deltar[temppos] = (FLOAT) abs(deltayarr[temppos]);
				}
				continue;
			}
			deltasita[temppos] = (FLOAT) ( atan( 
				(FLOAT)deltayarr[temppos]
				/ (FLOAT)deltaxarr[temppos] ) + PI/2. );
			deltar[temppos] = (FLOAT) sqrt((DOUBLE) 
				( deltayarr[temppos]*deltayarr[temppos]
				+ deltaxarr[temppos]*deltaxarr[temppos] ) );
		}
	}

	delete [] deltaxarr; deltaxarr = NULL; //ɾ\B3\FDˮƽ\BAʹ\B9ֱ\CCݶ\C8\CA\FD\D7飻
	delete [] deltayarr; deltayarr = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Luc Vincent and Pierre Soille\B5ķ\D6ˮ\C1\EB\B7ָ\EEflood\B2\BD\D6\E8\B5\C4ʵ\CFִ\FA\C2룬 
// \D0޸\C4\D7\D4\CF\E0Ӧα\B4\FA\C2\EB, α\B4\FA\C2\EB\C0\B4\D7\D4\D7\F7\D5\DF\C2\DB\CEġ\B6Watersheds in Digital Spaces:
// An Efficient Algorithm Based on Immersion Simulations\A1\B7
// IEEE TRANSACTIONS ON PATTERN ANALYSIS AND MACHINE INTELLIGENCE.
// VOL.13, NO.6, JUNE 1991;
// by dzj, 2004.06.28 
// MyImageGraPt* imiarr - \CA\E4\C8\EB\B5\C4\C5\C5\D0\F2\BA\F3\CA\FD\D7\E9
// int* graddarr -------- \CA\E4\C8\EB\B5ĸ\F7\CCݶ\C8\CA\FD\D7飬\D3ɴ\CBֱ\BDӴ\E6ȡ\B8\F7H\CF\F1\CBص\E3
// int minh\A3\ACint maxh == \D7\EEС\D7\EE\B4\F3\CCݶ\C8
// int* flagarr --------- \CA\E4\B3\F6\B1\EA\BC\C7\CA\FD\D7\E9
// ע\D2⣺Ŀǰ\B2\BB\C9\E8\B7\D6ˮ\C1\EB\B1\EA\BCǣ\ACֻ\C9\E8ÿ\B8\F6\CF\F1\CB\D8\CB\F9\CA\F4\C7\F8\D3\F2\A3\BB
//////////////////////////////////////////////////////////////////////////
void  FloodVincent(MyImageGraPt* imiarr,int imageWidth,int imageHeight, int* graddarr, int minh, int maxh, int* flagarr, int& outrgnumber)
{
	const int INIT = -2;
	const int MASK = -1;
	const int WATERSHED = 0;
	int h = 0,i=0;
	int imagelen = imageWidth * imageHeight;
	for (i=0; i<imagelen; i++)
	{
		flagarr[i] = INIT;
	}
	//memset(flagarr, INIT, sizeof(int)*imagelen);
	int* imd = new int[imagelen];//\BE\E0\C0\EB\CA\FD\D7飬ֱ\BDӴ\E6ȡ\A3\BB
	for (i=0; i<imagelen; i++)
	{
		imd[i] = 0;
	}
	//memset(imd, 0, sizeof(int)*imagelen);
	std::queue <int> myqueue;
	int curlabel = 0;//\B8\F7\C5\E8\B5ر\EA\BCǣ\BB
	int ini;
	for (h=minh; h<=maxh; h++)
	{
		int stpos = graddarr[h];
		int edpos = graddarr[h+1];
		for (ini=stpos; ini<edpos; ini++)
		{
			int x = imiarr[ini].x;
			int y = imiarr[ini].y;
			int ipos = y*imageWidth + x;
			flagarr[ipos] = MASK;
			//\D2\D4\CF¼\EC\B2\E9\B8õ\E3\C1\DA\D3\F2\CAǷ\F1\D2ѱ\EA\BC\C7\CA\F4\D3\DAĳ\C7\F8\BB\F2\B7\D6ˮ\C1룬\C8\F4\CAǣ\AC\D4򽫸õ\E3\BC\D3\C8\EBfifo;
			int left = ipos - 1;
			if (x-1>=0) 
			{
				if (flagarr[left]>=0)
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
					continue;
				}				
			}
			int right = ipos + 1;
			if (x+1<imageWidth) 
			{
				if (flagarr[right]>=0) 
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
					continue;
				}
			}
			int up = ipos - imageWidth;
			if (y-1>=0) 
			{
				if (flagarr[up]>=0)
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
					continue;
				}				
			}
			int down = ipos + imageWidth;
			if (y+1<imageHeight)
			{
				if (flagarr[down]>=0) 
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
					continue;
				}			
			}
		}

    	//\D2\D4\CF¸\F9\BE\DD\CFȽ\F8\CFȳ\F6\B6\D3\C1\D0\C0\A9չ\CF\D6\D3\D0\C5\E8\B5أ\BB
		int curdist = 1; myqueue.push(-99);//\CC\D8\CA\E2\B1\EA\BCǣ\BB
		while (TRUE)
		{
			int p = myqueue.front();
			myqueue.pop();
			if (p == -99)
			{
				if ( myqueue.empty() )
				{
					break;
				}else
				{
					myqueue.push(-99);
					curdist = curdist + 1;
					p = myqueue.front();
					myqueue.pop();
				}
			}

			//\D2\D4\CF\C2\D5\D2p\B5\C4\C1\DA\D3\F2\A3\BB
			int y = (int) (p/imageWidth);
			int x = p - y*imageWidth;
			int left = p - 1;
			if  (x-1>=0)
			{
				if ( ( (imd[left]<curdist) && flagarr[left]>0)
					|| (flagarr[left]==0) ) 
				{
					if ( flagarr[left]>0 )
					{
						//ppei\CA\F4\D3\DAĳ\C7\F8\D3򣨲\BB\CAǷ\D6ˮ\C1룩\A3\BB
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//\BD\AB\C6\E4\C9\E8Ϊ\C1ڵ\E3\CB\F9\CA\F4\C7\F8\D3\F2\A3\BB
							flagarr[p] = flagarr[left];
						}else if (flagarr[p]!=flagarr[left])
						{
							//ԭ\C0\B4\B8\B3\B5\C4\C7\F8\D3\EB\CF\D6\D4ڸ\B3\B5\C4\C7\F8\B2\BBͬ\A3\AC\C9\E8Ϊ\B7\D6ˮ\C1룻
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppeiΪ\B7\D6\C1룻
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[left]==MASK) && (imd[left]==0) )
				//ppei\D6\D0\D2\D1MASK\B5ĵ㣬\B5\AB\C9\D0δ\B1\EA\BCǣ\A8\BC\B4\B2\BB\CA\F4ĳ\C7\F8Ҳ\B2\BB\CAǷ\D6ˮ\C1룩;
				{
					imd[left] = curdist + 1; myqueue.push(left);
				}
			}
			
			int right = p + 1;
			if (x+1<imageWidth) 
			{
				if ( ( (imd[right]<curdist) &&  flagarr[right]>0)
					|| (flagarr[right]==0) )
				{
					if ( flagarr[right]>0 )
					{
						//ppei\CA\F4\D3\DAĳ\C7\F8\D3򣨲\BB\CAǷ\D6ˮ\C1룩\A3\BB
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//\BD\AB\C6\E4\C9\E8Ϊ\C1ڵ\E3\CB\F9\CA\F4\C7\F8\D3\F2\A3\BB
							flagarr[p] = flagarr[right];
						}else if (flagarr[p]!=flagarr[right])
						{
							//ԭ\C0\B4\B8\B3\B5\C4\C7\F8\D3\EB\CF\D6\D4ڸ\B3\B5\C4\C7\F8\B2\BBͬ\A3\AC\C9\E8Ϊ\B7\D6ˮ\C1룻
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppeiΪ\B7\D6\C1룻
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[right]==MASK) && (imd[right]==0) )
					//ppei\D6\D0\D2\D1MASK\B5ĵ㣬\B5\AB\C9\D0δ\B1\EA\BCǣ\A8\BC\B4\B2\BB\CA\F4ĳ\C7\F8Ҳ\B2\BB\CAǷ\D6ˮ\C1룩;
				{
					imd[right] = curdist + 1; myqueue.push(right);
				}
			}
			
			int up = p - imageWidth;
			if (y-1>=0) 
			{
				if ( ( (imd[up]<curdist) &&  flagarr[up]>0)
					|| (flagarr[up]==0) )
				{
					if ( flagarr[up]>0 )
					{
						//ppei\CA\F4\D3\DAĳ\C7\F8\D3򣨲\BB\CAǷ\D6ˮ\C1룩\A3\BB
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//\BD\AB\C6\E4\C9\E8Ϊ\C1ڵ\E3\CB\F9\CA\F4\C7\F8\D3\F2\A3\BB
							flagarr[p] = flagarr[up];
						}else if (flagarr[p]!=flagarr[up])
						{
							//ԭ\C0\B4\B8\B3\B5\C4\C7\F8\D3\EB\CF\D6\D4ڸ\B3\B5\C4\C7\F8\B2\BBͬ\A3\AC\C9\E8Ϊ\B7\D6ˮ\C1룻
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppeiΪ\B7\D6\C1룻
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[up]==MASK) && (imd[up]==0) )
					//ppei\D6\D0\D2\D1MASK\B5ĵ㣬\B5\AB\C9\D0δ\B1\EA\BCǣ\A8\BC\B4\B2\BB\CA\F4ĳ\C7\F8Ҳ\B2\BB\CAǷ\D6ˮ\C1룩;
				{
					imd[up] = curdist + 1; myqueue.push(up);
				}
			}
			
			int down = p + imageWidth;
			if (y+1<imageHeight) 
			{
				if ( ( (imd[down]<curdist) &&  flagarr[down]>0)
					|| (flagarr[down]==0) )
				{
					if ( flagarr[down]>0 )
					{
						//ppei\CA\F4\D3\DAĳ\C7\F8\D3򣨲\BB\CAǷ\D6ˮ\C1룩\A3\BB
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//\BD\AB\C6\E4\C9\E8Ϊ\C1ڵ\E3\CB\F9\CA\F4\C7\F8\D3\F2\A3\BB
							flagarr[p] = flagarr[down];
						}else if (flagarr[p]!=flagarr[down])
						{
							//ԭ\C0\B4\B8\B3\B5\C4\C7\F8\D3\EB\CF\D6\D4ڸ\B3\B5\C4\C7\F8\B2\BBͬ\A3\AC\C9\E8Ϊ\B7\D6ˮ\C1룻
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppeiΪ\B7\D6\C1룻
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[down]==MASK) && (imd[down]==0) )
					//ppei\D6\D0\D2\D1MASK\B5ĵ㣬\B5\AB\C9\D0δ\B1\EA\BCǣ\A8\BCȲ\BB\CA\F4ĳ\C7\F8Ҳ\B2\BB\CAǷ\D6ˮ\C1룩;
				{
					imd[down] = curdist + 1; myqueue.push(down);
				}	
			}

		}//\D2\D4\C9\CF\CF\D6\D3\D0\C5\E8\B5ص\C4\C0\A9չ\A3\BB

		//\D2\D4\CF´\A6\C0\ED\D0·\A2\CFֵ\C4\C5\E8\B5أ\BB
		for ( ini=stpos; ini<edpos; ini++)
		{
			int x = imiarr[ini].x;
			int y = imiarr[ini].y;
			int ipos = y*imageWidth + x;
			imd[ipos] = 0;//\D6\D8\D6\C3\CB\F9\D3о\E0\C0\EB
			if (flagarr[ipos]==MASK)
			{
				//\BE\AD\B9\FDǰ\CA\F6\C0\A9չ\BA\F3\B8õ\E3\C8\D4ΪMASK\A3\AC\D4\F2\B8õ\E3\B1\D8Ϊ\D0\C2\C5\E8\B5ص\C4һ\B8\F6\C6\F0ʼ\B5\E3;
				curlabel = curlabel + 1;
				myqueue.push(ipos); 
				flagarr[ipos] = curlabel;
				
				while ( myqueue.empty()==FALSE )
				{
					int ppei = myqueue.front();
					myqueue.pop();
					int ppeiy = (int) (ppei/imageWidth);
			        int ppeix = ppei - ppeiy*imageWidth;
					
					int ppeileft = ppei - 1;
					if ( (ppeix-1>=0) && (flagarr[ppeileft]==MASK) )
					{
						myqueue.push(ppeileft);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
						flagarr[ppeileft] = curlabel;
					}
					int ppeiright = ppei + 1;
					if ( (ppeix+1<imageWidth) && (flagarr[ppeiright]==MASK) )
					{
						myqueue.push(ppeiright);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
						flagarr[ppeiright] = curlabel;
					}
					int ppeiup = ppei - imageWidth;
					if ( (ppeiy-1>=0) && (flagarr[ppeiup]==MASK) )
					{
						myqueue.push(ppeiup);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
						flagarr[ppeiup] = curlabel;
					}
					int ppeidown = ppei + imageWidth;
					if ( (ppeiy+1<imageHeight) && (flagarr[ppeidown]==MASK) )
					{
						myqueue.push(ppeidown);//\B5\E3λ\D6\C3ѹ\C8\EBfifo;
						flagarr[ppeidown] = curlabel;
					}					
				}				
			}
		}//\D2\D4\C9ϴ\A6\C0\ED\D0·\A2\CFֵ\C4\C5\E8\B5أ\BB

	}

	outrgnumber = curlabel;	
	delete [] imd; imd = NULL;
}

#define NearMeasureBias 200.0//\C5ж\A8\C7\F8\D3\F2\D1\D5ɫ\CF\E0\CBƵ\C4\E3\D0ֵ\A3\BB
void  MergeRgs(MyRgnInfo* rginfoarr, int rgnumber, int* flag, int width, int height, int* outmerge, int& rgnum)
//\BAϲ\A2\CF\E0\CB\C6\C7\F8\D3\F2\A3\BB
{
	//////////////////////////////////////////////////////////////////////////
	//1\A1\A2\BD\A8\C1\A2\B8\F7\C7\F8\B5\C4\C1\DA\D3\F2\CA\FD\D7飻
	//2\A1\A2\D2\C0\B4\CEɨ\C3\E8\B8\F7\C7\F8\D3\F2\A3\ACѰ\D5Ҽ\ABС\C7\F8\D3\F2\A3\BB
	//3\A1\A2\B6\D4ÿ\B8\F6\BC\ABС\C7\F8\A3\A8A\A3\A9\A3\AC\D4\DA\CF\E0\C1\DA\C7\F8\D6\D0\D5ҵ\BD\D7\EE\CF\E0\CB\C6\D5ߣ\BB
	//4\A1\A2\D3\EB\CF\E0\CB\C6\C7\F8\A3\A8B\A3\A9\BAϲ\A2\A3\A8\B8\F7\D6\D6\D0\C5Ϣˢ\D0£\A9\A3\AC\D4ڼ\ABС\C7\F8\A3\A8A\A3\A9\B5\C4\C1\DA\D3\F2\D6\D0
	//   ɾ\B3\FD\CF\E0\CB\C6\C7\F8\A3\A8B\A3\A9\A3\AC\D4\DA\C1\DA\D3\F2\CA\FD\D7\E9\D6\D0ɾ\B3\FD\CF\E0\CB\C6\C7\F8\A3\A8B\A3\A9\B6\D4Ӧ\B5\C4\CF\BD\AB
	//   \CF\E0\CB\C6\C7\F8\A3\A8B\A3\A9\B5\C4\CF\E0\C1\DA\C7\F8s\BCӵ\BD\BC\ABС\C7\F8\A3\A8A\A3\A9\B5\C4\C1\DA\D3\F2\D6\D0ȥ\A3\BB
	//5\A1\A2\BC\C7¼\BAϲ\A2\D0\C5Ϣ\A3\AC\C9\E8һ\CA\FD\D7\E9ר\C3Ŵ\E6\B7Ÿ\C3\D0\C5Ϣ\A3\AC\B8\C3\CA\FD\D7\E9\B5ĵ\DAA\B8\F6Ԫ\CB\D8ֵ\C9\E8ΪB\A3\BB
	//6\A1\A2\C5ж\CF\CAǷ\F1\C8\D4Ϊ\BC\ABС\C7\F8\A3\AC\C8\F4\CA\C7\D4򷵻\D83\A3\BB
	//7\A1\A2\CAǷ\F1\CB\F9\D3\D0\C7\F8\D3\F2\B6\BC\D2Ѵ\A6\C0\ED\CD\EA\B1ϣ\AC\C8\F4\B7\C7\D4򷵻\D82\A3\BB
	//
	//   \D3\C9\D3ڸ\F7\C7\F8\B5\C4\CF\E0\C1\DA\C7\F8\B2\BB\BB\E1̫\B6࣬\D2\F2\B4˲\C9\D3\C3\C1ڽ\D3\CA\FD\D7\E9\D7\F7Ϊ\B4洢\BDṹ\A3\BB
	//////////////////////////////////////////////////////////////////////////
	CString* neiarr = new CString[rgnumber+1];//\B5\DAһ\B8\F6\B2\BB\D3ã\BB
	int* mergearr = outmerge;//\BC\C7¼\BAϲ\A2\C7\E9\BF\F6\CA\FD\D7飻
	MyMath myMath;
	//\BD\A8\C1\A2\C1\DA\D3\F2\CA\FD\D7飻
	for (int y=0; y<height; y++)
	{
		int lstart = y * width;
		for (int x=0; x<width; x++)
		{
			int pos = lstart + x;
			int left=-1, right=-1, up=-1, down=-1;
			myMath.GetNeiInt(x, y, pos, width, height
		, left, right, up, down);//\D5\D2pos\B5\C4\CBĸ\F6\C1\DA\D3\F2\A3\BB
			//ȷ\B6\A8\B2\A2ˢ\D0\C2\C1\DA\D3\F2\C7\F8\D0\C5Ϣ\A3\BB
			int curid = flag[pos];
			AddNeiOfCur(curid, left, right
				, up, down, flag, neiarr);
		}
	}//\BD\A8\C1\A2\C1\DA\D3\F2\CA\FD\D7飻
	
	//\C7\F8\D3\F2\D0\C5Ϣ\CA\FD\D7\E9\D6е\C4\D3\D0Ч\D0\C5Ϣ\B4\D31\BF\AAʼ\A3\AC\B5\DAi\B8\F6λ\D6ô\E6\B7ŵ\DAi\B8\F6\C7\F8\D3\F2\B5\C4\D0\C5Ϣ\A3\BB
	for (int rgi=1; rgi<=rgnumber; rgi++)
	{
		//ɨ\C3\E8\CB\F9\D3\D0\C7\F8\D3\F2\A3\AC\D5Ҽ\ABС\C7\F8\A3\BB
		LONG allpoints = width * height;
		LONG nmin = (LONG) (allpoints / 400);
		int curid = rgi;

		//rginfoarr[rgi].isflag\B3\F5ʼΪFALSE\A3\AC\D4ڱ\BB\BAϲ\A2\B5\BD\C6\E4\CB\FC\C7\F8\BA\F3\B8\C4ΪTRUE\A3\BB
		while ( ( (rginfoarr[rgi].ptcount)<nmin ) 
			&& !rginfoarr[rgi].isflag )
		{
			//\B8\C3\C7\F8Ϊ\BC\ABС\C7\F8\A3\AC\B1\E9\C0\FA\CB\F9\D3\D0\CF\E0\C1\DA\C7\F8\A3\AC\D5\D2\D7\EE\BDӽ\FC\D5ߣ\BB
			CString neistr = neiarr[curid];
			int nearid = FindNearestNei(curid, neistr, rginfoarr, mergearr);
			//\BAϲ\A2curid\D3\EBnearid\A3\BB
			MergeTwoRgn(curid, nearid, neiarr
				, rginfoarr, mergearr);			
		} 
	}

	//\D2\D4\CF\C2\D4ٺϲ\A2\CF\E0\CB\C6\C7\F8\D3򣬣\A8\CE\DE\C2۴\F3С\A3\A9,\C8\E7\B9\FB\B2\BB\D0\E8Ҫ\A3\ACֱ\BDӽ\AB\D5\FB\B8\F6ѭ\BB\B7ע\CA͵\F4\BE\CD\D0\D0\C1ˣ\BB
	int countjjj = 0;
	//\C7\F8\D3\F2\D0\C5Ϣ\CA\FD\D7\E9\D6е\C4\D3\D0Ч\D0\C5Ϣ\B4\D31\BF\AAʼ\A3\AC\B5\DAi\B8\F6λ\D6ô\E6\B7ŵ\DAi\B8\F6\C7\F8\D3\F2\B5\C4\D0\C5Ϣ\A3\BB
	for (int ii=1; ii<=rgnumber; ii++)
	{
		if (!rginfoarr[ii].isflag)
		{
			int curid = ii;
			MergeNearest(curid, rginfoarr, neiarr, mergearr);
		}
	}



	int counttemp = 0;
	for (int i=0; i<rgnumber; i++)
	{
		if (!rginfoarr[i].isflag)
		{
			counttemp ++;
		}
	}

	rgnum = counttemp;

	delete [] neiarr; neiarr = NULL;
}
int FindMergedRgn(int idint, int* mergearr)
//\D5ҵ\BDidint\D7\EE\D6\D5\CB\F9\BAϲ\A2\B5\BD\B5\C4\C7\F8\BAţ\BB
{
	int outid = idint;
	while ( mergearr[outid] > 0 )
	{
		outid = mergearr[outid];
	}
	return outid;
}
void  MergeNearest(int curid, MyRgnInfo* rginfoarr, CString* neiarr, int* mergearr)
//\BAϲ\A2\CF\E0\CB\C6\C7\F8\D3\F2\A3\BB
{
	//\D2\C0\B4δ\A6\C0\ED\B8\F7\B8\F6\C1\DA\D3\F2\A3\AC\C8\F4\CF\E0\CBƣ\AC\D4\F2\BAϲ\A2\A3\BB
	//CString neistr = neiarr[curid];
	FLOAT cl, cu, cv;
	cl = rginfoarr[curid].l;//\B5\B1ǰ\C7\F8\B5\C4LUVֵ\A3\BB
	cu = rginfoarr[curid].u;
	cv = rginfoarr[curid].v;
	BOOL loopmerged = TRUE;//һ\B4\CEѭ\BB\B7\D6\D0\CAǷ\F1\D3кϲ\A2\B2\D9\D7\F7\B7\A2\C9\FA\A3\AC\C8\F4\CEޣ\AC\D4\F2\CD˳\F6ѭ\BB\B7\A3\BB

	while (loopmerged)
	{
		loopmerged = FALSE;
		CString tempstr = neiarr[curid];//\D3\C3\D3ڱ\BE\BA\AF\CA\FD\C4ڲ\BF\B4\A6\C0\ED\A3\BB
		while (tempstr.GetLength()>0)
		{
			int pos = tempstr.Find(" ");
			ASSERT(pos>=0);
			CString idstr = tempstr.Left(pos);
			tempstr.Delete(0, pos+1);
			
			int idint = (int) strtol(idstr, NULL, 10);
			//\C5жϸ\C3\C7\F8\CAǷ\F1\D2ѱ\BB\BAϲ\A2\A3\AC\C8\F4\CAǣ\AC\D4\F2һֱ\D5ҵ\BD\B8\C3\C7\F8\B5\B1ǰ\B5\C4\C7\F8\BAţ\BB
			idint = FindMergedRgn(idint, mergearr);
			if (idint==curid)
			{
				continue;//\D5\E2\B8\F6\C1\DA\C7\F8\D2ѱ\BB\BAϲ\A2\B5\BD\B5\B1ǰ\C7\F8\A3\AC\CC\F8\B9\FD\A3\BB
			}
			FLOAT tl, tu, tv;
			tl = rginfoarr[idint].l;//\B5\B1ǰ\B4\A6\C0\ED\B5\C4\C1\DA\C7\F8\B5\C4LUVֵ;
			tu = rginfoarr[idint].u;
			tv = rginfoarr[idint].v;
			DOUBLE tempdis = pow(tl-cl, 2) 
				+ pow(tu-cu, 2) + pow(tv-cv, 2);
			if (tempdis<NearMeasureBias)
			{
				MergeTwoRgn(curid, idint, neiarr, rginfoarr, mergearr);
				cl = rginfoarr[curid].l;//\B5\B1ǰ\C7\F8\B5\C4LUVֵˢ\D0£\BB
				cu = rginfoarr[curid].u;
				cv = rginfoarr[curid].v;
				loopmerged = TRUE;
			}		
		}
	}
}

void  MergeTwoRgn(int curid, int nearid
	, CString* neiarr, MyRgnInfo* rginfoarr, int* mergearr)
//\BD\ABnearid\BAϲ\A2\B5\BDcurid\D6\D0ȥ\A3\AC\B8\FC\D0ºϲ\A2\BA\F3\B5\C4\C7\F8\D0\C5Ϣ\A3\AC\B2\A2\BC\C7¼\B8úϲ\A2\A3\BB
{
	//\BD\AB\C7\F8\D0\C5Ϣ\D6\D0nearid\B6\D4Ӧ\CF\EE\B5ı\EA\BC\C7\C9\E8Ϊ\D2ѱ\BB\BAϲ\A2\A3\BB
	rginfoarr[nearid].isflag = TRUE;
	//\B8\FC\D0ºϲ\A2\BA\F3\B5\C4LUV\D0\C5Ϣ\A3\BB
	LONG ptincur = rginfoarr[curid].ptcount;
	LONG ptinnear = rginfoarr[nearid].ptcount;
	DOUBLE curpercent = (FLOAT)ptincur / (FLOAT)(ptincur+ptinnear);
	rginfoarr[curid].ptcount = ptincur + ptinnear;
	rginfoarr[curid].l = (FLOAT) ( curpercent * rginfoarr[curid].l
		+ (1-curpercent) * rginfoarr[nearid].l );
	rginfoarr[curid].u = (FLOAT) ( curpercent * rginfoarr[curid].u
		+ (1-curpercent) * rginfoarr[nearid].u );
	rginfoarr[curid].v = (FLOAT) ( curpercent * rginfoarr[curid].v
		+ (1-curpercent) * rginfoarr[nearid].v );
	//\BD\ABnearid\B5\C4\C1\DA\D3\F2\BCӵ\BDcurid\B5\C4\C1\DA\D3\F2\D6\D0ȥ\A3\BB
	AddBNeiToANei(curid, nearid, neiarr, mergearr);
	//\BC\C7¼\B8úϲ\A2\A3\BB
	mergearr[nearid] = curid;
}

void  AddBNeiToANei(int curid, int nearid, CString* neiarr, int* mergearr)
//\BD\ABnearid\B5\C4\C1\DA\D3\F2\BCӵ\BDcurid\B5\C4\C1\DA\D3\F2\D6\D0ȥ\A3\BB
{
	//\CFȴ\D3curid\B5\C4\C1\DA\C7\F8\D6а\D1nearidɾȥ\A3\BB
/*
	CString tempstr;
	tempstr.Format("%d ", nearid);
	int temppos = neiarr[curid].Find(tempstr, 0);
	while (temppos>0 && neiarr[curid].GetAt(temppos-1)!=' ')
	{
		temppos = neiarr[curid].Find(tempstr, temppos+1);
	}
	if (temppos>=0)
	{
		//\B7\F1\D4\F2\C1ڽ\FC\C7\F8Ϊ\BAϲ\A2\B9\FD\C0\B4\B5\C4\C7\F8\A3\AC\BA\F6\C2ԣ\BB
		neiarr[curid].Delete(temppos, tempstr.GetLength());
	}
*/
    //\BD\ABnearid\B5\C4\C1\DA\C7\F8\D2\C0\B4μӵ\BDcurid\B5\C4\C1\DA\C7\F8\D6\D0ȥ\A3\BB
	CString neistr = neiarr[nearid];
	CString curstr = neiarr[curid];
	//һ\B0\E3˵\C0\B4\A3\AC\BC\ABС\C7\F8\B5\C4\C1\DA\D3\F2Ӧ\B8ý\CF\C9٣\AC\D2\F2\B4ˣ\ACΪ\D7\C5\CC\E1\B8ߺϲ\A2\CBٶȣ\AC\BD\AB
	//curstr\BCӵ\BDneistr\D6\D0ȥ\A3\ACȻ\BA󽫽\E1\B9\FB\B8\B3\B8\F8neiarr[curid];
	while ( curstr.GetLength()>0 )
	{
		int pos = curstr.Find(" ");		
		ASSERT(pos>=0);
		CString idstr = curstr.Left(pos);
        curstr.Delete(0, pos+1);
		int idint = (int) strtol(idstr, NULL, 10);
		idint = FindMergedRgn(idint, mergearr);
		idstr += " ";
		if ( (idint == curid) || (idint == nearid) )
		{
			continue;//\B1\BE\C7\F8\B2\BB\D3뱾\C7\F8\CF\E0\C1ڣ\BB
		}else
		{
			if ( neistr.Find(idstr, 0) >= 0 )
			{
				continue;
			}else
			{
				neistr += idstr;//\BCӵ\BD\C1\DA\C7\F8\D6\D0ȥ;
			}
		}		
	}
	neiarr[curid] = neistr;
/*
	CString toaddneis = neiarr[nearid];
	while (toaddneis.GetLength()>0)
	{
		int pos = toaddneis.Find(" ");		
		ASSERT(pos>=0);
		CString idstr = toaddneis.Left(pos);
        toaddneis.Delete(0, pos+1);
		int idint = (int) strtol(idstr, NULL, 10);
		idint = FindMergedRgn(idint, mergearr);
		idstr += " ";
		if ( (idint == curid) || (idint == nearid) )
		{
			continue;//\B1\BE\C7\F8\B2\BB\D3뱾\C7\F8\CF\E0\C1ڣ\BB
		}else
		{
			if ( neiarr[curid].Find(idstr, 0) >= 0 )
			{
				continue;
			}else
			{
				neiarr[curid] += idstr;//\BCӵ\BD\C1\DA\C7\F8\D6\D0ȥ;
			}
		}		
	}
*/
}


int  FindNearestNei(int curid, CString neistr, MyRgnInfo* rginfoarr, int* mergearr)
//Ѱ\D5\D2neistr\D6\D0\D3\EBcurid\D7\EE\BDӽ\FC\B5\C4\C7\F8\A3\AC\B7\B5\BBظ\C3\C7\F8id\BAţ\BB
{
	int outid = -1;
	DOUBLE mindis = 999999;
	FLOAT cl, cu, cv;
	cl = rginfoarr[curid].l;//\B5\B1ǰ\C7\F8\B5\C4LUVֵ\A3\BB
	cu = rginfoarr[curid].u;
	cv = rginfoarr[curid].v;

	CString tempstr = neistr;//\D3\C3\D3ڱ\BE\BA\AF\CA\FD\C4ڲ\BF\B4\A6\C0\ED\A3\BB
	while (tempstr.GetLength()>0)
	{
		int pos = tempstr.Find(" ");
		ASSERT(pos>=0);
		CString idstr = tempstr.Left(pos);
		tempstr.Delete(0, pos+1);

		int idint = (int) strtol(idstr, NULL, 10);
		//\C5жϸ\C3\C7\F8\CAǷ\F1\D2ѱ\BB\BAϲ\A2\A3\AC\C8\F4\CAǣ\AC\D4\F2һֱ\D5ҵ\BD\B8\C3\C7\F8\B5\B1ǰ\B5\C4\C7\F8\BAţ\BB
		idint = FindMergedRgn(idint, mergearr);
		if (idint==curid)
		{
			continue;//\D5\E2\B8\F6\C1\DA\C7\F8\D2ѱ\BB\BAϲ\A2\B5\BD\B5\B1ǰ\C7\F8\A3\AC\CC\F8\B9\FD\A3\BB
		}
		FLOAT tl, tu, tv;
		tl = rginfoarr[idint].l;//\B5\B1ǰ\B4\A6\C0\ED\B5\C4\C1\DA\C7\F8\B5\C4LUVֵ;
		tu = rginfoarr[idint].u;
		tv = rginfoarr[idint].v;
		DOUBLE tempdis = pow(tl-cl, 2) 
			+ pow(tu-cu, 2) + pow(tv-cv, 2);
		if (tempdis<mindis)
		{
			mindis = tempdis;//\D7\EE\B4\F3\BE\E0\C0\EB\BAͶ\D4Ӧ\B5\C4\CF\E0\C1\DA\C7\F8ID\A3\BB
			outid = idint;
		}		
	}

	return outid;
}

int  FindMergedRgnMaxbias(int idint, int* mergearr, int bias)
//\B4\F3\E3\D0ֵ\D6\D5ֹ\B2\E9\D5Һϲ\A2\C7\F8\A3\AC\D3\C3\D3\DAcoarse watershed, 
//\B5\F7\D3\C3\D5߱\D8\D0뱣֤idint\D3\D0Ч\A3\AC\BC\B4\A3\BAmergearr[idint]>0\A3\BB
//\D2Լ\B0mergearr\D3\D0Ч\A3\AC\BC\B4\A3\BAmergearr[idint]<idint;
{
	int outid = idint;
	while ( mergearr[outid]<bias )
	{
		outid = mergearr[outid];
	}
	return mergearr[outid];
}


void  AddNeiRgn(int curid, int neiid, CString* neiarr)
//\D4\F6\BC\D3neiidΪcurid\B5\C4\CF\E0\C1\DA\C7\F8
{
	CString tempneis = neiarr[curid];//\B5\B1ǰ\B5\C4\CF\E0\C1\DA\C7\F8\A3\BB
	CString toaddstr;
	toaddstr.Format("%d ", neiid);

	int temppos = tempneis.Find(toaddstr, 0);
	while (temppos>0 && neiarr[curid].GetAt(temppos-1)!=' ')
	{
		temppos = neiarr[curid].Find(toaddstr, temppos+1);
	}
	
	if ( temppos<0 )
	{
		//\B5\B1ǰ\CF\E0\C1\DA\C7\F8\D6\D0û\D3\D0tempneis,\D4\F2\BC\D3\C8\EB
		neiarr[curid] += toaddstr;
	}
}

void  AddNeiOfCur(int curid, int left, int right, int up, int down, int* flag, CString* neiarr)
//ˢ\D0µ\B1ǰ\B5\E3\B5\C4\CB\F9\D3\D0\CF\E0\C1\DA\C7\F8\A3\BB
{
	int leftid, rightid, upid, downid;
	leftid = rightid = upid = downid = curid;
	if (left>=0)
	{
		leftid = flag[left];
		if (leftid!=curid)
		{
			//\C1ڵ\E3\CA\F4\D3\DA\C1\EDһ\C7\F8, \BC\D3\C1\DA\D3\F2\B5\E3\D0\C5Ϣ\A3\BB
			AddNeiRgn(curid, leftid, neiarr);
		}
	}
	if (right>0)
	{
		rightid = flag[right];
		if (rightid!=curid)
		{
			//\C1ڵ\E3\CA\F4\D3\DA\C1\EDһ\C7\F8, \BC\D3\C1\DA\D3\F2\B5\E3\D0\C5Ϣ\A3\BB
			AddNeiRgn(curid, rightid, neiarr);
		}
	}
	if (up>=0)
	{
		upid = flag[up];
		if (upid!=curid)
		{
			//\C1ڵ\E3\CA\F4\D3\DA\C1\EDһ\C7\F8, \BC\D3\C1\DA\D3\F2\B5\E3\D0\C5Ϣ\A3\BB
			AddNeiRgn(curid, upid, neiarr);
		}
	}
	if (down>0)
	{
		downid = flag[down];
		if (downid!=curid)
		{
			//\C1ڵ\E3\CA\F4\D3\DA\C1\EDһ\C7\F8, \BC\D3\C1\DA\D3\F2\B5\E3\D0\C5Ϣ\A3\BB
			AddNeiRgn(curid, downid, neiarr);
		}
	}
}
/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   RegionSegFixThreshold()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib					- ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   int nThreshold					- \C7\F8\D3\F2\B7ָ\EE\B5\C4\E3\D0ֵ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   1\A3\A8\C2߼\AD\A3\A9\B1\EDʾ\B6\D4Ӧ\CF\F3\CB\D8Ϊǰ\BE\B0\C7\F8\D3\F2\A3\AC0\B1\EDʾ\B1\B3\BE\B0
 *   \E3\D0ֵ\B7ָ\EE\B5Ĺؼ\FC\CE\CA\CC\E2\D4\DA\D3\DA\E3\D0ֵ\B5\C4ѡȡ\A1\A3\E3\D0ֵ\B5\C4ѡȡһ\B0\E3Ӧ\B8\C3\CA\D3ʵ\BCʵ\C4Ӧ\D3ö\F8
 *   \C1\E9\BB\EE\C9趨\A1\A3
 *
 *************************************************************************
 */
/*void RegionSegFixThreshold(CDIB * pDib, int nThreshold)
{
	//\B1\E9\C0\FAͼ\CF\F3\B5\C4\D7\DD\D7\F8\B1\EA
	int y;

	//\B1\E9\C0\FAͼ\CF\F3\B5ĺ\E1\D7\F8\B1\EA
	int x;

	//ͼ\CF\F3\B5ĳ\A4\BF\ED\B4\F3С
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	//ͼ\CF\F1\D4ڼ\C6\CB\E3\BB\FA\D4ڴ洢\D6е\C4ʵ\BCʴ\F3С
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	//ͼ\CF\F1\D4\DA\C4ڴ\E6\D6\D0ÿһ\D0\D0\CF\F3\CB\D8ռ\D3õ\C4ʵ\BCʿռ\E4
	int nSaveWidth = sizeImageSave.cx;

	
	//ͼ\CF\F1\CA\FD\BEݵ\C4ָ\D5\EB
	LPBYTE  pImageData = pDib->m_lpImage;

	for(y=0; y<nHeight ; y++ )
		for(x=0; x<nWidth ; x++ )
		{
			if( *(pImageData+y*nSaveWidth+x) < nThreshold)
				*(pImageData+y*nSaveWidth+x) = 0;
		}
}*/

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   RegionSegAdaptive()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib					- ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   1\A3\A8\C2߼\AD\A3\A9\B1\EDʾ\B6\D4Ӧ\CF\F3\CB\D8Ϊǰ\BE\B0\C7\F8\D3\F2\A3\AC0\B1\EDʾ\B1\B3\BE\B0
 *   \E3\D0ֵ\B7ָ\EE\B5Ĺؼ\FC\CE\CA\CC\E2\D4\DA\D3\DA\E3\D0ֵ\B5\C4ѡȡ\A1\A3\E3\D0ֵ\B5\C4ѡȡһ\B0\E3Ӧ\B8\C3\CA\D3ʵ\BCʵ\C4Ӧ\D3ö\F8
 *   \C1\E9\BB\EE\C9趨\A1\A3\B1\BE\BA\AF\CA\FD\D6У\AC\E3\D0ֵ\B2\BB\CAǹ̶\A8\B5ģ\AC\B6\F8\CAǸ\F9\BE\DDͼ\CF\F3\CF\F3\CBص\C4ʵ\BC\CA\D0\D4\D6ʶ\F8\C9趨\B5ġ\A3
 *   \D5\E2\B8\F6\BA\AF\CA\FD\B0\D1ͼ\CF\F1\B7ֳ\C9\CBĸ\F6\D7\D3ͼ\CF\F3\A3\ACȻ\BA\F3\BC\C6\CB\E3ÿ\B8\F6\D7\D3ͼ\CF\F3\B5ľ\F9ֵ\A3\AC\B8\F9\BEݾ\F9ֵ\C9\E8\D6\C3\E3\D0ֵ
 *   \E3\D0ֵֻ\CA\C7Ӧ\D3\C3\D4ڶ\D4Ӧ\B5\C4\D7\D3ͼ\CF\F3
 *
 *************************************************************************
 */
/*void RegionSegAdaptive(CDIB * pDib)
{
	//\B1\E9\C0\FAͼ\CF\F3\B5\C4\D7\DD\D7\F8\B1\EA
	int y;

	//\B1\E9\C0\FAͼ\CF\F3\B5ĺ\E1\D7\F8\B1\EA
	int x;

	//ͼ\CF\F3\B5ĳ\A4\BF\ED\B4\F3С
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	//ͼ\CF\F1\D4ڼ\C6\CB\E3\BB\FA\D4ڴ洢\D6е\C4ʵ\BCʴ\F3С
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	//ͼ\CF\F1\D4\DA\C4ڴ\E6\D6\D0ÿһ\D0\D0\CF\F3\CB\D8ռ\D3õ\C4ʵ\BCʿռ\E4
	int nSaveWidth = sizeImageSave.cx;

	//ͼ\CF\F1\CA\FD\BEݵ\C4ָ\D5\EB
	LPBYTE  lpImage = pDib->m_lpImage;

	// \BEֲ\BF\E3\D0ֵ
	int nThd[2][2] ;

	// \D7\D3ͼ\CF\F3\B5\C4ƽ\BE\F9ֵ
	int nLocAvg ;
	
	// \B6\D4\D7\F3\C9\CFͼ\CF\F1\D6\F0\B5\E3ɨ\C3裺
	nLocAvg = 0 ;
	// y\B7\BD\CF\F2
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=0; x<nWidth/2 ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// \BC\C6\CB\E3\BE\F9ֵ
	nLocAvg /= ( (nHeight/2) * (nWidth/2) ) ;

	// \C9\E8\D6\C3\E3\D0ֵΪ\D7\D3ͼ\CF\F3\B5\C4ƽ\BE\F9ֵ
	nThd[0][0] = nLocAvg ;

	// \B6\D4\D7\F3\C9\CFͼ\CF\F1\D6\F0\B5\E3ɨ\C3\E8\BD\F8\D0зָ
	// y\B7\BD\CF\F2
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=0; x<nWidth/2 ; x++ )
		{
			if(lpImage[y*nSaveWidth + x]<nThd[0][0])
				lpImage[y*nSaveWidth + x] = 255 ;
			else
			{
				lpImage[y*nSaveWidth + x] = 0 ;
			}
			
		}
	}

	// =============================================
	// \B6\D4\D7\F3\CF\C2ͼ\CF\F1\D6\F0\B5\E3ɨ\C3裺
	nLocAvg = 0 ;
	// y\B7\BD\CF\F2
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=0; x<nWidth/2 ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// \BC\C6\CB\E3\BE\F9ֵ
	nLocAvg /= ( (nHeight - nHeight/2) * (nWidth/2) ) ;

	// \C9\E8\D6\C3\E3\D0ֵΪ\D7\D3ͼ\CF\F3\B5\C4ƽ\BE\F9ֵ
	nThd[1][0] = nLocAvg ;

	// \B6\D4\D7\F3\CF\C2ͼ\CF\F1\D6\F0\B5\E3ɨ\C3\E8\BD\F8\D0зָ
	// y\B7\BD\CF\F2
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=0; x<nWidth/2 ; x++ )
		{
			if(lpImage[y*nSaveWidth + x]<nThd[1][0])
				lpImage[y*nSaveWidth + x] = 255 ;
			else
			{
				lpImage[y*nSaveWidth + x] = 0 ;
			}
			
		}
	}

	// =============================================
	// \B6\D4\D3\D2\C9\CFͼ\CF\F1\D6\F0\B5\E3ɨ\C3裺
	nLocAvg = 0 ;
	// y\B7\BD\CF\F2
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=nWidth/2; x<nWidth ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// \BC\C6\CB\E3\BE\F9ֵ
	nLocAvg /= ( (nHeight/2) * (nWidth - nWidth/2) ) ;
	
	// \C9\E8\D6\C3\E3\D0ֵΪ\D7\D3ͼ\CF\F3\B5\C4ƽ\BE\F9ֵ
	nThd[0][1] = nLocAvg ;

	// \B6\D4\D3\D2\C9\CFͼ\CF\F1\D6\F0\B5\E3ɨ\C3\E8\BD\F8\D0зָ
	// y\B7\BD\CF\F2
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=nWidth/2; x<nWidth ; x++ )
		{
			if(lpImage[y*nSaveWidth + x]<nThd[0][1])
				lpImage[y*nSaveWidth + x] = 255 ;
			else
			{
				lpImage[y*nSaveWidth + x] = 0 ;
			}
			
		}
	}

	// =============================================
	// \B6\D4\D3\D2\CF\C2ͼ\CF\F1\D6\F0\B5\E3ɨ\C3裺
	nLocAvg = 0 ;
	// y\B7\BD\CF\F2
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=nWidth/2; x<nWidth ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// \BC\C6\CB\E3\BE\F9ֵ
	nLocAvg /= ( (nHeight - nHeight/2) * (nWidth - nWidth/2) ) ;

	// \C9\E8\D6\C3\E3\D0ֵΪ\D7\D3ͼ\CF\F3\B5\C4ƽ\BE\F9ֵ
	nThd[1][1] = nLocAvg ;

	// \B6\D4\D3\D2\CF\C2ͼ\CF\F1\D6\F0\B5\E3ɨ\C3\E8\BD\F8\D0зָ
	// y\B7\BD\CF\F2
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=nWidth/2; x<nWidth ; x++ )
		{
			if(lpImage[y*nSaveWidth + x]<nThd[1][1])
				lpImage[y*nSaveWidth + x] = 255 ;
			else
			{
				lpImage[y*nSaveWidth + x] = 0 ;
			}
		}
	}
	
	// Ϊ\C1\CB\CF\D4ʾ\B7\BD\B1\E3\CF\D4ʾ\A3\AC\C2߼\AD1\D3ú\DAɫ\CF\D4ʾ\A3\AC\C2߼\AD0\D3ð\D7ɫ\CF\D4ʾ
	for(y=0; y<nHeight ; y++ )
	{
		// x\B7\BD\CF\F2
		for(x=0; x<nWidth ; x++ )
		{
			lpImage[y*nSaveWidth + x] = 255 - lpImage[y*nSaveWidth + x] ;
		}
	}
}*/

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   RobertsOperator()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib		- ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   double * pdGrad	- ָ\CF\F2\CCݶ\C8\CA\FD\BEݵ\C4ָ\D5룬\BA\AC\D3\D0ͼ\CF\F1\B5\C4\CCݶ\C8\D0\C5Ϣ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   Roberts\CB\E3\D7\D3
 *
 *************************************************************************
 */
/*void RobertsOperator(CDIB * pDib, double * pdGrad)
{
	// \B1\E9\C0\FAͼ\CF\F3\B5\C4\D7\DD\D7\F8\B1\EA
	int y;

	// \B1\E9\C0\FAͼ\CF\F3\B5ĺ\E1\D7\F8\B1\EA
	int x;

	// ͼ\CF\F3\B5ĳ\A4\BF\ED\B4\F3С
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	// ͼ\CF\F1\D4ڼ\C6\CB\E3\BB\FA\D4ڴ洢\D6е\C4ʵ\BCʴ\F3С
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	// ͼ\CF\F1\D4\DA\C4ڴ\E6\D6\D0ÿһ\D0\D0\CF\F3\CB\D8ռ\D3õ\C4ʵ\BCʿռ\E4
	int nSaveWidth = sizeImageSave.cx;

	// ͼ\CF\F1\CA\FD\BEݵ\C4ָ\D5\EB
	LPBYTE  pImageData = pDib->m_lpImage;

	// \B3\F5ʼ\BB\AF
	for(y=0; y<nHeight ; y++ )
		for(x=0 ; x<nWidth ; x++ )
		{
			*(pdGrad+y*nWidth+x)=0;
		}


	// \CF\C2\C3濪ʼ\C0\FB\D3\C3Roberts\CB\E3\D7ӽ\F8\D0м\C6\CB㣬Ϊ\C1˱\A3֤\BC\C6\CB\E3\CB\F9\D0\E8Ҫ\B5\C4
	// \B5\C4\CA\FD\BE\DDλ\D3\DAͼ\CF\F1\CA\FD\BEݵ\C4\C4ڲ\BF\A3\AC\CF\C2\C3\E6\B5\C4\C1\BD\D6\D8ѭ\BB\B7\B5\C4\CC\F5\BC\FE\CA\C7
	// y<nHeight-1 \B6\F8\B2\BB\CA\C7y<nHeight\A3\AC\CF\E0Ӧ\B5\C4x\B7\BD\CF\F2Ҳ\CA\C7x<nWidth-1
	// \B6\F8\B2\BB\CA\C7x<nWidth

	//\D5\E2\C1\BD\B8\F6\B1\E4\C1\BF\D3\C3\C0\B4\B1\EDʾRoberts\CB\E3\D7ӵ\DAһ\B8\F6ģ\B0\E5\B5\C4\C1\BD\B8\F6\CF\F3\CB\D8ֵ
	int nUpLeft;
	int nDownRight;

	// \D5\E2\C1\BD\B8\F6\B1\E4\C1\BF\D3\C3\C0\B4\B1\EDʾRoberts\CB\E3\D7ӵڶ\FE\B8\F6ģ\B0\E5\B5\C4\C1\BD\B8\F6\CF\F3\CB\D8ֵ
	int nUpRight;
	int nDownLeft;

	// \D5\E2\C1\BD\B8\F6\B1\E4\C1\BF\D3\C3\C0\B4\B1\EDʾRoberts\CB\E3\D7Ӽ\C6\CB\E3\B5Ľ\E1\B9\FB
	int nValueOne;
	int nValueTwo;

	// \C1\D9ʱ\B1\E4\C1\BF
	double dGrad;

	for(y=0; y<nHeight-1 ; y++ )
		for(x=0 ; x<nWidth-1 ; x++ )
		{
			// Roberts\CB\E3\D7ӵ\DAһ\B8\F6ģ\B0\E5\D0\E8Ҫ\B5\C4\CF\F3\CB\D8ֵ
			nUpLeft		=*(pImageData+y*nSaveWidth+x) ; 
			nDownRight	=*(	pImageData+(y+1)*nSaveWidth+x+1	);
			nDownRight	*=-1;

			//Roberts\CB\E3\D7ӵĵ\DAһ\B8\F6ģ\B0\E5\BC\C6\CB\E3\BD\E1\B9\FB
			nValueOne	=nUpLeft+nDownRight	;

			// Roberts\CB\E3\D7ӵڶ\FE\B8\F6ģ\B0\E5\D0\E8Ҫ\B5\C4\CF\F3\CB\D8ֵ
			nUpRight	=*(	pImageData+y*nSaveWidth+x+1	)		;
			nDownLeft	=*(	pImageData+(y+1)*nSaveWidth+x	);
			nDownLeft	*=-1;

			// Roberts\CB\E3\D7ӵĵڶ\FE\B8\F6ģ\B0\E5\BC\C6\CB\E3\BD\E1\B9\FB
			nValueTwo	=nUpRight+nDownLeft;

			// \BC\C6\CB\E3\C1\BD\B8\F6ƫ\B5\BC\CA\FD\B5\C4ƽ\B7\BD\BA\CD
			dGrad=nValueOne*nValueOne + nValueTwo*nValueTwo;

			// \BF\AA\B7\BD
			dGrad=pow(dGrad,0.5);

			// \B7\B6\CA\FD\B2\C9\D3\C3ŷʽ\BE\E0\C0\EB
			*(pdGrad+y*nWidth+x)=dGrad;
		}
}*/
/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   LaplacianOperator()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib		- ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   double * pdGrad	- ָ\CF\F2\CCݶ\C8\CA\FD\BEݵ\C4ָ\D5룬\BA\AC\D3\D0ͼ\CF\F1\B5\C4\CCݶ\C8\D0\C5Ϣ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   LaplacianOperator\CB\E3\D7\D3,\CAǶ\FE\BD\D7\CB\E3\D7ӣ\AC\B2\BB\CF\EBRoberts\CB\E3\D7\D3\C4\C7\D1\F9\D0\E8Ҫ\C1\BD\B8\F6ģ\B0\E5\BC\C6\CB\E3
 *   \CCݶȣ\ACLaplacianOperator\CB\E3\D7\D3ֻҪһ\B8\F6\CB\E3\D7ӾͿ\C9\D2Լ\C6\CB\E3\CCݶȡ\A3\B5\AB\CA\C7\D2\F2Ϊ\C0\FB\D3\C3\C1\CB
 *   \B6\FE\BD\D7\D0\C5Ϣ\A3\AC\B6\D4\D4\EB\C9\F9\B1Ƚ\CF\C3\F4\B8\D0
 *
 *************************************************************************
 */
/*void LaplacianOperator(CDIB * pDib, double * pdGrad)
{
		// \B1\E9\C0\FAͼ\CF\F3\B5\C4\D7\DD\D7\F8\B1\EA
	int y;

	// \B1\E9\C0\FAͼ\CF\F3\B5ĺ\E1\D7\F8\B1\EA
	int x;

	// ͼ\CF\F3\B5ĳ\A4\BF\ED\B4\F3С
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	// ͼ\CF\F1\D4ڼ\C6\CB\E3\BB\FA\D4ڴ洢\D6е\C4ʵ\BCʴ\F3С
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	// ͼ\CF\F1\D4\DA\C4ڴ\E6\D6\D0ÿһ\D0\D0\CF\F3\CB\D8ռ\D3õ\C4ʵ\BCʿռ\E4
	int nSaveWidth = sizeImageSave.cx;

	// ͼ\CF\F1\CA\FD\BEݵ\C4ָ\D5\EB
	LPBYTE  lpImage = pDib->m_lpImage;

	// \B3\F5ʼ\BB\AF
	for(y=0; y<nHeight ; y++ )
		for(x=0 ; x<nWidth ; x++ )
		{
			*(pdGrad+y*nWidth+x)=0;
		}

	// \C9\E8\D6\C3ģ\B0\E5ϵ\CA\FD
	static int nWeight[3][3] ;
	nWeight[0][0] = -1 ;   
	nWeight[0][1] = -1 ;   
	nWeight[0][2] = -1 ;   
	nWeight[1][0] = -1 ;   
	nWeight[1][1] =  8 ;   
	nWeight[1][2] = -1 ;   
	nWeight[2][0] = -1 ;   
	nWeight[2][1] = -1 ;   
	nWeight[2][2] = -1 ;   



	//\D5\E2\B8\F6\B1\E4\C1\BF\D3\C3\C0\B4\B1\EDʾLaplacian\CB\E3\D7\D3\CF\F3\CB\D8ֵ
	int nTmp[3][3];
	
	// \C1\D9ʱ\B1\E4\C1\BF
	double dGrad;

	// ģ\B0\E5ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int yy ;
	int xx ;

	
	// \CF\C2\C3濪ʼ\C0\FB\D3\C3Laplacian\CB\E3\D7ӽ\F8\D0м\C6\CB㣬Ϊ\C1˱\A3֤\BC\C6\CB\E3\CB\F9\D0\E8Ҫ\B5\C4
	// \B5\C4\CA\FD\BE\DDλ\D3\DAͼ\CF\F1\CA\FD\BEݵ\C4\C4ڲ\BF\A3\AC\CF\C2\C3\E6\B5\C4\C1\BD\D6\D8ѭ\BB\B7\B5\C4\CC\F5\BC\FE\CA\C7
	// y<nHeight-2 \B6\F8\B2\BB\CA\C7y<nHeight\A3\AC\CF\E0Ӧ\B5\C4x\B7\BD\CF\F2Ҳ\CA\C7x<nWidth-2
	// \B6\F8\B2\BB\CA\C7x<nWidth
	for(y=1; y<nHeight-2 ; y++ )
		for(x=1 ; x<nWidth-2 ; x++ )
		{
			dGrad = 0 ; 
			// Laplacian\CB\E3\D7\D3\D0\E8Ҫ\B5ĸ\F7\B5\E3\CF\F3\CB\D8ֵ

			// ģ\B0\E5\B5\DAһ\D0\D0
			nTmp[0][0] = lpImage[(y-1)*nSaveWidth + x - 1 ] ; 
			nTmp[0][1] = lpImage[(y-1)*nSaveWidth + x     ] ; 
			nTmp[0][2] = lpImage[(y-1)*nSaveWidth + x + 1 ] ; 

			// ģ\B0\E5\B5ڶ\FE\D0\D0
			nTmp[1][0] = lpImage[y*nSaveWidth + x - 1 ] ; 
			nTmp[1][1] = lpImage[y*nSaveWidth + x     ] ; 
			nTmp[1][2] = lpImage[y*nSaveWidth + x + 1 ] ; 

			// ģ\B0\E5\B5\DA\C8\FD\D0\D0
			nTmp[2][0] = lpImage[(y+1)*nSaveWidth + x - 1 ] ; 
			nTmp[2][1] = lpImage[(y+1)*nSaveWidth + x     ] ; 
			nTmp[2][2] = lpImage[(y+1)*nSaveWidth + x + 1 ] ; 
			
			// \BC\C6\CB\E3\CCݶ\C8
			for(yy=0; yy<3; yy++)
				for(xx=0; xx<3; xx++)
				{
					dGrad += nTmp[yy][xx] * nWeight[yy][xx] ;
				}
			
			// \CCݶ\C8ֵд\C8\EB\C4ڴ\E6
			*(pdGrad+y*nWidth+x)=dGrad;
		}

}*/

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   RegionGrow()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib					- ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   unsigned char * pUnRegion		- ָ\CF\F2\C7\F8\D3\F2\C9\FA\B3\A4\BD\E1\B9\FB\B5\C4ָ\D5\EB
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   pUnRegionָ\D5\EBָ\CF\F2\B5\C4\CA\FD\BE\DD\C7\F8\B4洢\C1\CB\C7\F8\D3\F2\C9\FA\B3\A4\B5Ľ\E1\B9\FB\A3\AC\C6\E4\D6\D01\A3\A8\C2߼\AD\A3\A9\B1\EDʾ
 *	 \B6\D4Ӧ\CF\F3\CB\D8Ϊ\C9\FA\B3\A4\C7\F8\D3\F2\A3\AC0\B1\EDʾΪ\B7\C7\C9\FA\B3\A4\C7\F8\D3\F2
 *   \C7\F8\D3\F2\C9\FA\B3\A4һ\B0\E3\B0\FC\BA\AC\C8\FD\B8\F6\B1Ƚ\CF\D6\D8Ҫ\B5\C4\CE\CA\CC⣺
 *		1. \D6\D6\D7ӵ\E3\B5\C4ѡȡ
 *		2. \C9\FA\B3\A4׼\D4\F2
 *		3. \D6\D5ֹ\CC\F5\BC\FE
 *	 \BF\C9\D2\D4\C8\CFΪ\A3\AC\D5\E2\C8\FD\B8\F6\CE\CA\CC\E2\D0\E8Ҫ\BE\DF\CC\E5\B7\D6\CE\F6\A3\AC\B6\F8\C7\D2ÿ\B8\F6\CE\CA\CC\E2\BD\E2\BE\F6\B5ĺû\B5ֱ\BDӹ\D8ϵ\B5\BD
 *	 \C7\F8\D3\F2\C9\FA\B3\A4\B5Ľ\E1\B9\FB\A1\A3
 *	 \B1\BE\BA\AF\CA\FD\B5\C4\D6\D6\D7ӵ\E3ѡȡΪͼ\CF\F1\B5\C4\D6\D0\D0ģ\AC\C9\FA\B3\A4׼\D4\F2\CA\C7\CF\E0\C1\DA\CF\F3\CBص\C4\CF\F3\CB\D8ֵС\D3\DA
 *	 nThreshold, \D6\D5ֹ\CC\F5\BC\FE\CA\C7һֱ\BD\F8\D0е\BD\D4\D9û\D3\D0\C2\FA\D7\E3\C9\FA\B3\A4׼\D4\F2\D0\E8Ҫ\B5\C4\CF\F3\CB\D8ʱΪֹ
 *
 *************************************************************************
 */
/*void RegionGrow(CDIB * pDib, unsigned char * pUnRegion, int nThreshold)
{
	static int nDx[]={-1,0,1,0};
	static int nDy[]={0,1,0,-1};

	// \B1\E9\C0\FAͼ\CF\F3\B5\C4\D7\DD\D7\F8\B1\EA
//	int y;

	// \B1\E9\C0\FAͼ\CF\F3\B5ĺ\E1\D7\F8\B1\EA
//	int x;

	// ͼ\CF\F3\B5ĳ\A4\BF\ED\B4\F3С
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	// ͼ\CF\F1\D4ڼ\C6\CB\E3\BB\FA\D4ڴ洢\D6е\C4ʵ\BCʴ\F3С
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	// ͼ\CF\F1\D4\DA\C4ڴ\E6\D6\D0ÿһ\D0\D0\CF\F3\CB\D8ռ\D3õ\C4ʵ\BCʿռ\E4
	int nSaveWidth = sizeImageSave.cx;

	// \B3\F5ʼ\BB\AF
	memset(pUnRegion,0,sizeof(unsigned char)*nWidth*nHeight);

	// \D6\D6\D7ӵ\E3
	int nSeedX, nSeedY;

	// \C9\E8\D6\C3\D6\D6\D7ӵ\E3Ϊͼ\CF\F1\B5\C4\D6\D0\D0\C4
	nSeedX = nWidth /2 ;
	nSeedY = nHeight/2 ;

	// \B6\A8\D2\E5\B6\D1ջ\A3\AC\B4洢\D7\F8\B1\EA
	int * pnGrowQueX ;
	int * pnGrowQueY ;
	
	// \B7\D6\C5\E4\BFռ\E4
	pnGrowQueX = new int [nWidth*nHeight];
	pnGrowQueY = new int [nWidth*nHeight];

	// ͼ\CF\F1\CA\FD\BEݵ\C4ָ\D5\EB
	unsigned char *  pUnchInput =(unsigned char * )pDib->m_lpImage;
	
	// \B6\A8\D2\E5\B6\D1ջ\B5\C4\C6\F0\B5\E3\BA\CD\D6յ\E3
	// \B5\B1nStart=nEnd, \B1\EDʾ\B6\D1ջ\D6\D0ֻ\D3\D0һ\B8\F6\B5\E3
	int nStart ;
	int nEnd   ;

	//\B3\F5ʼ\BB\AF
	nStart = 0 ;
	nEnd   = 0 ;

	// \B0\D1\D6\D6\D7ӵ\E3\B5\C4\D7\F8\B1\EAѹ\C8\EBջ
	pnGrowQueX[nEnd] = nSeedX;
	pnGrowQueY[nEnd] = nSeedY;

	// \B5\B1ǰ\D5\FD\D4ڴ\A6\C0\ED\B5\C4\CF\F3\CB\D8
	int nCurrX ;
	int nCurrY ;

	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int k ;

	// ͼ\CF\F3\B5ĺ\E1\D7\DD\D7\F8\B1\EA,\D3\C3\C0\B4\B6Ե\B1ǰ\CF\F3\CBص\C44\C1\DA\D3\F2\BD\F8\D0б\E9\C0\FA
	int xx;
	int yy;

	while (nStart<=nEnd)
	{
		// \B5\B1ǰ\D6\D6\D7ӵ\E3\B5\C4\D7\F8\B1\EA
		nCurrX = pnGrowQueX[nStart];
		nCurrY = pnGrowQueY[nStart];					

		// \B6Ե\B1ǰ\B5\E3\B5\C44\C1\DA\D3\F2\BD\F8\D0б\E9\C0\FA
		for (k=0; k<4; k++)	
		{	
			// 4\C1\DA\D3\F2\CF\F3\CBص\C4\D7\F8\B1\EA
			xx = nCurrX+nDx[k];
			yy = nCurrY+nDy[k];
			
			// \C5ж\CF\CF\F3\CB\D8(xx\A3\ACyy) \CAǷ\F1\D4\DAͼ\CF\F1\C4ڲ\BF
			// \C5ж\CF\CF\F3\CB\D8(xx\A3\ACyy) \CAǷ\F1\D2Ѿ\AD\B4\A6\C0\ED\B9\FD
			// pUnRegion[yy*nWidth+xx]==0 \B1\EDʾ\BB\B9û\D3д\A6\C0\ED

			// \C9\FA\B3\A4\CC\F5\BC\FE\A3\BA\C5ж\CF\CF\F3\CB\D8(xx\A3\ACyy)\BA͵\B1ǰ\CF\F3\CB\D8(nCurrX,nCurrY) \CF\F3\CB\D8ֵ\B2\EE\B5ľ\F8\B6\D4ֵ
			if (	(xx < nWidth) && (xx>=0) && (yy<nHeight) && (yy>=0) 
				    && (pUnRegion[yy*nWidth+xx]==0) 
					&& abs(pUnchInput[yy*nSaveWidth+xx] - pUnchInput[nCurrY*nSaveWidth+nCurrX])<nThreshold )
			{
				// \B6\D1ջ\B5\C4β\B2\BFָ\D5\EB\BA\F3\D2\C6һλ
				nEnd++;

				// \CF\F3\CB\D8(xx\A3\ACyy) ѹ\C8\EBջ
				pnGrowQueX[nEnd] = xx;
				pnGrowQueY[nEnd] = yy;

				// \B0\D1\CF\F3\CB\D8(xx\A3\ACyy)\C9\E8\D6ó\C9\C2߼\AD1\A3\A8255\A3\A9
				// ͬʱҲ\B1\ED\C3\F7\B8\C3\CF\F3\CBش\A6\C0\ED\B9\FD
				pUnRegion[yy*nWidth+xx] = 255 ;
			}
		}
		nStart++;
	}

	// \CAͷ\C5\C4ڴ\E6
	delete []pnGrowQueX;
	delete []pnGrowQueY;
    pnGrowQueX = NULL ;
	pnGrowQueY = NULL ;
}*/

//void DFT_2D(CDIB * pDib,double * pTrRstRpart, double * pTrRstIpart)
//{}


//void IDFT_2D(CDIB * pDib,double * pTrRstRpart, double * pTrRstIpart)
//{}

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   SobelOperator()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib		  - ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   double * pdGrad	- ָ\CF\F2\CCݶ\C8\CA\FD\BEݵ\C4ָ\D5룬\BA\AC\D3\D0ͼ\CF\F1\B5\C4\CCݶ\C8\D0\C5Ϣ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   Sobe\CB\E3\D7\D3
 *
 *   \B2\A2\D0б߽\E7\B7ָ\EE
 *
 *************************************************************************
 */
//void SobelOperator(CDIB * pDib, double * pdGrad)
//{}

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   PrewittOperator()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib		  - ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   double * pdGrad	- ָ\CF\F2\CCݶ\C8\CA\FD\BEݵ\C4ָ\D5룬\BA\AC\D3\D0ͼ\CF\F1\B5\C4\CCݶ\C8\D0\C5Ϣ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   Prewitt\CB\E3\D7\D3
 *
 *   \B2\A2\D0б߽\E7\B7ָ\EE
 *
 *************************************************************************
 */
//void PrewittOperator(CDIB * pDib, double * pdGrad)
//{}

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   EdgeTrack()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   CDIB * pDib					- ָ\CF\F2CDIB\C0\E0\B5\C4ָ\D5룬\BA\AC\D3\D0ԭʼͼ\CF\F3\D0\C5Ϣ
 *   unsigned char * pUnEdgeTrack	- ָ\CF\F2\B1߽\E7\B8\FA\D7ٽ\E1\B9\FB\B5\C4ָ\D5\EB
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   pUnEdgeTrackָ\D5\EBָ\CF\F2\B5\C4\CA\FD\BE\DD\C7\F8\B4洢\C1˱߽\E7\B8\FA\D7ٵĽ\E1\B9\FB\A3\AC\C6\E4\D6\D01\A3\A8\C2߼\AD\A3\A9\B1\EDʾ
 *	 \B6\D4Ӧ\CF\F3\CB\D8Ϊ\B1߽\E7\B5㣬0\B1\EDʾΪ\B7Ǳ߽\E7\B5\E3
 *
 *   \B4\AE\D0б߽\E7\B7ָ\EE
 *
 *************************************************************************
 */
//void EdgeTrack(CDIB * pDib, unsigned char * pUnEdgeTrack)
//{}


/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   MakeGauss()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   double sigma									        - \B8\DF˹\BA\AF\CA\FD\B5ı\EA׼\B2\EE
 *   double **pdKernel										- ָ\CF\F2\B8\DF˹\CA\FD\BE\DD\CA\FD\D7\E9\B5\C4ָ\D5\EB
 *   int *pnWindowSize										- \CA\FD\BEݵĳ\A4\B6\C8
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \D5\E2\B8\F6\BA\AF\CA\FD\BF\C9\D2\D4\C9\FA\B3\C9һ\B8\F6һά\B5ĸ\DF˹\BA\AF\CA\FD\B5\C4\CA\FD\D7\D6\CA\FD\BEݣ\AC\C0\ED\C2\DB\C9ϸ\DF˹\CA\FD\BEݵĳ\A4\B6\C8Ӧ
 *   \B8\C3\CA\C7\CE\DE\CF޳\A4\B5ģ\AC\B5\AB\CA\C7Ϊ\C1˼\C6\CB\E3\B5ļ򵥺\CD\CBٶȣ\ACʵ\BCʵĸ\DF˹\CA\FD\BE\DDֻ\C4\DC\CA\C7\D3\D0\CF޳\A4\B5\C4
 *   pnWindowSize\BE\CD\CA\C7\CA\FD\BEݳ\A4\B6\C8
 *   
 *************************************************************************
 */
void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize)
{
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int i   ;
	
	// \CA\FD\D7\E9\B5\C4\D6\D0\D0ĵ\E3
	int nCenter;

	// \CA\FD\D7\E9\B5\C4ĳһ\B5㵽\D6\D0\D0ĵ\E3\B5ľ\E0\C0\EB
	double dDis;


	// \D6м\E4\B1\E4\C1\BF
	double  dValue; 
	double  dSum;
	dSum = 0 ; 
	double pie;
	pie=3.14159;
	// \CA\FD\D7鳤\B6ȣ\AC\B8\F9\BEݸ\C5\C2\CA\C2۵\C4֪ʶ\A3\ACѡȡ[-3*sigma, 3*sigma]\D2\D4\C4ڵ\C4\CA\FD\BEݡ\A3
	// \D5\E2Щ\CA\FD\BEݻḲ\B8Ǿ\F8\B4󲿷ֵ\C4\C2˲\A8ϵ\CA\FD
	*pnWindowSize = 1 + 2 * (int)ceil(3 * sigma);
	
	// \D6\D0\D0\C4
	nCenter = (*pnWindowSize) / 2;
	
	// \B7\D6\C5\E4\C4ڴ\E6
	*pdKernel = new double[*pnWindowSize] ;
	
	for(i=0; i< (*pnWindowSize); i++)
	{
		dDis = (double)(i - nCenter);
		dValue = exp(-(1/2)*dDis*dDis/(sigma*sigma)) / (sqrt(2 * pie) * sigma );
		(*pdKernel)[i] = dValue ;
		dSum += dValue;
	}
	
	// \B9\E9һ\BB\AF
	for(i=0; i<(*pnWindowSize) ; i++)
	{
		(*pdKernel)[i] /= dSum;
	}
}

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   GaussianSmooth()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   unsigned char * pUnchImg				- ָ\CF\F2ͼ\CF\F3\CA\FD\BEݵ\C4ָ\D5\EB
 *   int nWidth											- ͼ\CF\F3\CA\FD\BEݿ\ED\B6\C8
 *   int nHeight										- ͼ\CF\F3\CA\FD\BEݸ߶\C8
 *   double dSigma									- \B8\DF˹\BA\AF\CA\FD\B5ı\EA׼\B2\EE
 *   unsigned char * pUnchSmthdImg	- ָ\CF򾭹\FDƽ\BB\AC֮\BA\F3\B5\C4ͼ\CF\F3\CA\FD\BE\DD
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   Ϊ\C1\CB\D2\D6ֹ\D4\EB\C9\F9\A3\AC\B2\C9\D3ø\DF˹\C2˲\A8\B6\D4ͼ\CF\F3\BD\F8\D0\D0\C2˲\A8\A3\AC\C2˲\A8\CFȶ\D4x\B7\BD\CF\F2\BD\F8\D0У\ACȻ\BA\F3\B6\D4
 *   y\B7\BD\CF\F2\BD\F8\D0С\A3
 *   
 *************************************************************************
 */
void GaussianSmooth(unsigned char *pUnchImg, int nWidth, int nHeight, 
										double sigma, unsigned char * pUnchSmthdImg)
{
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
  int y;
	int x;
	
	int i;
	
	// \B8\DF˹\C2˲\A8\C6\F7\B5\C4\CA\FD\D7鳤\B6\C8
	
	int nWindowSize;
	
	//  \B4\B0\BFڳ\A4\B6ȵ\C41/2
	int	nHalfLen;   
	
	// һά\B8\DF˹\CA\FD\BE\DD\C2˲\A8\C6\F7
	double *pdKernel ;
	
	// \B8\DF˹ϵ\CA\FD\D3\EBͼ\CF\F3\CA\FD\BEݵĵ\E3\B3\CB
	double  dDotMul     ;
	
	// \B8\DF˹\C2˲\A8ϵ\CA\FD\B5\C4\D7ܺ\CD
	double  dWeightSum     ;          
  
	// \D6м\E4\B1\E4\C1\BF
	double * pdTmp ;
	
	// \B7\D6\C5\E4\C4ڴ\E6
	pdTmp = new double[nWidth*nHeight];
	
	// \B2\FA\C9\FAһά\B8\DF˹\CA\FD\BE\DD\C2˲\A8\C6\F7
	// MakeGauss(sigma, &dKernel, &nWindowSize);
	MakeGauss(sigma, &pdKernel, &nWindowSize) ;
	
	// MakeGauss\B7\B5\BBش\B0\BFڵĳ\A4\B6ȣ\AC\C0\FB\D3ô˱\E4\C1\BF\BC\C6\CB㴰\BFڵİ볤
	nHalfLen = nWindowSize / 2;
	
  // x\B7\BD\CF\F2\BD\F8\D0\D0\C2˲\A8
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			dDotMul		= 0;
			dWeightSum = 0;
			for(i=(-nHalfLen); i<=nHalfLen; i++)
			{
				// \C5ж\CF\CAǷ\F1\D4\DAͼ\CF\F3\C4ڲ\BF
				if( (i+x) >= 0  && (i+x) < nWidth )
				{
					dDotMul += (double)pUnchImg[y*nWidth + (i+x)] * pdKernel[nHalfLen+i];
					dWeightSum += pdKernel[nHalfLen+i];
				}
			}
			pdTmp[y*nWidth + x] = dDotMul/dWeightSum ;
		}
	}
	
	// y\B7\BD\CF\F2\BD\F8\D0\D0\C2˲\A8
	for(x=0; x<nWidth; x++)
	{
		for(y=0; y<nHeight; y++)
		{
			dDotMul		= 0;
			dWeightSum = 0;
			for(i=(-nHalfLen); i<=nHalfLen; i++)
			{
				// \C5ж\CF\CAǷ\F1\D4\DAͼ\CF\F3\C4ڲ\BF
				if( (i+y) >= 0  && (i+y) < nHeight )
				{
					dDotMul += (double)pdTmp[(y+i)*nWidth + x] * pdKernel[nHalfLen+i];
					dWeightSum += pdKernel[nHalfLen+i];
				}
			}
			pUnchSmthdImg[y*nWidth + x] = (unsigned char)(int)dDotMul/dWeightSum ;
		}
	}

	// \CAͷ\C5\C4ڴ\E6
	delete []pdKernel;
	pdKernel = NULL ;
	
	delete []pdTmp;
	pdTmp = NULL;
}


/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   DirGrad()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   unsigned char *pUnchSmthdImg					- \BE\AD\B9\FD\B8\DF˹\C2˲\A8\BA\F3\B5\C4ͼ\CF\F3
 *   int nWidht														- ͼ\CF\F3\BF\ED\B6\C8
 *   int nHeight      										- ͼ\CF\F3\B8߶\C8
 *   int *pnGradX                         - x\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD
 *   int *pnGradY                         - y\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \D5\E2\B8\F6\BA\AF\CA\FD\BC\C6\CB㷽\CF\F2\B5\B9\CA\FD\A3\AC\B2\C9\D3õ\C4΢\B7\D6\CB\E3\D7\D3\CA\C7(-1 0 1) \BA\CD (-1 0 1)'(ת\D6\C3)
 *   \BC\C6\CB\E3\B5\C4ʱ\BA\F2\B6Ա߽\E7\CF\F3\CBز\C9\D3\C3\C1\CB\CC\D8\CA⴦\C0\ED
 *   
 *   
 *************************************************************************
 */
void DirGrad(unsigned char *pUnchSmthdImg, int nWidth, int nHeight,
										 int *pnGradX , int *pnGradY)
{
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int y ;
	int x ;
	
	// \BC\C6\CB\E3x\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD\A3\AC\D4ڱ߽\E7\B3\F6\BD\F8\D0\D0\C1˴\A6\C0\ED\A3\AC\B7\C0ֹҪ\B7\C3\CEʵ\C4\CF\F3\CBس\F6\BD\E7
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			pnGradX[y*nWidth+x] = (int) ( pUnchSmthdImg[y*nWidth+min(nWidth-1,x+1)]  
													          - pUnchSmthdImg[y*nWidth+max(0,x-1)]     );
		}
	}

	// \BC\C6\CB\E3y\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD\A3\AC\D4ڱ߽\E7\B3\F6\BD\F8\D0\D0\C1˴\A6\C0\ED\A3\AC\B7\C0ֹҪ\B7\C3\CEʵ\C4\CF\F3\CBس\F6\BD\E7
	for(x=0; x<nWidth; x++)
	{
		for(y=0; y<nHeight; y++)
		{
			pnGradY[y*nWidth+x] = (int) ( pUnchSmthdImg[min(nHeight-1,y+1)*nWidth + x]  
																		- pUnchSmthdImg[max(0,y-1)*nWidth+ x ]     );
		}
	}
}
/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   GradMagnitude()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   int *pnGradX                         - x\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD
 *   int *pnGradY                         - y\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD
 *   int nWidht														- ͼ\CF\F3\BF\ED\B6\C8
 *   int nHeight      										- ͼ\CF\F3\B8߶\C8
 *   int *pnMag                           - \CCݶȷ\F9\B6\C8   
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \D5\E2\B8\F6\BA\AF\CA\FD\C0\FB\D3÷\BD\CF\F2\B5\B9\CA\FD\BC\C6\CB\E3\CCݶȷ\F9\B6ȣ\AC\B7\BD\CF\F2\B5\B9\CA\FD\CA\C7DirGrad\BA\AF\CA\FD\BC\C6\CB\E3\B5Ľ\E1\B9\FB
 *   
 *************************************************************************
 */
void GradMagnitude(int *pnGradX, int *pnGradY, int nWidth, int nHeight, int *pnMag)
{
	
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int y ;
	int x ;

	// \D6м\E4\B1\E4\C1\BF
	double dSqtOne;
	double dSqtTwo;
	
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			dSqtOne = pnGradX[y*nWidth + x] * pnGradX[y*nWidth + x];
			dSqtTwo = pnGradY[y*nWidth + x] * pnGradY[y*nWidth + x];
			pnMag[y*nWidth + x] = (int)(sqrt(dSqtOne + dSqtTwo) + 0.5);
		}
	}
}
/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   NonmaxSuppress()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   int *pnMag                - \CCݶ\C8ͼ
 *   int *pnGradX							 - x\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD	
 *   int *pnGradY              - y\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD
 *   int nWidth                - ͼ\CF\F3\CA\FD\BEݿ\ED\B6\C8
 *   int nHeight               - ͼ\CF\F3\CA\FD\BEݸ߶\C8
 *   unsigned char *pUnchRst   - \BE\AD\B9\FDNonmaxSuppress\B4\A6\C0\ED\BA\F3\B5Ľ\E1\B9\FB
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \D2\D6ֹ\CCݶ\C8ͼ\D6зǾֲ\BF\BC\ABֵ\B5\E3\B5\C4\CF\F3\CBء\A3
 *   
 *************************************************************************
 */
void NonmaxSuppress(int *pnMag, int *pnGradX, int *pnGradY, int nWidth, 
										int nHeight,	unsigned char *pUnchRst)
{
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int y ;
	int x ;
	int nPos;

	// x\B7\BD\CF\F2\CCݶȷ\D6\C1\BF
	int gx  ;
	int gy  ;

	// \C1\D9ʱ\B1\E4\C1\BF
	int g1, g2, g3, g4 ;
	double weight  ;
	double dTmp1   ;
	double dTmp2   ;
	double dTmp    ;
	
	// \C9\E8\D6\C3ͼ\CF\F3\B1\DFԵ\B2\BF\B7\D6Ϊ\B2\BB\BF\C9\C4ܵı߽\E7\B5\E3
	for(x=0; x<nWidth; x++)		
	{
		pUnchRst[x] = 0 ;
		pUnchRst[nHeight-1+x] = 0;
	}
	for(y=0; y<nHeight; y++)		
	{
		pUnchRst[y*nWidth] = 0 ;
		pUnchRst[y*nWidth + nWidth-1] = 0;
	}

	for(y=1; y<nHeight-1; y++)
	{
		for(x=1; x<nWidth-1; x++)
		{
			nPos = y*nWidth + x ;
			
			// \C8\E7\B9\FB\B5\B1ǰ\CF\F3\CBص\C4\CCݶȷ\F9\B6\C8Ϊ0\A3\AC\D4\F2\B2\BB\CAǱ߽\E7\B5\E3
			if(pnMag[nPos] == 0 )
			{
				pUnchRst[nPos] = 0 ;
			}
			else
			{
				// \B5\B1ǰ\CF\F3\CBص\C4\CCݶȷ\F9\B6\C8
				dTmp = pnMag[nPos] ;
				
				// x\A3\ACy\B7\BD\CF\F2\B5\BC\CA\FD
				gx = pnGradX[nPos]  ;
				gy = pnGradY[nPos]  ;

				// \C8\E7\B9\FB\B7\BD\CF\F2\B5\BC\CA\FDy\B7\D6\C1\BF\B1\C8x\B7\D6\C1\BF\B4\F3\A3\AC˵\C3\F7\B5\BC\CA\FD\B5ķ\BD\CF\F2\B8\FC\BCӡ\B0\C7\F7\CF\F2\A1\B1\D3\DAy\B7\D6\C1\BF\A1\A3
				if (abs(gy) > abs(gx)) 
				{
					// \BC\C6\CB\E3\B2\E5ֵ\B5ı\C8\C0\FD
					weight = fabs((float)gx)/fabs((float)gy); 

					g2 = pnMag[nPos-nWidth] ; 
					g4 = pnMag[nPos+nWidth] ;
					
					// \C8\E7\B9\FBx\A3\ACy\C1\BD\B8\F6\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD\B5ķ\FB\BA\C5\CF\E0ͬ
					// C\CAǵ\B1ǰ\CF\F3\CBأ\AC\D3\EBg1-g4\B5\C4λ\D6ù\D8ϵΪ\A3\BA
					//	g1 g2 
					//		 C         
					//		 g4 g3 
					if (gx*gy > 0) 
					{ 					
						g1 = pnMag[nPos-nWidth-1] ;
						g3 = pnMag[nPos+nWidth+1] ;
					} 

					// \C8\E7\B9\FBx\A3\ACy\C1\BD\B8\F6\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD\B5ķ\FB\BA\C5\CF෴
					// C\CAǵ\B1ǰ\CF\F3\CBأ\AC\D3\EBg1-g4\B5\C4λ\D6ù\D8ϵΪ\A3\BA
					//	   g2 g1
					//		 C         
					//	g3 g4  
					else 
					{ 
						g1 = pnMag[nPos-nWidth+1] ;
						g3 = pnMag[nPos+nWidth-1] ;
					} 
				}
				
				// \C8\E7\B9\FB\B7\BD\CF\F2\B5\BC\CA\FDx\B7\D6\C1\BF\B1\C8y\B7\D6\C1\BF\B4\F3\A3\AC˵\C3\F7\B5\BC\CA\FD\B5ķ\BD\CF\F2\B8\FC\BCӡ\B0\C7\F7\CF\F2\A1\B1\D3\DAx\B7\D6\C1\BF
				// \D5\E2\B8\F6\C5ж\CF\D3\EF\BE\E4\B0\FC\BA\AC\C1\CBx\B7\D6\C1\BF\BA\CDy\B7\D6\C1\BF\CF\E0\B5ȵ\C4\C7\E9\BF\F6
				else
				{
					// \BC\C6\CB\E3\B2\E5ֵ\B5ı\C8\C0\FD
					weight = fabs((float)gy)/fabs((float)gx); 
					
					g2 = pnMag[nPos+1] ; 
					g4 = pnMag[nPos-1] ;
					
					// \C8\E7\B9\FBx\A3\ACy\C1\BD\B8\F6\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD\B5ķ\FB\BA\C5\CF\E0ͬ
					// C\CAǵ\B1ǰ\CF\F3\CBأ\AC\D3\EBg1-g4\B5\C4λ\D6ù\D8ϵΪ\A3\BA
					//	g3   
					//	g4 C g2       
					//       g1
					if (gx*gy > 0) 
					{				
						g1 = pnMag[nPos+nWidth+1] ;
						g3 = pnMag[nPos-nWidth-1] ;
					} 
					// \C8\E7\B9\FBx\A3\ACy\C1\BD\B8\F6\B7\BD\CF\F2\B5ķ\BD\CF\F2\B5\BC\CA\FD\B5ķ\FB\BA\C5\CF෴
					// C\CAǵ\B1ǰ\CF\F3\CBأ\AC\D3\EBg1-g4\B5\C4λ\D6ù\D8ϵΪ\A3\BA
					//	     g1
					//	g4 C g2       
					//  g3     
					else 
					{ 
						g1 = pnMag[nPos-nWidth+1] ;
						g3 = pnMag[nPos+nWidth-1] ;
					}
				}

				// \CF\C2\C3\E6\C0\FB\D3\C3g1-g4\B6\D4\CCݶȽ\F8\D0в\E5ֵ
				{
					dTmp1 = weight*g1 + (1-weight)*g2 ;
					dTmp2 = weight*g3 + (1-weight)*g4 ;
					
					// \B5\B1ǰ\CF\F3\CBص\C4\CCݶ\C8\CAǾֲ\BF\B5\C4\D7\EE\B4\F3ֵ
					// \B8õ\E3\BF\C9\C4\DC\CAǸ\F6\B1߽\E7\B5\E3
					if(dTmp>=dTmp1 && dTmp>=dTmp2)
					{
						pUnchRst[nPos] = 128 ;
					}
					else
					{
						// \B2\BB\BF\C9\C4\DC\CAǱ߽\E7\B5\E3
						pUnchRst[nPos] = 0 ;
					}
				}
			} //else
		} // for

	}
} 

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   TraceEdge()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   int    x									- \B8\FA\D7\D9\C6\F0\B5\E3\B5\C4x\D7\F8\B1\EA 
 *   int    y									- \B8\FA\D7\D9\C6\F0\B5\E3\B5\C4y\D7\F8\B1\EA
 *   int nLowThd							- \C5ж\CFһ\B8\F6\B5\E3\CAǷ\F1Ϊ\B1߽\E7\B5\E3\B5ĵ\CD\E3\D0ֵ
 *   unsigned char *pUnchEdge - \BC\C7¼\B1߽\E7\B5\E3\B5Ļ\BA\B3\E5\C7\F8
 *   int *pnMag               - \CCݶȷ\F9\B6\C8ͼ
 *   int nWidth               - ͼ\CF\F3\CA\FD\BEݿ\ED\B6\C8
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \B5ݹ\E9\B5\F7\D3\C3  
 *   \B4\D3(x,y)\D7\F8\B1\EA\B3\F6\B7\A2\A3\AC\BD\F8\D0б߽\E7\B5\E3\B5ĸ\FA\D7٣\AC\B8\FA\D7\D9ֻ\BF\BC\C2\C7pUnchEdge\D6\D0û\D3д\A6\C0\ED\B2\A2\C7\D2
 *   \BF\C9\C4\DC\CAǱ߽\E7\B5\E3\B5\C4\C4\C7Щ\CF\F3\CB\D8(=128),\CF\F3\CB\D8ֵΪ0\B1\ED\C3\F7\B8õ㲻\BF\C9\C4\DC\CAǱ߽\E7\B5㣬\CF\F3\CB\D8ֵ
 *   Ϊ255\B1\ED\C3\F7\B8õ\E3\D2Ѿ\AD\B1\BB\C9\E8\D6\C3Ϊ\B1߽\E7\B5㣬\B2\BB\B1\D8\D4ٿ\BC\C2\C7
 *   
 *   
 *************************************************************************
 */
void TraceEdge (int y, int x, int nLowThd, unsigned char *pUnchEdge, int *pnMag, int nWidth) 
{ 
	// \B6\D48\C1\DA\D3\F2\CF\F3\CBؽ\F8\D0в\E9ѯ
	int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1} ;
	int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1} ;

	int yy ;
	int xx ;
	
	int k  ;
	
	for(k=0; k<8; k++)
	{
		yy = y + yNb[k] ;
		xx = x + xNb[k] ;
		// \C8\E7\B9\FB\B8\C3\CF\F3\CB\D8Ϊ\BF\C9\C4ܵı߽\E7\B5㣬\D3\D6û\D3д\A6\C0\ED\B9\FD
		// \B2\A2\C7\D2\CCݶȴ\F3\D3\DA\E3\D0ֵ
		if(pUnchEdge[yy*nWidth+xx] == 128  && pnMag[yy*nWidth+xx]>=nLowThd)
		{
			// \B0Ѹõ\E3\C9\E8\D6ó\C9Ϊ\B1߽\E7\B5\E3
			pUnchEdge[yy*nWidth+xx] = 255 ;

			// \D2Ըõ\E3Ϊ\D6\D0\D0Ľ\F8\D0и\FA\D7\D9
			TraceEdge(yy, xx, nLowThd, pUnchEdge, pnMag, nWidth);
		}
	}
} 

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   EstimateThreshold()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   int *pnMag               - \CCݶȷ\F9\B6\C8ͼ
 *	 int nWidth               - ͼ\CF\F3\CA\FD\BEݿ\ED\B6\C8
 *	 int nHeight              - ͼ\CF\F3\CA\FD\BEݸ߶\C8
 *   int *pnThdHigh           - \B8\DF\E3\D0ֵ
 *   int *pnThdLow            - \B5\CD\E3\D0ֵ
 *	 double dRatioLow         - \B5\CD\E3\D0ֵ\BA͸\DF\E3\D0ֵ֮\BC\E4\B5ı\C8\C0\FD
 *	 double dRatioHigh        - \B8\DF\E3\D0ֵռͼ\CF\F3\CF\F3\CB\D8\D7\DC\CA\FD\B5ı\C8\C0\FD
 *   unsigned char *pUnchEdge - \BE\AD\B9\FDnon-maximum\B4\A6\C0\ED\BA\F3\B5\C4\CA\FD\BE\DD
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \BE\AD\B9\FDnon-maximum\B4\A6\C0\ED\BA\F3\B5\C4\CA\FD\BE\DDpUnchEdge\A3\ACͳ\BC\C6pnMag\B5\C4ֱ\B7\BDͼ\A3\ACȷ\B6\A8\E3\D0ֵ\A1\A3
 *   \B1\BE\BA\AF\CA\FD\D6\D0ֻ\CA\C7ͳ\BC\C6pUnchEdge\D6п\C9\C4\DCΪ\B1߽\E7\B5\E3\B5\C4\C4\C7Щ\CF\F3\CBء\A3Ȼ\BA\F3\C0\FB\D3\C3ֱ\B7\BDͼ\A3\AC
 *   \B8\F9\BE\DDdRatioHigh\C9\E8\D6ø\DF\E3\D0ֵ\A3\AC\B4洢\B5\BDpnThdHigh\A1\A3\C0\FB\D3\C3dRationLow\BA͸\DF\E3\D0ֵ\A3\AC
 *   \C9\E8\D6õ\CD\E3\D0ֵ\A3\AC\B4洢\B5\BD*pnThdLow\A1\A3dRatioHigh\CA\C7һ\D6ֱ\C8\C0\FD\A3\BA\B1\ED\C3\F7\CCݶ\C8С\D3\DA
 *   *pnThdHigh\B5\C4\CF\F3\CB\D8\CA\FDĿռ\CF\F3\CB\D8\D7\DC\CA\FDĿ\B5ı\C8\C0\FD\A1\A3dRationLow\B1\ED\C3\F7*pnThdHigh
 *   \BA\CD*pnThdLow\B5ı\C8\C0\FD\A3\AC\D5\E2\B8\F6\B1\C8\C0\FD\D4\DAcanny\CB㷨\B5\C4ԭ\CE\C4\C0\D7\F7\D5߸\F8\B3\F6\C1\CBһ\B8\F6\C7\F8\BC䡣
 *
 *************************************************************************
 */
void EstimateThreshold(int *pnMag, int nWidth, int nHeight, int *pnThdHigh,int *pnThdLow, 
											 unsigned char * pUnchEdge, double dRatioHigh, double dRationLow) 
{ 
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int y;
	int x;
	int k;
	
	// \B8\C3\CA\FD\D7\E9\B5Ĵ\F3С\BA\CD\CCݶ\C8ֵ\B5ķ\B6Χ\D3йأ\AC\C8\E7\B9\FB\B2\C9\D3ñ\BE\B3\CC\D0\F2\B5\C4\CB㷨\A3\AC\C4\C7ô\CCݶȵķ\B6Χ\B2\BB\BBᳬ\B9\FDpow(2,10)
	int nHist[1024] ;

	// \BF\C9\C4ܵı߽\E7\CA\FDĿ
	int nEdgeNb     ;

	// \D7\EE\B4\F3\CCݶ\C8ֵ
	int nMaxMag     ;

	int nHighCount  ;

	nMaxMag = 0     ; 
	
	// \B3\F5ʼ\BB\AF
	for(k=0; k<1024; k++) 
	{
		nHist[k] = 0; 
	}

	// ͳ\BC\C6ֱ\B7\BDͼ\A3\ACȻ\BA\F3\C0\FB\D3\C3ֱ\B7\BDͼ\BC\C6\CB\E3\E3\D0ֵ
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			// ֻ\CA\C7ͳ\BC\C6\C4\C7Щ\BF\C9\C4\DC\CAǱ߽\E7\B5㣬\B2\A2\C7һ\B9û\D3д\A6\C0\ED\B9\FD\B5\C4\CF\F3\CB\D8
			if(pUnchEdge[y*nWidth+x]==128)
			{
				nHist[ pnMag[y*nWidth+x] ]++;
			}
		}
	}

	nEdgeNb = nHist[0]  ;
	nMaxMag = 0         ;
	// ͳ\BCƾ\AD\B9\FD\A1\B0\B7\C7\D7\EE\B4\F3ֵ\D2\D6ֹ(non-maximum suppression)\A1\B1\BA\F3\D3ж\E0\C9\D9\CF\F3\CB\D8
	for(k=1; k<1024; k++)
	{
		if(nHist[k] != 0)
		{
			// \D7\EE\B4\F3\CCݶ\C8ֵ
			nMaxMag = k;
		}
		
		// \CCݶ\C8Ϊ0\B5ĵ\E3\CAǲ\BB\BF\C9\C4\DCΪ\B1߽\E7\B5\E3\B5\C4
		// \BE\AD\B9\FDnon-maximum suppression\BA\F3\D3ж\E0\C9\D9\CF\F3\CB\D8
		nEdgeNb += nHist[k];
	}

	// \CCݶȱȸ\DF\E3\D0ֵ*pnThdHighС\B5\C4\CF\F3\CBص\E3\D7\DC\CA\FDĿ
	nHighCount = (int)(dRatioHigh * nEdgeNb +0.5);
	
	k = 1;
	nEdgeNb = nHist[1];
	
	// \BC\C6\CB\E3\B8\DF\E3\D0ֵ
	while( (k<(nMaxMag-1)) && (nEdgeNb < nHighCount) )
	{
		k++;
		nEdgeNb += nHist[k];
	}

	// \C9\E8\D6ø\DF\E3\D0ֵ
	*pnThdHigh = k ;

	// \C9\E8\D6õ\CD\E3\D0ֵ
	*pnThdLow  = (int)((*pnThdHigh) * dRationLow+ 0.5);
}

/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   Hysteresis()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   int *pnMag               - \CCݶȷ\F9\B6\C8ͼ
 *	 int nWidth               - ͼ\CF\F3\CA\FD\BEݿ\ED\B6\C8
 *	 int nHeight              - ͼ\CF\F3\CA\FD\BEݸ߶\C8
 *	 double dRatioLow         - \B5\CD\E3\D0ֵ\BA͸\DF\E3\D0ֵ֮\BC\E4\B5ı\C8\C0\FD
 *	 double dRatioHigh        - \B8\DF\E3\D0ֵռͼ\CF\F3\CF\F3\CB\D8\D7\DC\CA\FD\B5ı\C8\C0\FD
 *   unsigned char *pUnchEdge - \BC\C7¼\B1߽\E7\B5\E3\B5Ļ\BA\B3\E5\C7\F8
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   \B1\BE\BA\AF\CA\FDʵ\CF\D6\C0\E0\CBơ\B0\B4\C5\D6\CD\CF\D6\CF󡱵\C4һ\B8\F6\B9\A6\C4ܣ\ACҲ\BE\CD\CAǣ\AC\CFȵ\F7\D3\C3EstimateThreshold
 *   \BA\AF\CA\FD\B6Ծ\AD\B9\FDnon-maximum\B4\A6\C0\ED\BA\F3\B5\C4\CA\FD\BE\DDpUnchSpr\B9\C0\BC\C6һ\B8\F6\B8\DF\E3\D0ֵ\A3\ACȻ\BA\F3\C5ж\CF
 *   pUnchSpr\D6п\C9\C4ܵı߽\E7\CF\F3\CB\D8(=128)\B5\C4\CCݶ\C8\CAǲ\BB\CAǴ\F3\D3ڸ\DF\E3\D0ֵnThdHigh\A3\AC\C8\E7\B9\FB\B1\C8
 *   \B8\C3\E3\D0ֵ\B4󣬸õ㽫\D7\F7Ϊһ\B8\F6\B1߽\E7\B5\C4\C6\F0\B5㣬\B5\F7\D3\C3TraceEdge\BA\AF\CA\FD\A3\AC\B0Ѷ\D4Ӧ\B8ñ߽\E7
 *   \B5\C4\CB\F9\D3\D0\CF\F3\CB\D8\D5ҳ\F6\C0\B4\A1\A3\D7\EE\BA󣬵\B1\D5\FB\B8\F6\CB\D1\CB\F7\CD\EA\B1\CFʱ\A3\AC\C8\E7\B9\FB\BB\B9\D3\D0\CF\F3\CB\D8û\D3б\BB\B1\EA־\B3\C9
 *   \B1߽\E7\B5㣬\C4\C7ô\BE\CDһ\B6\A8\B2\BB\CAǱ߽\E7\B5㡣
 *   
 *************************************************************************
 */
void Hysteresis(int *pnMag, int nWidth, int nHeight, double dRatioLow, 
								double dRatioHigh, unsigned char *pUnchEdge)
{
	// ѭ\BB\B7\BF\D8\D6Ʊ\E4\C1\BF
	int y;
	int x;

	int nThdHigh ;
	int nThdLow  ;

	int nPos;

	// \B9\C0\BC\C6TraceEdge\D0\E8Ҫ\B5ĵ\CD\E3\D0ֵ\A3\AC\D2Լ\B0Hysteresis\BA\AF\CA\FDʹ\D3õĸ\DF\E3\D0ֵ
	EstimateThreshold(pnMag, nWidth, nHeight, &nThdHigh, 
		               &nThdLow, pUnchEdge,dRatioHigh, dRatioLow);

  // \D5\E2\B8\F6ѭ\BB\B7\D3\C3\C0\B4Ѱ\D5Ҵ\F3\D3\DAnThdHigh\B5ĵ㣬\D5\E2Щ\B5㱻\D3\C3\C0\B4\B5\B1\D7\F7\B1߽\E7\B5㣬Ȼ\BA\F3\D3\C3
	// TraceEdge\BA\AF\CA\FD\C0\B4\B8\FA\D7ٸõ\E3\B6\D4Ӧ\B5ı߽\E7
   for(y=0; y<nHeight; y++)
	 {
      for(x=0; x<nWidth; x++)
			{
				nPos = y*nWidth + x ; 

				// \C8\E7\B9\FB\B8\C3\CF\F3\CB\D8\CAǿ\C9\C4ܵı߽\E7\B5㣬\B2\A2\C7\D2\CCݶȴ\F3\D3ڸ\DF\E3\D0ֵ\A3\AC\B8\C3\CF\F3\CB\D8\D7\F7Ϊ
				// һ\B8\F6\B1߽\E7\B5\C4\C6\F0\B5\E3
				if((pUnchEdge[nPos] == 128) && (pnMag[nPos] >= nThdHigh))
				{
					// \C9\E8\D6øõ\E3Ϊ\B1߽\E7\B5\E3
					pUnchEdge[nPos] = 255;
					TraceEdge(y, x, nThdLow, pUnchEdge, pnMag, nWidth);
				}
      }
   }

	 // \C4\C7Щ\BB\B9û\D3б\BB\C9\E8\D6\C3Ϊ\B1߽\E7\B5\E3\B5\C4\CF\F3\CB\D8\D2Ѿ\AD\B2\BB\BF\C9\C4ܳ\C9Ϊ\B1߽\E7\B5\E3
   for(y=0; y<nHeight; y++)
	 {
		 for(x=0; x<nWidth; x++)
		 {
			 nPos = y*nWidth + x ; 
			 if(pUnchEdge[nPos] != 255)
			 {
				 // \C9\E8\D6\C3Ϊ\B7Ǳ߽\E7\B5\E3
				 pUnchEdge[nPos] = 0 ;
			 }
		 }
	 }
}


/*************************************************************************
 *
 * \\BA\AF\CA\FD\C3\FB\B3ƣ\BA
 *   Canny()
 *
 * \\CA\E4\C8\EB\B2\CE\CA\FD:
 *   unsigned char *pUnchImage- ͼ\CF\F3\CA\FD\BE\DD
 *	 int nWidth               - ͼ\CF\F3\CA\FD\BEݿ\ED\B6\C8
 *	 int nHeight              - ͼ\CF\F3\CA\FD\BEݸ߶\C8
 *   double sigma             - \B8\DF˹\C2˲\A8\B5ı\EA׼\B7\BD\B2\EE
 *	 double dRatioLow         - \B5\CD\E3\D0ֵ\BA͸\DF\E3\D0ֵ֮\BC\E4\B5ı\C8\C0\FD
 *	 double dRatioHigh        - \B8\DF\E3\D0ֵռͼ\CF\F3\CF\F3\CB\D8\D7\DC\CA\FD\B5ı\C8\C0\FD
 *   unsigned char *pUnchEdge - canny\CB\E3\D7Ӽ\C6\CB\E3\BA\F3\B5ķָ\EEͼ
 *
 * \\B7\B5\BB\D8ֵ:
 *   \CE\DE
 *
 * \˵\C3\F7:
 *   canny\B7ָ\EE\CB\E3\D7ӣ\AC\BC\C6\CB\E3\B5Ľ\E1\B9\FB\B1\A3\B4\E6\D4\DApUnchEdge\D6У\AC\C2߼\AD1(255)\B1\EDʾ\B8õ\E3Ϊ
 *   \B1߽\E7\B5㣬\C2߼\AD0(0)\B1\EDʾ\B8õ\E3Ϊ\B7Ǳ߽\E7\B5㡣\B8ú\AF\CA\FD\B5Ĳ\CE\CA\FDsigma\A3\ACdRatioLow
 *   dRatioHigh\A3\AC\CA\C7\D0\E8Ҫָ\B6\A8\B5ġ\A3\D5\E2Щ\B2\CE\CA\FD\BB\E1Ӱ\CF\EC\B7ָ\EE\BA\F3\B1߽\E7\B5\E3\CA\FDĿ\B5Ķ\E0\C9\D9
 *************************************************************************
 */
void Canny(unsigned char *pUnchImage, int nWidth, int nHeight, double sigma,
					 double dRatioLow, double dRatioHigh, unsigned char *pUnchEdge)
{
	// \BE\AD\B9\FD\B8\DF˹\C2˲\A8\BA\F3\B5\C4ͼ\CF\F3\CA\FD\BE\DD
	unsigned char * pUnchSmooth ;
  
	// ָ\CF\F2x\B7\BD\CF\F2\B5\BC\CA\FD\B5\C4ָ\D5\EB
	int * pnGradX ; 

	// ָ\CF\F2y\B7\BD\CF\F2\B5\BC\CA\FD\B5\C4ָ\D5\EB
	int * pnGradY ;

	// \CCݶȵķ\F9\B6\C8
	int * pnGradMag ;

	pUnchSmooth  = new unsigned char[nWidth*nHeight] ;
	pnGradX      = new int [nWidth*nHeight]          ;
	pnGradY      = new int [nWidth*nHeight]          ;
	pnGradMag    = new int [nWidth*nHeight]          ;

	// \B6\D4ԭͼ\CF\F3\BD\F8\D0\D0\C2˲\A8
	GaussianSmooth(pUnchImage, nWidth, nHeight, sigma, pUnchSmooth) ;

	// \BC\C6\CB㷽\CF\F2\B5\BC\CA\FD
	DirGrad(pUnchSmooth, nWidth, nHeight, pnGradX, pnGradY) ;

	// \BC\C6\CB\E3\CCݶȵķ\F9\B6\C8
	GradMagnitude(pnGradX, pnGradY, nWidth, nHeight, pnGradMag) ;

	// Ӧ\D3\C3non-maximum \D2\D6\D6\C6
	NonmaxSuppress(pnGradMag, pnGradX, pnGradY, nWidth, nHeight, pUnchEdge) ;

	// Ӧ\D3\C3Hysteresis\A3\AC\D5ҵ\BD\CB\F9\D3еı߽\E7
	Hysteresis(pnGradMag, nWidth, nHeight, dRatioLow, dRatioHigh, pUnchEdge);


	// \CAͷ\C5\C4ڴ\E6
	delete []pnGradX      ;
	pnGradX      = NULL ;
	delete []pnGradY      ;
	pnGradY      = NULL ;
	delete []pnGradMag    ;
	pnGradMag    = NULL ;
	delete []pUnchSmooth ;
	pUnchSmooth  = NULL ;
}
void Gradient(unsigned char*pImage, int nWidth, int nHeight,float* pEdge)
{
	int X,Y,I,J;
	float sumX,sumY,SUM;
	int			GX[3][3];
	int			GY[3][3];
	
	/* 3x3 GX Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
	GX[0][0] = -1; GX[0][1] = 0; GX[0][2] = 1;
	GX[1][0] = -2; GX[1][1] = 0; GX[1][2] = 2;
	GX[2][0] = -1; GX[2][1] = 0; GX[2][2] = 1;
	
	/* 3x3 GY Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
	GY[0][0] =  1; GY[0][1] =  2; GY[0][2] =  1;
	GY[1][0] =  0; GY[1][1] =  0; GY[1][2] =  0;
	GY[2][0] = -1; GY[2][1] = -2; GY[2][2] = -1;
	
	
	for(Y=0; Y<=(nHeight-1); Y++)  {
		for(X=0; X<=(nWidth-1); X++)  {
			sumX = 0;
			sumY = 0;
			
			/* image boundaries */
			if(Y==0 || Y==nHeight-1)
				SUM = 0;
			else if(X==0 || X==nWidth-1)
				SUM = 0;
			
			/* Convolution starts here */
			else   {
				
				/*-------X GRADIENT APPROXIMATION------*/
				for(I=-1; I<=1; I++)  {
					for(J=-1; J<=1; J++)  {
						sumX = sumX + (float)( (*(pImage+ X + I +(Y + J)*nWidth)) *	
							GX[I+1][J+1]);
					}
				}
				
				/*-------Y GRADIENT APPROXIMATION-------*/
				for(I=-1; I<=1; I++)  {
					for(J=-1; J<=1; J++)  {
						sumY = sumY + (float)( (*(pImage+ X + I + 
							(Y + J)*nWidth)) * GY[I+1][J+1]);
					}
				}
				
				/*---GRADIENT MAGNITUDE APPROXIMATION (Myler p.218)----*/
				SUM = abs(sumX) + abs(sumY);
			}
			*(pEdge+ X + Y*nWidth) = SUM;
		}
	}	
}

