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
	// 调用SOBEL FILTER compute gradient absolute value
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
	// 调用canny函数进行边界提取
	Canny(input,w,h, sig,low,high, EM);
	
	delete[] input;
}

MyLUV*  GetNearPixelsLUV(int xPos, int yPos
	  , MyLUV* inLUVs, int picWidth, int picHeight
	  , int inScale, int& outWidth, int& outHeight)
//得到LUV邻域；
{
	//首先计算可能的邻域点数；
	int templeft = xPos - inScale;
	int tempright = xPos + inScale;
	int left, right, up, down;//邻域的左右上下边界；
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
			//以下求输出的第x,y个元素在原数组中的X和Y位置；
			int originx = xPos - left + x;
			int originy = yPos - up + y;
			//在输入的图像数据中的位置；
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
//得到邻域像素值(正方形,G通道),输入位置从0开始计数, 边缘处对称延拓；
{
	int matrixwidth = (radius*2+1);
	BYTE* temparr = new BYTE[matrixwidth*matrixwidth];//包括指定点自身；

	LONG pos = 0;
	int rposx, rposy;
	rposx = rposy = 0;//在图像中的位置；

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

			//在输入的图像数据中的位置；
			LONG inpos = ( rposy*picWidth + rposx ) * 3 + 1;//RGB三色值；
			//在输出数组中的位置；
			LONG pos = ( (y+radius)*matrixwidth + (x+radius) );
			temparr[pos] = ( inPixels[inpos] );
		}
	}

	*outArr = temparr;
}


void  GetNearPixelsExt(int xPos, int yPos
	, BYTE* inPixels, int picWidth, int picHeight
	, int radius, BYTE** outArr)
//得到邻域像素值(正方形),输入位置从0开始计数, 边缘处延拓；
{
	int matrixwidth = (radius*2+1);
	BYTE* temparr = new BYTE[matrixwidth*matrixwidth*3];//包括指定点自身；

	LONG pos = 0;
	int rposx, rposy;
	rposx = rposy = 0;//在图像中的位置；

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

			//在输入的图像数据中的位置；
			LONG inpos = ( rposy*picWidth + rposx ) * 3;//RGB三色值；
			//在输出数组中的位置；
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
//得到邻域像素值, 输入位置从0开始计数；
{
	//首先计算可能的邻域点数；
	int templeft = xPos - inScale;
	int tempright = xPos + inScale;
	int left, right, up, down;//邻域的左右上下边界；
	if (templeft>0 && tempright<picWidth)
	{
		//outWidth = inScale * 2 + 1;//加1则包含了像素本身；
		left = inScale;
		right = inScale;
	}else
	{
		if (templeft<=0)
		{
			//outWidth += xPos;//以左全在邻域内；
			left = xPos;
		}else
		{
			left = inScale;
		}
		if (tempright>=picWidth)
		{
			//outWidth += picWidth - xPos - 1;//以右全在邻域内；
			right = picWidth - xPos - 1;
		}else
		{
			right = inScale;
		}

/*
		if (outWidth>picWidth)
		{
			outWidth = picWidth;//邻域宽度与图像宽相等；
		}
*/
	}

	int temptop = yPos - inScale;
	int tempbottom = yPos + inScale;
	if ( temptop>0 && tempbottom<picHeight )
	{
		//outWidth = inScale * 2 + 1;//加1则包含了像素本身；
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
			outWidth = picWidth;//邻域宽度与图像宽相等；
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
			pos = ( y*outWidth + x ) * 3;//RGB三色值；
			//以下求输出的第x,y个元素在原数组中的X和Y位置；
			int originx = xPos - left + x;
			int originy = yPos - up + y;
			//在输入的图像数据中的位置；
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
//得到输入图像的梯度；
{
	//下面计算各像素在水平和垂直方向上的梯度,边缘点梯度计为0；
	int* deltaxarr;
	int* deltayarr;
	int grawidth = width;
	int graheight = height;
	int deltacount = grawidth * graheight;
	deltaxarr = new int[deltacount];
	deltayarr = new int[deltacount];

    //暂不计算边缘点；
	for (int y=1; y<graheight-1; y++)
	{
		for (int x=1; x<grawidth-1; x++)
		{
			int inarrpos = ((y)*width + (x))*3 + 1;//在输入块中的位置；
			int deltaarrpos = y*grawidth + x;//在梯度数组中的位置；
			//卷积计算；
			deltaxarr[deltaarrpos] = (int) ( (
				image[((y-1)*width + (x+1))*3 + 1] //右上
				+ image[((y)*width + (x+1))*3 + 1] //右
				+ image[((y+1)*width + (x+1))*3 + 1] //右下
				- image[((y-1)*width + (x-1))*3 + 1] //左上
				- image[((y)*width + (x-1))*3 + 1] //左
				- image[((y+1)*width + (x-1))*3 + 1] ) / 3 );//左下
			deltayarr[deltaarrpos] = (int) ( ( 
				image[((y-1)*width + (x+1))*3 + 1] //右上
				+ image[((y-1)*width + (x))*3 + 1] //上
				+ image[((y-1)*width + (x-1))*3 + 1] //左上
				- image[((y+1)*width + (x-1))*3 + 1] //左下
				- image[((y+1)*width + (x))*3 + 1] //下
				- image[((y+1)*width + (x+1))*3 + 1]) / 3 );//右下
		}
	}

	//边缘赋为其内侧点的值；
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
		int inner = x + grawidth;//下一行；
		deltaxarr[pos1] = deltaxarr[inner];
		deltayarr[pos1] = deltayarr[inner];
		int y2 = graheight-1;
		int pos2 = y2*grawidth + x;
		inner = pos2 - grawidth;//上一行；
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
					deltasita[temppos] = 0;//水平方向;
					deltar[temppos] = (FLOAT) abs(deltayarr[temppos]);
				}else
				{
					deltasita[temppos] = -1;//无确定方向;
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

	delete [] deltaxarr; deltaxarr = NULL; //删除水平和垂直梯度数组；
	delete [] deltayarr; deltayarr = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Luc Vincent and Pierre Soille的分水岭分割flood步骤的实现代码， 
// 修改自相应伪代码, 伪代码来自作者论文《Watersheds in Digital Spaces:
// An Efficient Algorithm Based on Immersion Simulations》
// IEEE TRANSACTIONS ON PATTERN ANALYSIS AND MACHINE INTELLIGENCE.
// VOL.13, NO.6, JUNE 1991;
// by dzj, 2004.06.28 
// MyImageGraPt* imiarr - 输入的排序后数组
// int* graddarr -------- 输入的各梯度数组，由此直接存取各H像素点
// int minh，int maxh == 最小最大梯度
// int* flagarr --------- 输出标记数组
// 注意：目前不设分水岭标记，只设每个像素所属区域；
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
	int* imd = new int[imagelen];//距离数组，直接存取；
	for (i=0; i<imagelen; i++)
	{
		imd[i] = 0;
	}
	//memset(imd, 0, sizeof(int)*imagelen);
	std::queue <int> myqueue;
	int curlabel = 0;//各盆地标记；
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
			//以下检查该点邻域是否已标记属于某区或分水岭，若是，则将该点加入fifo;
			int left = ipos - 1;
			if (x-1>=0) 
			{
				if (flagarr[left]>=0)
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//点位置压入fifo;
					continue;
				}				
			}
			int right = ipos + 1;
			if (x+1<imageWidth) 
			{
				if (flagarr[right]>=0) 
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//点位置压入fifo;
					continue;
				}
			}
			int up = ipos - imageWidth;
			if (y-1>=0) 
			{
				if (flagarr[up]>=0)
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//点位置压入fifo;
					continue;
				}				
			}
			int down = ipos + imageWidth;
			if (y+1<imageHeight)
			{
				if (flagarr[down]>=0) 
				{
					imd[ipos] = 1;
					myqueue.push(ipos);//点位置压入fifo;
					continue;
				}			
			}
		}

    	//以下根据先进先出队列扩展现有盆地；
		int curdist = 1; myqueue.push(-99);//特殊标记；
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

			//以下找p的邻域；
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
						//ppei属于某区域（不是分水岭）；
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//将其设为邻点所属区域；
							flagarr[p] = flagarr[left];
						}else if (flagarr[p]!=flagarr[left])
						{
							//原来赋的区与现在赋的区不同，设为分水岭；
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppei为分岭；
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[left]==MASK) && (imd[left]==0) )
				//ppei中已MASK的点，但尚未标记（即不属某区也不是分水岭）;
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
						//ppei属于某区域（不是分水岭）；
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//将其设为邻点所属区域；
							flagarr[p] = flagarr[right];
						}else if (flagarr[p]!=flagarr[right])
						{
							//原来赋的区与现在赋的区不同，设为分水岭；
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppei为分岭；
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[right]==MASK) && (imd[right]==0) )
					//ppei中已MASK的点，但尚未标记（即不属某区也不是分水岭）;
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
						//ppei属于某区域（不是分水岭）；
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//将其设为邻点所属区域；
							flagarr[p] = flagarr[up];
						}else if (flagarr[p]!=flagarr[up])
						{
							//原来赋的区与现在赋的区不同，设为分水岭；
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppei为分岭；
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[up]==MASK) && (imd[up]==0) )
					//ppei中已MASK的点，但尚未标记（即不属某区也不是分水岭）;
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
						//ppei属于某区域（不是分水岭）；
						if ( (flagarr[p]==MASK) 
							|| (flagarr[p]==WATERSHED) )
						{
							//将其设为邻点所属区域；
							flagarr[p] = flagarr[down];
						}else if (flagarr[p]!=flagarr[down])
						{
							//原来赋的区与现在赋的区不同，设为分水岭；
							//flagarr[p] = WATERSHED;
						}
					}else if (flagarr[p]==MASK)//ppei为分岭；
					{
						flagarr[p] = WATERSHED;
					}
				}else if ( (flagarr[down]==MASK) && (imd[down]==0) )
					//ppei中已MASK的点，但尚未标记（既不属某区也不是分水岭）;
				{
					imd[down] = curdist + 1; myqueue.push(down);
				}	
			}

		}//以上现有盆地的扩展；

		//以下处理新发现的盆地；
		for ( ini=stpos; ini<edpos; ini++)
		{
			int x = imiarr[ini].x;
			int y = imiarr[ini].y;
			int ipos = y*imageWidth + x;
			imd[ipos] = 0;//重置所有距离
			if (flagarr[ipos]==MASK)
			{
				//经过前述扩展后该点仍为MASK，则该点必为新盆地的一个起始点;
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
						myqueue.push(ppeileft);//点位置压入fifo;
						flagarr[ppeileft] = curlabel;
					}
					int ppeiright = ppei + 1;
					if ( (ppeix+1<imageWidth) && (flagarr[ppeiright]==MASK) )
					{
						myqueue.push(ppeiright);//点位置压入fifo;
						flagarr[ppeiright] = curlabel;
					}
					int ppeiup = ppei - imageWidth;
					if ( (ppeiy-1>=0) && (flagarr[ppeiup]==MASK) )
					{
						myqueue.push(ppeiup);//点位置压入fifo;
						flagarr[ppeiup] = curlabel;
					}
					int ppeidown = ppei + imageWidth;
					if ( (ppeiy+1<imageHeight) && (flagarr[ppeidown]==MASK) )
					{
						myqueue.push(ppeidown);//点位置压入fifo;
						flagarr[ppeidown] = curlabel;
					}					
				}				
			}
		}//以上处理新发现的盆地；

	}

	outrgnumber = curlabel;	
	delete [] imd; imd = NULL;
}

#define NearMeasureBias 200.0//判定区域颜色相似的阈值；
void  MergeRgs(MyRgnInfo* rginfoarr, int rgnumber, int* flag, int width, int height, int* outmerge, int& rgnum)
//合并相似区域；
{
	//////////////////////////////////////////////////////////////////////////
	//1、建立各区的邻域数组；
	//2、依次扫描各区域，寻找极小区域；
	//3、对每个极小区（A），在相邻区中找到最相似者；
	//4、与相似区（B）合并（各种信息刷新），在极小区（A）的邻域中
	//   删除相似区（B），在邻域数组中删除相似区（B）对应的项，将
	//   相似区（B）的相邻区s加到极小区（A）的邻域中去；
	//5、记录合并信息，设一数组专门存放该信息，该数组的第A个元素值设为B；
	//6、判断是否仍为极小区，若是则返回3；
	//7、是否所有区域都已处理完毕，若非则返回2；
	//
	//   由于各区的相邻区不会太多，因此采用邻接数组作为存储结构；
	//////////////////////////////////////////////////////////////////////////
	CString* neiarr = new CString[rgnumber+1];//第一个不用；
	int* mergearr = outmerge;//记录合并情况数组；
	MyMath myMath;
	//建立邻域数组；
	for (int y=0; y<height; y++)
	{
		int lstart = y * width;
		for (int x=0; x<width; x++)
		{
			int pos = lstart + x;
			int left=-1, right=-1, up=-1, down=-1;
			myMath.GetNeiInt(x, y, pos, width, height
		, left, right, up, down);//找pos的四个邻域；
			//确定并刷新邻域区信息；
			int curid = flag[pos];
			AddNeiOfCur(curid, left, right
				, up, down, flag, neiarr);
		}
	}//建立邻域数组；
	
	//区域信息数组中的有效信息从1开始，第i个位置存放第i个区域的信息；
	for (int rgi=1; rgi<=rgnumber; rgi++)
	{
		//扫描所有区域，找极小区；
		LONG allpoints = width * height;
		LONG nmin = (LONG) (allpoints / 400);
		int curid = rgi;

		//rginfoarr[rgi].isflag初始为FALSE，在被合并到其它区后改为TRUE；
		while ( ( (rginfoarr[rgi].ptcount)<nmin ) 
			&& !rginfoarr[rgi].isflag )
		{
			//该区为极小区，遍历所有相邻区，找最接近者；
			CString neistr = neiarr[curid];
			int nearid = FindNearestNei(curid, neistr, rginfoarr, mergearr);
			//合并curid与nearid；
			MergeTwoRgn(curid, nearid, neiarr
				, rginfoarr, mergearr);			
		} 
	}

	//以下再合并相似区域，（无论大小）,如果不需要，直接将整个循环注释掉就行了；
	int countjjj = 0;
	//区域信息数组中的有效信息从1开始，第i个位置存放第i个区域的信息；
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
//找到idint最终所合并到的区号；
{
	int outid = idint;
	while ( mergearr[outid] > 0 )
	{
		outid = mergearr[outid];
	}
	return outid;
}
void  MergeNearest(int curid, MyRgnInfo* rginfoarr, CString* neiarr, int* mergearr)
//合并相似区域；
{
	//依次处理各个邻域，若相似，则合并；
	//CString neistr = neiarr[curid];
	FLOAT cl, cu, cv;
	cl = rginfoarr[curid].l;//当前区的LUV值；
	cu = rginfoarr[curid].u;
	cv = rginfoarr[curid].v;
	BOOL loopmerged = TRUE;//一次循环中是否有合并操作发生，若无，则退出循环；

	while (loopmerged)
	{
		loopmerged = FALSE;
		CString tempstr = neiarr[curid];//用于本函数内部处理；
		while (tempstr.GetLength()>0)
		{
			int pos = tempstr.Find(" ");
			ASSERT(pos>=0);
			CString idstr = tempstr.Left(pos);
			tempstr.Delete(0, pos+1);
			
			int idint = (int) strtol(idstr, NULL, 10);
			//判断该区是否已被合并，若是，则一直找到该区当前的区号；
			idint = FindMergedRgn(idint, mergearr);
			if (idint==curid)
			{
				continue;//这个邻区已被合并到当前区，跳过；
			}
			FLOAT tl, tu, tv;
			tl = rginfoarr[idint].l;//当前处理的邻区的LUV值;
			tu = rginfoarr[idint].u;
			tv = rginfoarr[idint].v;
			DOUBLE tempdis = pow(tl-cl, 2) 
				+ pow(tu-cu, 2) + pow(tv-cv, 2);
			if (tempdis<NearMeasureBias)
			{
				MergeTwoRgn(curid, idint, neiarr, rginfoarr, mergearr);
				cl = rginfoarr[curid].l;//当前区的LUV值刷新；
				cu = rginfoarr[curid].u;
				cv = rginfoarr[curid].v;
				loopmerged = TRUE;
			}		
		}
	}
}

void  MergeTwoRgn(int curid, int nearid
	, CString* neiarr, MyRgnInfo* rginfoarr, int* mergearr)
//将nearid合并到curid中去，更新合并后的区信息，并记录该合并；
{
	//将区信息中nearid对应项的标记设为已被合并；
	rginfoarr[nearid].isflag = TRUE;
	//更新合并后的LUV信息；
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
	//将nearid的邻域加到curid的邻域中去；
	AddBNeiToANei(curid, nearid, neiarr, mergearr);
	//记录该合并；
	mergearr[nearid] = curid;
}

void  AddBNeiToANei(int curid, int nearid, CString* neiarr, int* mergearr)
//将nearid的邻域加到curid的邻域中去；
{
	//先从curid的邻区中把nearid删去；
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
		//否则邻近区为合并过来的区，忽略；
		neiarr[curid].Delete(temppos, tempstr.GetLength());
	}
*/
    //将nearid的邻区依次加到curid的邻区中去；
	CString neistr = neiarr[nearid];
	CString curstr = neiarr[curid];
	//一般说来，极小区的邻域应该较少，因此，为着提高合并速度，将
	//curstr加到neistr中去，然后将结果赋给neiarr[curid];
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
			continue;//本区不与本区相邻；
		}else
		{
			if ( neistr.Find(idstr, 0) >= 0 )
			{
				continue;
			}else
			{
				neistr += idstr;//加到邻区中去;
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
			continue;//本区不与本区相邻；
		}else
		{
			if ( neiarr[curid].Find(idstr, 0) >= 0 )
			{
				continue;
			}else
			{
				neiarr[curid] += idstr;//加到邻区中去;
			}
		}		
	}
*/
}


int  FindNearestNei(int curid, CString neistr, MyRgnInfo* rginfoarr, int* mergearr)
//寻找neistr中与curid最接近的区，返回该区id号；
{
	int outid = -1;
	DOUBLE mindis = 999999;
	FLOAT cl, cu, cv;
	cl = rginfoarr[curid].l;//当前区的LUV值；
	cu = rginfoarr[curid].u;
	cv = rginfoarr[curid].v;

	CString tempstr = neistr;//用于本函数内部处理；
	while (tempstr.GetLength()>0)
	{
		int pos = tempstr.Find(" ");
		ASSERT(pos>=0);
		CString idstr = tempstr.Left(pos);
		tempstr.Delete(0, pos+1);

		int idint = (int) strtol(idstr, NULL, 10);
		//判断该区是否已被合并，若是，则一直找到该区当前的区号；
		idint = FindMergedRgn(idint, mergearr);
		if (idint==curid)
		{
			continue;//这个邻区已被合并到当前区，跳过；
		}
		FLOAT tl, tu, tv;
		tl = rginfoarr[idint].l;//当前处理的邻区的LUV值;
		tu = rginfoarr[idint].u;
		tv = rginfoarr[idint].v;
		DOUBLE tempdis = pow(tl-cl, 2) 
			+ pow(tu-cu, 2) + pow(tv-cv, 2);
		if (tempdis<mindis)
		{
			mindis = tempdis;//最大距离和对应的相邻区ID；
			outid = idint;
		}		
	}

	return outid;
}

int  FindMergedRgnMaxbias(int idint, int* mergearr, int bias)
//大阈值终止查找合并区，用于coarse watershed, 
//调用者必须保证idint有效，即：mergearr[idint]>0；
//以及mergearr有效，即：mergearr[idint]<idint;
{
	int outid = idint;
	while ( mergearr[outid]<bias )
	{
		outid = mergearr[outid];
	}
	return mergearr[outid];
}


void  AddNeiRgn(int curid, int neiid, CString* neiarr)
//增加neiid为curid的相邻区
{
	CString tempneis = neiarr[curid];//当前的相邻区；
	CString toaddstr;
	toaddstr.Format("%d ", neiid);

	int temppos = tempneis.Find(toaddstr, 0);
	while (temppos>0 && neiarr[curid].GetAt(temppos-1)!=' ')
	{
		temppos = neiarr[curid].Find(toaddstr, temppos+1);
	}
	
	if ( temppos<0 )
	{
		//当前相邻区中没有tempneis,则加入
		neiarr[curid] += toaddstr;
	}
}

void  AddNeiOfCur(int curid, int left, int right, int up, int down, int* flag, CString* neiarr)
//刷新当前点的所有相邻区；
{
	int leftid, rightid, upid, downid;
	leftid = rightid = upid = downid = curid;
	if (left>=0)
	{
		leftid = flag[left];
		if (leftid!=curid)
		{
			//邻点属于另一区, 加邻域点信息；
			AddNeiRgn(curid, leftid, neiarr);
		}
	}
	if (right>0)
	{
		rightid = flag[right];
		if (rightid!=curid)
		{
			//邻点属于另一区, 加邻域点信息；
			AddNeiRgn(curid, rightid, neiarr);
		}
	}
	if (up>=0)
	{
		upid = flag[up];
		if (upid!=curid)
		{
			//邻点属于另一区, 加邻域点信息；
			AddNeiRgn(curid, upid, neiarr);
		}
	}
	if (down>0)
	{
		downid = flag[down];
		if (downid!=curid)
		{
			//邻点属于另一区, 加邻域点信息；
			AddNeiRgn(curid, downid, neiarr);
		}
	}
}
/*************************************************************************
 *
 * \函数名称：
 *   RegionSegFixThreshold()
 *
 * \输入参数:
 *   CDIB * pDib					- 指向CDIB类的指针，含有原始图象信息
 *   int nThreshold					- 区域分割的阈值
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   1（逻辑）表示对应象素为前景区域，0表示背景
 *   阈值分割的关键问题在于阈值的选取。阈值的选取一般应该视实际的应用而
 *   灵活设定。
 *
 *************************************************************************
 */
/*void RegionSegFixThreshold(CDIB * pDib, int nThreshold)
{
	//遍历图象的纵坐标
	int y;

	//遍历图象的横坐标
	int x;

	//图象的长宽大小
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	//图像在计算机在存储中的实际大小
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	//图像在内存中每一行象素占用的实际空间
	int nSaveWidth = sizeImageSave.cx;

	
	//图像数据的指针
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
 * \函数名称：
 *   RegionSegAdaptive()
 *
 * \输入参数:
 *   CDIB * pDib					- 指向CDIB类的指针，含有原始图象信息
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   1（逻辑）表示对应象素为前景区域，0表示背景
 *   阈值分割的关键问题在于阈值的选取。阈值的选取一般应该视实际的应用而
 *   灵活设定。本函数中，阈值不是固定的，而是根据图象象素的实际性质而设定的。
 *   这个函数把图像分成四个子图象，然后计算每个子图象的均值，根据均值设置阈值
 *   阈值只是应用在对应的子图象
 *
 *************************************************************************
 */
/*void RegionSegAdaptive(CDIB * pDib)
{
	//遍历图象的纵坐标
	int y;

	//遍历图象的横坐标
	int x;

	//图象的长宽大小
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	//图像在计算机在存储中的实际大小
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	//图像在内存中每一行象素占用的实际空间
	int nSaveWidth = sizeImageSave.cx;

	//图像数据的指针
	LPBYTE  lpImage = pDib->m_lpImage;

	// 局部阈值
	int nThd[2][2] ;

	// 子图象的平均值
	int nLocAvg ;
	
	// 对左上图像逐点扫描：
	nLocAvg = 0 ;
	// y方向
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x方向
		for(x=0; x<nWidth/2 ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// 计算均值
	nLocAvg /= ( (nHeight/2) * (nWidth/2) ) ;

	// 设置阈值为子图象的平均值
	nThd[0][0] = nLocAvg ;

	// 对左上图像逐点扫描进行分割：
	// y方向
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x方向
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
	// 对左下图像逐点扫描：
	nLocAvg = 0 ;
	// y方向
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x方向
		for(x=0; x<nWidth/2 ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// 计算均值
	nLocAvg /= ( (nHeight - nHeight/2) * (nWidth/2) ) ;

	// 设置阈值为子图象的平均值
	nThd[1][0] = nLocAvg ;

	// 对左下图像逐点扫描进行分割：
	// y方向
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x方向
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
	// 对右上图像逐点扫描：
	nLocAvg = 0 ;
	// y方向
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x方向
		for(x=nWidth/2; x<nWidth ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// 计算均值
	nLocAvg /= ( (nHeight/2) * (nWidth - nWidth/2) ) ;
	
	// 设置阈值为子图象的平均值
	nThd[0][1] = nLocAvg ;

	// 对右上图像逐点扫描进行分割：
	// y方向
	for(y=0; y<nHeight/2 ; y++ )
	{
		// x方向
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
	// 对右下图像逐点扫描：
	nLocAvg = 0 ;
	// y方向
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x方向
		for(x=nWidth/2; x<nWidth ; x++ )
		{
			nLocAvg += lpImage[y*nSaveWidth + x];
		}
	}
	// 计算均值
	nLocAvg /= ( (nHeight - nHeight/2) * (nWidth - nWidth/2) ) ;

	// 设置阈值为子图象的平均值
	nThd[1][1] = nLocAvg ;

	// 对右下图像逐点扫描进行分割：
	// y方向
	for(y=nHeight/2; y<nHeight ; y++ )
	{
		// x方向
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
	
	// 为了显示方便显示，逻辑1用黑色显示，逻辑0用白色显示
	for(y=0; y<nHeight ; y++ )
	{
		// x方向
		for(x=0; x<nWidth ; x++ )
		{
			lpImage[y*nSaveWidth + x] = 255 - lpImage[y*nSaveWidth + x] ;
		}
	}
}*/

/*************************************************************************
 *
 * \函数名称：
 *   RobertsOperator()
 *
 * \输入参数:
 *   CDIB * pDib		- 指向CDIB类的指针，含有原始图象信息
 *   double * pdGrad	- 指向梯度数据的指针，含有图像的梯度信息
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   Roberts算子
 *
 *************************************************************************
 */
/*void RobertsOperator(CDIB * pDib, double * pdGrad)
{
	// 遍历图象的纵坐标
	int y;

	// 遍历图象的横坐标
	int x;

	// 图象的长宽大小
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	// 图像在计算机在存储中的实际大小
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	// 图像在内存中每一行象素占用的实际空间
	int nSaveWidth = sizeImageSave.cx;

	// 图像数据的指针
	LPBYTE  pImageData = pDib->m_lpImage;

	// 初始化
	for(y=0; y<nHeight ; y++ )
		for(x=0 ; x<nWidth ; x++ )
		{
			*(pdGrad+y*nWidth+x)=0;
		}


	// 下面开始利用Roberts算子进行计算，为了保证计算所需要的
	// 的数据位于图像数据的内部，下面的两重循环的条件是
	// y<nHeight-1 而不是y<nHeight，相应的x方向也是x<nWidth-1
	// 而不是x<nWidth

	//这两个变量用来表示Roberts算子第一个模板的两个象素值
	int nUpLeft;
	int nDownRight;

	// 这两个变量用来表示Roberts算子第二个模板的两个象素值
	int nUpRight;
	int nDownLeft;

	// 这两个变量用来表示Roberts算子计算的结果
	int nValueOne;
	int nValueTwo;

	// 临时变量
	double dGrad;

	for(y=0; y<nHeight-1 ; y++ )
		for(x=0 ; x<nWidth-1 ; x++ )
		{
			// Roberts算子第一个模板需要的象素值
			nUpLeft		=*(pImageData+y*nSaveWidth+x) ; 
			nDownRight	=*(	pImageData+(y+1)*nSaveWidth+x+1	);
			nDownRight	*=-1;

			//Roberts算子的第一个模板计算结果
			nValueOne	=nUpLeft+nDownRight	;

			// Roberts算子第二个模板需要的象素值
			nUpRight	=*(	pImageData+y*nSaveWidth+x+1	)		;
			nDownLeft	=*(	pImageData+(y+1)*nSaveWidth+x	);
			nDownLeft	*=-1;

			// Roberts算子的第二个模板计算结果
			nValueTwo	=nUpRight+nDownLeft;

			// 计算两个偏导数的平方和
			dGrad=nValueOne*nValueOne + nValueTwo*nValueTwo;

			// 开方
			dGrad=pow(dGrad,0.5);

			// 范数采用欧式距离
			*(pdGrad+y*nWidth+x)=dGrad;
		}
}*/
/*************************************************************************
 *
 * \函数名称：
 *   LaplacianOperator()
 *
 * \输入参数:
 *   CDIB * pDib		- 指向CDIB类的指针，含有原始图象信息
 *   double * pdGrad	- 指向梯度数据的指针，含有图像的梯度信息
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   LaplacianOperator算子,是二阶算子，不想Roberts算子那样需要两个模板计算
 *   梯度，LaplacianOperator算子只要一个算子就可以计算梯度。但是因为利用了
 *   二阶信息，对噪声比较敏感
 *
 *************************************************************************
 */
/*void LaplacianOperator(CDIB * pDib, double * pdGrad)
{
		// 遍历图象的纵坐标
	int y;

	// 遍历图象的横坐标
	int x;

	// 图象的长宽大小
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	// 图像在计算机在存储中的实际大小
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	// 图像在内存中每一行象素占用的实际空间
	int nSaveWidth = sizeImageSave.cx;

	// 图像数据的指针
	LPBYTE  lpImage = pDib->m_lpImage;

	// 初始化
	for(y=0; y<nHeight ; y++ )
		for(x=0 ; x<nWidth ; x++ )
		{
			*(pdGrad+y*nWidth+x)=0;
		}

	// 设置模板系数
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



	//这个变量用来表示Laplacian算子象素值
	int nTmp[3][3];
	
	// 临时变量
	double dGrad;

	// 模板循环控制变量
	int yy ;
	int xx ;

	
	// 下面开始利用Laplacian算子进行计算，为了保证计算所需要的
	// 的数据位于图像数据的内部，下面的两重循环的条件是
	// y<nHeight-2 而不是y<nHeight，相应的x方向也是x<nWidth-2
	// 而不是x<nWidth
	for(y=1; y<nHeight-2 ; y++ )
		for(x=1 ; x<nWidth-2 ; x++ )
		{
			dGrad = 0 ; 
			// Laplacian算子需要的各点象素值

			// 模板第一行
			nTmp[0][0] = lpImage[(y-1)*nSaveWidth + x - 1 ] ; 
			nTmp[0][1] = lpImage[(y-1)*nSaveWidth + x     ] ; 
			nTmp[0][2] = lpImage[(y-1)*nSaveWidth + x + 1 ] ; 

			// 模板第二行
			nTmp[1][0] = lpImage[y*nSaveWidth + x - 1 ] ; 
			nTmp[1][1] = lpImage[y*nSaveWidth + x     ] ; 
			nTmp[1][2] = lpImage[y*nSaveWidth + x + 1 ] ; 

			// 模板第三行
			nTmp[2][0] = lpImage[(y+1)*nSaveWidth + x - 1 ] ; 
			nTmp[2][1] = lpImage[(y+1)*nSaveWidth + x     ] ; 
			nTmp[2][2] = lpImage[(y+1)*nSaveWidth + x + 1 ] ; 
			
			// 计算梯度
			for(yy=0; yy<3; yy++)
				for(xx=0; xx<3; xx++)
				{
					dGrad += nTmp[yy][xx] * nWeight[yy][xx] ;
				}
			
			// 梯度值写入内存
			*(pdGrad+y*nWidth+x)=dGrad;
		}

}*/

/*************************************************************************
 *
 * \函数名称：
 *   RegionGrow()
 *
 * \输入参数:
 *   CDIB * pDib					- 指向CDIB类的指针，含有原始图象信息
 *   unsigned char * pUnRegion		- 指向区域生长结果的指针
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   pUnRegion指针指向的数据区存储了区域生长的结果，其中1（逻辑）表示
 *	 对应象素为生长区域，0表示为非生长区域
 *   区域生长一般包含三个比较重要的问题：
 *		1. 种子点的选取
 *		2. 生长准则
 *		3. 终止条件
 *	 可以认为，这三个问题需要具体分析，而且每个问题解决的好坏直接关系到
 *	 区域生长的结果。
 *	 本函数的种子点选取为图像的中心，生长准则是相邻象素的象素值小于
 *	 nThreshold, 终止条件是一直进行到再没有满足生长准则需要的象素时为止
 *
 *************************************************************************
 */
/*void RegionGrow(CDIB * pDib, unsigned char * pUnRegion, int nThreshold)
{
	static int nDx[]={-1,0,1,0};
	static int nDy[]={0,1,0,-1};

	// 遍历图象的纵坐标
//	int y;

	// 遍历图象的横坐标
//	int x;

	// 图象的长宽大小
	CSize sizeImage		= pDib->GetDimensions();
	int nWidth			= sizeImage.cx		;
	int nHeight			= sizeImage.cy		;

	// 图像在计算机在存储中的实际大小
	CSize sizeImageSave	= pDib->GetDibSaveDim();

	// 图像在内存中每一行象素占用的实际空间
	int nSaveWidth = sizeImageSave.cx;

	// 初始化
	memset(pUnRegion,0,sizeof(unsigned char)*nWidth*nHeight);

	// 种子点
	int nSeedX, nSeedY;

	// 设置种子点为图像的中心
	nSeedX = nWidth /2 ;
	nSeedY = nHeight/2 ;

	// 定义堆栈，存储坐标
	int * pnGrowQueX ;
	int * pnGrowQueY ;
	
	// 分配空间
	pnGrowQueX = new int [nWidth*nHeight];
	pnGrowQueY = new int [nWidth*nHeight];

	// 图像数据的指针
	unsigned char *  pUnchInput =(unsigned char * )pDib->m_lpImage;
	
	// 定义堆栈的起点和终点
	// 当nStart=nEnd, 表示堆栈中只有一个点
	int nStart ;
	int nEnd   ;

	//初始化
	nStart = 0 ;
	nEnd   = 0 ;

	// 把种子点的坐标压入栈
	pnGrowQueX[nEnd] = nSeedX;
	pnGrowQueY[nEnd] = nSeedY;

	// 当前正在处理的象素
	int nCurrX ;
	int nCurrY ;

	// 循环控制变量
	int k ;

	// 图象的横纵坐标,用来对当前象素的4邻域进行遍历
	int xx;
	int yy;

	while (nStart<=nEnd)
	{
		// 当前种子点的坐标
		nCurrX = pnGrowQueX[nStart];
		nCurrY = pnGrowQueY[nStart];					

		// 对当前点的4邻域进行遍历
		for (k=0; k<4; k++)	
		{	
			// 4邻域象素的坐标
			xx = nCurrX+nDx[k];
			yy = nCurrY+nDy[k];
			
			// 判断象素(xx，yy) 是否在图像内部
			// 判断象素(xx，yy) 是否已经处理过
			// pUnRegion[yy*nWidth+xx]==0 表示还没有处理

			// 生长条件：判断象素(xx，yy)和当前象素(nCurrX,nCurrY) 象素值差的绝对值
			if (	(xx < nWidth) && (xx>=0) && (yy<nHeight) && (yy>=0) 
				    && (pUnRegion[yy*nWidth+xx]==0) 
					&& abs(pUnchInput[yy*nSaveWidth+xx] - pUnchInput[nCurrY*nSaveWidth+nCurrX])<nThreshold )
			{
				// 堆栈的尾部指针后移一位
				nEnd++;

				// 象素(xx，yy) 压入栈
				pnGrowQueX[nEnd] = xx;
				pnGrowQueY[nEnd] = yy;

				// 把象素(xx，yy)设置成逻辑1（255）
				// 同时也表明该象素处理过
				pUnRegion[yy*nWidth+xx] = 255 ;
			}
		}
		nStart++;
	}

	// 释放内存
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
 * \函数名称：
 *   SobelOperator()
 *
 * \输入参数:
 *   CDIB * pDib		  - 指向CDIB类的指针，含有原始图象信息
 *   double * pdGrad	- 指向梯度数据的指针，含有图像的梯度信息
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   Sobe算子
 *
 *   并行边界分割
 *
 *************************************************************************
 */
//void SobelOperator(CDIB * pDib, double * pdGrad)
//{}

/*************************************************************************
 *
 * \函数名称：
 *   PrewittOperator()
 *
 * \输入参数:
 *   CDIB * pDib		  - 指向CDIB类的指针，含有原始图象信息
 *   double * pdGrad	- 指向梯度数据的指针，含有图像的梯度信息
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   Prewitt算子
 *
 *   并行边界分割
 *
 *************************************************************************
 */
//void PrewittOperator(CDIB * pDib, double * pdGrad)
//{}

/*************************************************************************
 *
 * \函数名称：
 *   EdgeTrack()
 *
 * \输入参数:
 *   CDIB * pDib					- 指向CDIB类的指针，含有原始图象信息
 *   unsigned char * pUnEdgeTrack	- 指向边界跟踪结果的指针
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   pUnEdgeTrack指针指向的数据区存储了边界跟踪的结果，其中1（逻辑）表示
 *	 对应象素为边界点，0表示为非边界点
 *
 *   串行边界分割
 *
 *************************************************************************
 */
//void EdgeTrack(CDIB * pDib, unsigned char * pUnEdgeTrack)
//{}


/*************************************************************************
 *
 * \函数名称：
 *   MakeGauss()
 *
 * \输入参数:
 *   double sigma									        - 高斯函数的标准差
 *   double **pdKernel										- 指向高斯数据数组的指针
 *   int *pnWindowSize										- 数据的长度
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   这个函数可以生成一个一维的高斯函数的数字数据，理论上高斯数据的长度应
 *   该是无限长的，但是为了计算的简单和速度，实际的高斯数据只能是有限长的
 *   pnWindowSize就是数据长度
 *   
 *************************************************************************
 */
void MakeGauss(double sigma, double **pdKernel, int *pnWindowSize)
{
	// 循环控制变量
	int i   ;
	
	// 数组的中心点
	int nCenter;

	// 数组的某一点到中心点的距离
	double dDis;


	// 中间变量
	double  dValue; 
	double  dSum;
	dSum = 0 ; 
	double pie;
	pie=3.14159;
	// 数组长度，根据概率论的知识，选取[-3*sigma, 3*sigma]以内的数据。
	// 这些数据会覆盖绝大部分的滤波系数
	*pnWindowSize = 1 + 2 * (int)ceil(3 * sigma);
	
	// 中心
	nCenter = (*pnWindowSize) / 2;
	
	// 分配内存
	*pdKernel = new double[*pnWindowSize] ;
	
	for(i=0; i< (*pnWindowSize); i++)
	{
		dDis = (double)(i - nCenter);
		dValue = exp(-(1/2)*dDis*dDis/(sigma*sigma)) / (sqrt(2 * pie) * sigma );
		(*pdKernel)[i] = dValue ;
		dSum += dValue;
	}
	
	// 归一化
	for(i=0; i<(*pnWindowSize) ; i++)
	{
		(*pdKernel)[i] /= dSum;
	}
}

/*************************************************************************
 *
 * \函数名称：
 *   GaussianSmooth()
 *
 * \输入参数:
 *   unsigned char * pUnchImg				- 指向图象数据的指针
 *   int nWidth											- 图象数据宽度
 *   int nHeight										- 图象数据高度
 *   double dSigma									- 高斯函数的标准差
 *   unsigned char * pUnchSmthdImg	- 指向经过平滑之后的图象数据
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   为了抑止噪声，采用高斯滤波对图象进行滤波，滤波先对x方向进行，然后对
 *   y方向进行。
 *   
 *************************************************************************
 */
void GaussianSmooth(unsigned char *pUnchImg, int nWidth, int nHeight, 
										double sigma, unsigned char * pUnchSmthdImg)
{
	// 循环控制变量
  int y;
	int x;
	
	int i;
	
	// 高斯滤波器的数组长度
	
	int nWindowSize;
	
	//  窗口长度的1/2
	int	nHalfLen;   
	
	// 一维高斯数据滤波器
	double *pdKernel ;
	
	// 高斯系数与图象数据的点乘
	double  dDotMul     ;
	
	// 高斯滤波系数的总和
	double  dWeightSum     ;          
  
	// 中间变量
	double * pdTmp ;
	
	// 分配内存
	pdTmp = new double[nWidth*nHeight];
	
	// 产生一维高斯数据滤波器
	// MakeGauss(sigma, &dKernel, &nWindowSize);
	MakeGauss(sigma, &pdKernel, &nWindowSize) ;
	
	// MakeGauss返回窗口的长度，利用此变量计算窗口的半长
	nHalfLen = nWindowSize / 2;
	
  // x方向进行滤波
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			dDotMul		= 0;
			dWeightSum = 0;
			for(i=(-nHalfLen); i<=nHalfLen; i++)
			{
				// 判断是否在图象内部
				if( (i+x) >= 0  && (i+x) < nWidth )
				{
					dDotMul += (double)pUnchImg[y*nWidth + (i+x)] * pdKernel[nHalfLen+i];
					dWeightSum += pdKernel[nHalfLen+i];
				}
			}
			pdTmp[y*nWidth + x] = dDotMul/dWeightSum ;
		}
	}
	
	// y方向进行滤波
	for(x=0; x<nWidth; x++)
	{
		for(y=0; y<nHeight; y++)
		{
			dDotMul		= 0;
			dWeightSum = 0;
			for(i=(-nHalfLen); i<=nHalfLen; i++)
			{
				// 判断是否在图象内部
				if( (i+y) >= 0  && (i+y) < nHeight )
				{
					dDotMul += (double)pdTmp[(y+i)*nWidth + x] * pdKernel[nHalfLen+i];
					dWeightSum += pdKernel[nHalfLen+i];
				}
			}
			pUnchSmthdImg[y*nWidth + x] = (unsigned char)(int)dDotMul/dWeightSum ;
		}
	}

	// 释放内存
	delete []pdKernel;
	pdKernel = NULL ;
	
	delete []pdTmp;
	pdTmp = NULL;
}


/*************************************************************************
 *
 * \函数名称：
 *   DirGrad()
 *
 * \输入参数:
 *   unsigned char *pUnchSmthdImg					- 经过高斯滤波后的图象
 *   int nWidht														- 图象宽度
 *   int nHeight      										- 图象高度
 *   int *pnGradX                         - x方向的方向导数
 *   int *pnGradY                         - y方向的方向导数
 * \返回值:
 *   无
 *
 * \说明:
 *   这个函数计算方向倒数，采用的微分算子是(-1 0 1) 和 (-1 0 1)'(转置)
 *   计算的时候对边界象素采用了特殊处理
 *   
 *   
 *************************************************************************
 */
void DirGrad(unsigned char *pUnchSmthdImg, int nWidth, int nHeight,
										 int *pnGradX , int *pnGradY)
{
	// 循环控制变量
	int y ;
	int x ;
	
	// 计算x方向的方向导数，在边界出进行了处理，防止要访问的象素出界
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			pnGradX[y*nWidth+x] = (int) ( pUnchSmthdImg[y*nWidth+min(nWidth-1,x+1)]  
													          - pUnchSmthdImg[y*nWidth+max(0,x-1)]     );
		}
	}

	// 计算y方向的方向导数，在边界出进行了处理，防止要访问的象素出界
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
 * \函数名称：
 *   GradMagnitude()
 *
 * \输入参数:
 *   int *pnGradX                         - x方向的方向导数
 *   int *pnGradY                         - y方向的方向导数
 *   int nWidht														- 图象宽度
 *   int nHeight      										- 图象高度
 *   int *pnMag                           - 梯度幅度   
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   这个函数利用方向倒数计算梯度幅度，方向倒数是DirGrad函数计算的结果
 *   
 *************************************************************************
 */
void GradMagnitude(int *pnGradX, int *pnGradY, int nWidth, int nHeight, int *pnMag)
{
	
	// 循环控制变量
	int y ;
	int x ;

	// 中间变量
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
 * \函数名称：
 *   NonmaxSuppress()
 *
 * \输入参数:
 *   int *pnMag                - 梯度图
 *   int *pnGradX							 - x方向的方向导数	
 *   int *pnGradY              - y方向的方向导数
 *   int nWidth                - 图象数据宽度
 *   int nHeight               - 图象数据高度
 *   unsigned char *pUnchRst   - 经过NonmaxSuppress处理后的结果
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   抑止梯度图中非局部极值点的象素。
 *   
 *************************************************************************
 */
void NonmaxSuppress(int *pnMag, int *pnGradX, int *pnGradY, int nWidth, 
										int nHeight,	unsigned char *pUnchRst)
{
	// 循环控制变量
	int y ;
	int x ;
	int nPos;

	// x方向梯度分量
	int gx  ;
	int gy  ;

	// 临时变量
	int g1, g2, g3, g4 ;
	double weight  ;
	double dTmp1   ;
	double dTmp2   ;
	double dTmp    ;
	
	// 设置图象边缘部分为不可能的边界点
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
			
			// 如果当前象素的梯度幅度为0，则不是边界点
			if(pnMag[nPos] == 0 )
			{
				pUnchRst[nPos] = 0 ;
			}
			else
			{
				// 当前象素的梯度幅度
				dTmp = pnMag[nPos] ;
				
				// x，y方向导数
				gx = pnGradX[nPos]  ;
				gy = pnGradY[nPos]  ;

				// 如果方向导数y分量比x分量大，说明导数的方向更加“趋向”于y分量。
				if (abs(gy) > abs(gx)) 
				{
					// 计算插值的比例
					weight = fabs((float)gx)/fabs((float)gy); 

					g2 = pnMag[nPos-nWidth] ; 
					g4 = pnMag[nPos+nWidth] ;
					
					// 如果x，y两个方向的方向导数的符号相同
					// C是当前象素，与g1-g4的位置关系为：
					//	g1 g2 
					//		 C         
					//		 g4 g3 
					if (gx*gy > 0) 
					{ 					
						g1 = pnMag[nPos-nWidth-1] ;
						g3 = pnMag[nPos+nWidth+1] ;
					} 

					// 如果x，y两个方向的方向导数的符号相反
					// C是当前象素，与g1-g4的位置关系为：
					//	   g2 g1
					//		 C         
					//	g3 g4  
					else 
					{ 
						g1 = pnMag[nPos-nWidth+1] ;
						g3 = pnMag[nPos+nWidth-1] ;
					} 
				}
				
				// 如果方向导数x分量比y分量大，说明导数的方向更加“趋向”于x分量
				// 这个判断语句包含了x分量和y分量相等的情况
				else
				{
					// 计算插值的比例
					weight = fabs((float)gy)/fabs((float)gx); 
					
					g2 = pnMag[nPos+1] ; 
					g4 = pnMag[nPos-1] ;
					
					// 如果x，y两个方向的方向导数的符号相同
					// C是当前象素，与g1-g4的位置关系为：
					//	g3   
					//	g4 C g2       
					//       g1
					if (gx*gy > 0) 
					{				
						g1 = pnMag[nPos+nWidth+1] ;
						g3 = pnMag[nPos-nWidth-1] ;
					} 
					// 如果x，y两个方向的方向导数的符号相反
					// C是当前象素，与g1-g4的位置关系为：
					//	     g1
					//	g4 C g2       
					//  g3     
					else 
					{ 
						g1 = pnMag[nPos-nWidth+1] ;
						g3 = pnMag[nPos+nWidth-1] ;
					}
				}

				// 下面利用g1-g4对梯度进行插值
				{
					dTmp1 = weight*g1 + (1-weight)*g2 ;
					dTmp2 = weight*g3 + (1-weight)*g4 ;
					
					// 当前象素的梯度是局部的最大值
					// 该点可能是个边界点
					if(dTmp>=dTmp1 && dTmp>=dTmp2)
					{
						pUnchRst[nPos] = 128 ;
					}
					else
					{
						// 不可能是边界点
						pUnchRst[nPos] = 0 ;
					}
				}
			} //else
		} // for

	}
} 

/*************************************************************************
 *
 * \函数名称：
 *   TraceEdge()
 *
 * \输入参数:
 *   int    x									- 跟踪起点的x坐标 
 *   int    y									- 跟踪起点的y坐标
 *   int nLowThd							- 判断一个点是否为边界点的低阈值
 *   unsigned char *pUnchEdge - 记录边界点的缓冲区
 *   int *pnMag               - 梯度幅度图
 *   int nWidth               - 图象数据宽度
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   递归调用  
 *   从(x,y)坐标出发，进行边界点的跟踪，跟踪只考虑pUnchEdge中没有处理并且
 *   可能是边界点的那些象素(=128),象素值为0表明该点不可能是边界点，象素值
 *   为255表明该点已经被设置为边界点，不必再考虑
 *   
 *   
 *************************************************************************
 */
void TraceEdge (int y, int x, int nLowThd, unsigned char *pUnchEdge, int *pnMag, int nWidth) 
{ 
	// 对8邻域象素进行查询
	int xNb[8] = {1, 1, 0,-1,-1,-1, 0, 1} ;
	int yNb[8] = {0, 1, 1, 1,0 ,-1,-1,-1} ;

	int yy ;
	int xx ;
	
	int k  ;
	
	for(k=0; k<8; k++)
	{
		yy = y + yNb[k] ;
		xx = x + xNb[k] ;
		// 如果该象素为可能的边界点，又没有处理过
		// 并且梯度大于阈值
		if(pUnchEdge[yy*nWidth+xx] == 128  && pnMag[yy*nWidth+xx]>=nLowThd)
		{
			// 把该点设置成为边界点
			pUnchEdge[yy*nWidth+xx] = 255 ;

			// 以该点为中心进行跟踪
			TraceEdge(yy, xx, nLowThd, pUnchEdge, pnMag, nWidth);
		}
	}
} 

/*************************************************************************
 *
 * \函数名称：
 *   EstimateThreshold()
 *
 * \输入参数:
 *   int *pnMag               - 梯度幅度图
 *	 int nWidth               - 图象数据宽度
 *	 int nHeight              - 图象数据高度
 *   int *pnThdHigh           - 高阈值
 *   int *pnThdLow            - 低阈值
 *	 double dRatioLow         - 低阈值和高阈值之间的比例
 *	 double dRatioHigh        - 高阈值占图象象素总数的比例
 *   unsigned char *pUnchEdge - 经过non-maximum处理后的数据
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   经过non-maximum处理后的数据pUnchEdge，统计pnMag的直方图，确定阈值。
 *   本函数中只是统计pUnchEdge中可能为边界点的那些象素。然后利用直方图，
 *   根据dRatioHigh设置高阈值，存储到pnThdHigh。利用dRationLow和高阈值，
 *   设置低阈值，存储到*pnThdLow。dRatioHigh是一种比例：表明梯度小于
 *   *pnThdHigh的象素数目占象素总数目的比例。dRationLow表明*pnThdHigh
 *   和*pnThdLow的比例，这个比例在canny算法的原文里，作者给出了一个区间。
 *
 *************************************************************************
 */
void EstimateThreshold(int *pnMag, int nWidth, int nHeight, int *pnThdHigh,int *pnThdLow, 
											 unsigned char * pUnchEdge, double dRatioHigh, double dRationLow) 
{ 
	// 循环控制变量
	int y;
	int x;
	int k;
	
	// 该数组的大小和梯度值的范围有关，如果采用本程序的算法，那么梯度的范围不会超过pow(2,10)
	int nHist[1024] ;

	// 可能的边界数目
	int nEdgeNb     ;

	// 最大梯度值
	int nMaxMag     ;

	int nHighCount  ;

	nMaxMag = 0     ; 
	
	// 初始化
	for(k=0; k<1024; k++) 
	{
		nHist[k] = 0; 
	}

	// 统计直方图，然后利用直方图计算阈值
	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{
			// 只是统计那些可能是边界点，并且还没有处理过的象素
			if(pUnchEdge[y*nWidth+x]==128)
			{
				nHist[ pnMag[y*nWidth+x] ]++;
			}
		}
	}

	nEdgeNb = nHist[0]  ;
	nMaxMag = 0         ;
	// 统计经过“非最大值抑止(non-maximum suppression)”后有多少象素
	for(k=1; k<1024; k++)
	{
		if(nHist[k] != 0)
		{
			// 最大梯度值
			nMaxMag = k;
		}
		
		// 梯度为0的点是不可能为边界点的
		// 经过non-maximum suppression后有多少象素
		nEdgeNb += nHist[k];
	}

	// 梯度比高阈值*pnThdHigh小的象素点总数目
	nHighCount = (int)(dRatioHigh * nEdgeNb +0.5);
	
	k = 1;
	nEdgeNb = nHist[1];
	
	// 计算高阈值
	while( (k<(nMaxMag-1)) && (nEdgeNb < nHighCount) )
	{
		k++;
		nEdgeNb += nHist[k];
	}

	// 设置高阈值
	*pnThdHigh = k ;

	// 设置低阈值
	*pnThdLow  = (int)((*pnThdHigh) * dRationLow+ 0.5);
}

/*************************************************************************
 *
 * \函数名称：
 *   Hysteresis()
 *
 * \输入参数:
 *   int *pnMag               - 梯度幅度图
 *	 int nWidth               - 图象数据宽度
 *	 int nHeight              - 图象数据高度
 *	 double dRatioLow         - 低阈值和高阈值之间的比例
 *	 double dRatioHigh        - 高阈值占图象象素总数的比例
 *   unsigned char *pUnchEdge - 记录边界点的缓冲区
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   本函数实现类似“磁滞现象”的一个功能，也就是，先调用EstimateThreshold
 *   函数对经过non-maximum处理后的数据pUnchSpr估计一个高阈值，然后判断
 *   pUnchSpr中可能的边界象素(=128)的梯度是不是大于高阈值nThdHigh，如果比
 *   该阈值大，该点将作为一个边界的起点，调用TraceEdge函数，把对应该边界
 *   的所有象素找出来。最后，当整个搜索完毕时，如果还有象素没有被标志成
 *   边界点，那么就一定不是边界点。
 *   
 *************************************************************************
 */
void Hysteresis(int *pnMag, int nWidth, int nHeight, double dRatioLow, 
								double dRatioHigh, unsigned char *pUnchEdge)
{
	// 循环控制变量
	int y;
	int x;

	int nThdHigh ;
	int nThdLow  ;

	int nPos;

	// 估计TraceEdge需要的低阈值，以及Hysteresis函数使用的高阈值
	EstimateThreshold(pnMag, nWidth, nHeight, &nThdHigh, 
		               &nThdLow, pUnchEdge,dRatioHigh, dRatioLow);

  // 这个循环用来寻找大于nThdHigh的点，这些点被用来当作边界点，然后用
	// TraceEdge函数来跟踪该点对应的边界
   for(y=0; y<nHeight; y++)
	 {
      for(x=0; x<nWidth; x++)
			{
				nPos = y*nWidth + x ; 

				// 如果该象素是可能的边界点，并且梯度大于高阈值，该象素作为
				// 一个边界的起点
				if((pUnchEdge[nPos] == 128) && (pnMag[nPos] >= nThdHigh))
				{
					// 设置该点为边界点
					pUnchEdge[nPos] = 255;
					TraceEdge(y, x, nThdLow, pUnchEdge, pnMag, nWidth);
				}
      }
   }

	 // 那些还没有被设置为边界点的象素已经不可能成为边界点
   for(y=0; y<nHeight; y++)
	 {
		 for(x=0; x<nWidth; x++)
		 {
			 nPos = y*nWidth + x ; 
			 if(pUnchEdge[nPos] != 255)
			 {
				 // 设置为非边界点
				 pUnchEdge[nPos] = 0 ;
			 }
		 }
	 }
}


/*************************************************************************
 *
 * \函数名称：
 *   Canny()
 *
 * \输入参数:
 *   unsigned char *pUnchImage- 图象数据
 *	 int nWidth               - 图象数据宽度
 *	 int nHeight              - 图象数据高度
 *   double sigma             - 高斯滤波的标准方差
 *	 double dRatioLow         - 低阈值和高阈值之间的比例
 *	 double dRatioHigh        - 高阈值占图象象素总数的比例
 *   unsigned char *pUnchEdge - canny算子计算后的分割图
 *
 * \返回值:
 *   无
 *
 * \说明:
 *   canny分割算子，计算的结果保存在pUnchEdge中，逻辑1(255)表示该点为
 *   边界点，逻辑0(0)表示该点为非边界点。该函数的参数sigma，dRatioLow
 *   dRatioHigh，是需要指定的。这些参数会影响分割后边界点数目的多少
 *************************************************************************
 */
void Canny(unsigned char *pUnchImage, int nWidth, int nHeight, double sigma,
					 double dRatioLow, double dRatioHigh, unsigned char *pUnchEdge)
{
	// 经过高斯滤波后的图象数据
	unsigned char * pUnchSmooth ;
  
	// 指向x方向导数的指针
	int * pnGradX ; 

	// 指向y方向导数的指针
	int * pnGradY ;

	// 梯度的幅度
	int * pnGradMag ;

	pUnchSmooth  = new unsigned char[nWidth*nHeight] ;
	pnGradX      = new int [nWidth*nHeight]          ;
	pnGradY      = new int [nWidth*nHeight]          ;
	pnGradMag    = new int [nWidth*nHeight]          ;

	// 对原图象进行滤波
	GaussianSmooth(pUnchImage, nWidth, nHeight, sigma, pUnchSmooth) ;

	// 计算方向导数
	DirGrad(pUnchSmooth, nWidth, nHeight, pnGradX, pnGradY) ;

	// 计算梯度的幅度
	GradMagnitude(pnGradX, pnGradY, nWidth, nHeight, pnGradMag) ;

	// 应用non-maximum 抑制
	NonmaxSuppress(pnGradMag, pnGradX, pnGradY, nWidth, nHeight, pUnchEdge) ;

	// 应用Hysteresis，找到所有的边界
	Hysteresis(pnGradMag, nWidth, nHeight, dRatioLow, dRatioHigh, pUnchEdge);


	// 释放内存
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

