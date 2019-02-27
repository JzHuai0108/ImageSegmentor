// HC.cpp: implementation of the CHC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "image.h"
#include "HC.h"
#include "susan.h"
#include "GlobalApi.h"
#include "Thinner.h"
#include "MMOFUNC.h"
#include "segment-image.h"
//#include "mwcomtypes.h" 
//#include "regprops_idl_i.c" 
//#include "regprops_idl.h" 
#include "cv.h"
#include "highgui.h"

//using namespace std;
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//debug errors
//regmoment, to close opencv windows after application is exited cause objcore debug error and application process not terminated.
//in contrast, by the similar operation, regionthresh function does not elicit the debug error and few chances of unterminated application process.
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

short Referee(unsigned int a,unsigned int b, unsigned int c,unsigned int d)
{	
	if(a==b)
	{
		if(b==c)
		{
			if(d==c)
				return 0;
			else
				return 1;
		}
		else
		{
			if(d==b)
				return 2;
			else if(d==c)
				return 3;
			else
				return 4;
		}
	}
	else
	{
		if(c==a)
		{
			if(d==c)
				return 5;
			else if(d==b)
				return 6;
			else return 7;
		}
		else if(c==b)
		{
			if(d==a)
				return 8;
			else if(d==c)
				return 9;
			else return 10;
		}
		else
		{
			if(d==a)
				return 11;
			else if(d==b)
				return 12;
			else if(d==c)
				return 13;
			else return 14;
		}
	}
	
}

//input: the inf of k sufficing len<=2^(2*k),the cell location of the dither matrix
//output: the element corresponding the location in the dither matrix 2^(k)*2^(k),
//if the element is larger than len, the next element is retrieved.
int DitherGen(int len, int k, int& loc)
{
	int yloc=loc%(1<<k);
	int xloc=loc>>k;
	yloc^=xloc;
	int res=0;
	int d2i1=0,d2i=0,i=0;
	for(i=k;i>0;--i)
	{
		d2i1=yloc%2;
		d2i=xloc%2;
		res<<=2;
		res+=d2i1*2+d2i;
		xloc>>=1;
		yloc>>=1;	
	}
	if(res>=len)
		res=DitherGen(len,k,++loc);
	return res;
}

//input: U and V are partitions for size objects, and U[i] denotes the class of object i, 0 based.
//R and C are the number of classes in partition U and V. so the largest expected U[i] is R-1.
//output: rand index RI and its expectation EI
double RandIndex(int size,int*U,int R,int*V,int C,double &EI)
{
	int i=0,j=0;
	float*cT=new float[R*C];//Contigency table
	for(i=0;i<R*C;++i)
		cT[i]=0;
	float *UT=new float[R],*VT=new float[C];
	for(i=0;i<R;++i)
		UT[i]=0;
	for(i=0;i<C;++i)
		VT[i]=0;
	double snij2=0,sni2=0,snj2=0;
	double div=(double)size*(size-1)/2;
	for(i=0;i<size;++i)
	{
		++cT[U[i]*R+V[i]];
		++UT[U[i]];
		++VT[V[i]];
	}
	for(i=0;i<R;++i)
	{
		for(j=0;j<C;++j)
		{
			snij2+=cT[i*R+j]*(cT[i*R+j]-1)/2;
		}
		sni2+=UT[i]*(UT[i]-1)/2;
	}
	for(i=0;i<C;++i)
	{
		snj2+=VT[i]*(VT[i]-1)/2;
	}
	EI=((double)(2*sni2*snj2)/div-(sni2+snj2))/div+1;
	return 1+(double)(2*snij2-sni2-snj2)/div;
}

// 构造函数，初始化CHC对象的数据
CHC::CHC():Width(0),Height(0),data_(NULL),sData_(NULL),grid(NULL),Type(0),
d_(3),loop(1),eval(0),RI(0),
minsize(3),comps(0),K(20.0f),maxDelta(50.f),EM(NULL),ndv(NULL),tag(NULL),
wp(0.5f),wc(0.9f),hdiff(20.f)
{
}

// 析构函数
CHC::~CHC()
{
	Clear();
//	cvDestroyWindow("result");
}

// 清除以前的图像数据，并释放内存
void CHC::Clear()
{

/*	if(root)
	ElimTree(root);
	root=NULL;*/

	
	if(data_)
	{
		delete[] data_;
		delete[] sData_;
		data_=NULL;			
		sData_=NULL;
	}
	if(grid)	
		delete[]grid;			
	grid=NULL;
	if(A.size())
	{
		S.clear();
		A.clear();
	}
	if(EM)
		delete[]EM;
	EM=NULL;
	if(ndv)
		delete[]ndv;
	ndv=NULL;
	if(tag)
		delete[] tag;
	tag=NULL;
	bWArray.clear();


	
}



//initiate for gbs, input: comps
inline bool Prior(const NP&a,const NP&b)
	{return a.rInd<b.rInd;}
//input: graphbased or any other method seged image tag[L] 
//output: S with length comps current region number, tag2 for immediate father index record
//A is allocated large enough to store edges 
void CHC::InitiateRegions()
{
	int label=0,sernum=0,cand=0;
	int h=Height;
	int w=Width;
	int comp=0,compleft=0,compup=0,y=0,x=0,d=0;
	int L=w*h;
	int len=__min((2*L-w-h),4*comps);
	NPL::iterator nplp,nptemp,nptp2;
	A=vector<tPair>(len);
	S=vector<Region>(comps);
	grid=new CRect[comps];

	CRect abox(0,0,0,0);
//	assert(data_);
	for(y=0;y<comps;++y)
	{
		S[y].p=y;
		S[y].addition=data_+y*d_;
		S[y].sSum=sData_+y*d_;
		S[y].norbox=grid+y;
	//	grid[y]=abox;
	}
//		ofstream output3("reg82.txt",ios::app);


	for (y = 0; y < h; ++y)
	{
		for (x = 0; x < w; ++x)
		{
			comp =tag[label];
		//	if(comp==82)
		//		output3<<x<<"\t"<<y<<endl;
			abox.left=x;
			abox.right=x+1;
			abox.top=y;
			abox.bottom=y+1;
			if(S[comp].size==0)			
				grid[comp]=abox;
			else
				grid[comp].UnionRect(abox,grid[comp]);
			++(S[comp].size);
		/*	for(d=0;d<d_;++d)
			{
				S[comp].addition[d]+=data_[label*d_+d];
				S[comp].sSum[d]+=sData_[label*d_+d];
			}*/
			
			//initiate neighbor list and perim 
			//note that in each np stores neighbors index and common bl to save memory
			if(x==0&&y==0)//the head pixel
			{				
				S[comp].perim+=2;
				compleft=comp;			
				++label;
				continue;
			}
			if(x>0&&y==0)
			{						
				if(comp!=compleft)
				{	
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++(S[comp].perim);
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				}
				++S[comp].perim;
				if(x==w-1)
					++S[comp].perim;
				compleft=comp;
				++label;
				continue;
			}
			if(x==0&&y>0)
			{	
				compup=tag[label-w];
				if(compup!=comp)
				{	
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
				}			
				++S[comp].perim;
				if(y==h-1)
					++S[comp].perim;
				compleft=comp;			
				++label;
				continue;
			}
			compup=tag[label-w];
			if(compup==comp&&compleft==comp)
			{				
			}
			else if(compup==comp&&compleft!=comp)
			{
				NP nr(comp,1);
				S[compleft].NPList.push_back(nr);
				++S[compleft].perim;
				NP nl(compleft,1);
				S[comp].NPList.push_back(nl);
				++S[comp].perim;
			}
			else if(compup!=comp&&compleft==comp)
			{
				NP nu(compup,1);
				S[comp].NPList.push_back(nu);
				++S[comp].perim;
				NP nd(comp,1);
				S[compup].NPList.push_back(nd);
				++S[compup].perim;
			}
			else if(compup!=comp&&compleft==compup)
			{
				NP nl(compleft,2);
				S[comp].NPList.push_back(nl);
				S[comp].perim+=2;
				NP nr(comp,2);
				S[compup].NPList.push_back(nr);
				S[compup].perim+=2;
			}
			else
			{
				NP nu(compup,1);
				S[comp].NPList.push_back(nu);
				++S[comp].perim;
				NP nd(comp,1);
				S[compup].NPList.push_back(nd);
				++S[compup].perim;
				NP nl(compleft,1);
				S[comp].NPList.push_back(nl);
				++S[comp].perim;
				NP nr(comp,1);
				S[compleft].NPList.push_back(nr);
				++S[compleft].perim;
			}
			if((x==w-1)||(y==h-1))
			{
				if(x==w-1&&y==h-1)
					S[comp].perim+=2;
				else if(x==w-1&&y!=h-1)
					++S[comp].perim;
				else// if(x!=w-1&&y==h-1)
					++S[comp].perim;
			}
			compleft=comp;			
			++label;
		}//second loop
	} //first loop

	//initiate A
	int linknum=1;//start from 1, avoid 0 for linknum
	for(x=0;x<comps;++x)
	{
		S[x].InterDiff(d_,wc,wp);
		sort(S[x].NPList.begin(),S[x].NPList.end(),Prior);//because for list sort doesnot change
		//the position of elements while for vector sort changes the positions of elements, so
		//it's ok to combine this line with the following section for list<NP> but not feasible with vector<NP>
	}
	for(x=0;x<comps;++x)
	{
		nplp=S[x].NPList.begin();
		while(nplp!=S[x].NPList.end())
		{
			y=nplp->rInd;
			if(nplp->pInd<0)//this pair has been visited, so delete its successors with same rind
				//and go to the next neighbor
			{
				nplp->pInd=-(nplp->pInd);
				nptemp=++nplp;
				while(nplp!=S[x].NPList.end()&&(nplp->rInd)==y)
					++nplp;
				S[x].NPList.erase(nptemp,nplp);
				nplp=nptemp;//this line is added for vector<NP>
			}
			else
			{
				nptemp=nplp;//first item with rInd
				++nplp;
				while(nplp!=S[x].NPList.end()&&(nplp->rInd)==y)
				{	
					nptemp->pInd+=nplp->pInd;
					++nplp;
				}				
				//partner pair initiation notice
				//how to avoid second initiation for region pairs in S[y].Nplist?
				NP temp(x,0);
				nptp2=find(S[y].NPList.begin(),S[y].NPList.end(),temp);
				assert(nptp2!=S[y].NPList.end());
				nptp2->pInd=-linknum;//avoid redundancy visit trick 
				//if any region has pind negative after the for cycle, it signals error.
				//initiate A(unsigned int)
				A[linknum].bl=nptemp->pInd;//note that initially to save footprint store bl in pind
				nptemp->pInd=linknum;
				A[linknum].r[0]=x;
				A[linknum].r[1]=y;
				++linknum;
				S[x].NPList.erase(++nptemp,nplp);
				nplp=nptemp;//this line is intended for vector<NP>
				
			}
		}
	}

	InitializeASM(linknum,comps,GBS);
}
//initiate regions after graph based partition with 8-connectedness 
//interlen or comps denotes current region number which is equal to size of S 
void CHC::InitiateRegions8()
{
	unsigned int label=0;//the serial number
	unsigned int buoy=0;//count the region number
	int h=Height;
	int w=Width;
	int y=0,x=0,d=0;
	unsigned int comp=0,compup=0,compleft=0,compul=0,compd=0;
	int L=w*h;
	int len=__min((2*L-w-h),8*comps);
//	NPL::iterator nplp,nptemp,nptp2;
	A=vector<tPair>(len);
	S=vector<Region>(comps);
	grid=new CRect[comps];

	CRect abox(0,0,0,0);
//	assert(data_);
	for(y=0;y<comps;++y)
	{
		S[y].p=y;
		S[y].addition=data_+y*d_;
		S[y].sSum=sData_+y*d_;
		S[y].norbox=grid+y;
		grid[y]=abox;
	}

	for (y = 0; y < h; ++y)//each line
	{
		for (x = 0; x < w; ++x)//each pixel
		{
			comp =tag[label];
			abox.left=x;
			abox.right=x+1;
			abox.top=y;
			abox.bottom=y+1;
			if(S[comp].size==0)
			
				grid[comp]=abox;
			
			else			
			grid[comp].UnionRect(abox,grid[comp]);
			++(S[comp].size);
		
			if(x==0&&y==0)//the head pixel
			{				
				S[comp].perim+=2;
				compleft=comp;			
				++label;
				continue;
			}
			if(x>0&&y==0)
			{						
				if(comp!=compleft)
				{	
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++(S[comp].perim);
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				}
				++S[comp].perim;
				if(x==w-1)
					++S[comp].perim;
				compleft=comp;
				++label;
				continue;
			}	
			
			if(x==0&&y>0)
			{				
				compup=tag[label-w];
				compd=tag[label-w+1];
				if(compd==compup)
				{
					if(comp!=compd)
					{
						NP nd(compd,1);
						S[comp].NPList.push_back(nd);
						++(S[comp].perim);
						NP nb(comp,1);
						S[compd].NPList.push_back(nb);
						++S[compd].perim;
						
					}
				}
				else
				{
					if(comp==compd)
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++(S[comp].perim);
						NP nb(comp,1);
						S[compup].NPList.push_back(nb);
						++S[compup].perim;
						
					}
					else if(comp!=compup&&comp!=compd)
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++(S[comp].perim);
						NP nd(compd,0);
						S[comp].NPList.push_back(nd);
						NP nk(comp,0);
						S[compd].NPList.push_back(nk);
						NP nb(comp,1);
						S[compup].NPList.push_back(nb);
						++S[compup].perim;
						
					}
				}
				
				++S[comp].perim;
				if(y==h-1)
					++S[comp].perim;
				compleft=comp;
				compul=compup;
				compup=compd;
				++label;
				continue;
			}
			
			if(x==w-1&&y>0)
			{				
				switch(Referee(compleft,compul,compup))
				{
				case 1:
					if(comp!=compup)
					{
						NP nr(comp,2);
						S[compup].NPList.push_back(nr);
						S[compup].perim+=2;
						NP nl(compup,2);
						S[comp].NPList.push_back(nl);
						S[comp].perim+=2;
					}
					break;
				case 4:	
				case 10:
					if(comp!=compup&&comp!=compleft)					
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++S[comp].perim;
						NP nd(comp,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nl(compleft,1);
						S[comp].NPList.push_back(nl);
						++S[comp].perim;
						NP nr(comp,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						
					}
					else
					{
						NP nd(compleft,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nr(compup,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						
					}
					break;
				case 7:					
					if(comp!=compup&&comp!=compul)					
					{
						NP nu(compup,2);
						S[comp].NPList.push_back(nu);
						S[comp].perim+=2;
						NP nd(comp,2);
						S[compup].NPList.push_back(nd);
						S[compup].perim+=2;
						NP nl(compul,0);
						S[comp].NPList.push_back(nl);
						NP nr(comp,0);
						S[compul].NPList.push_back(nr);
						
					}
					else if(comp==compul)
					{
						NP nl(compleft,2);
						S[comp].NPList.push_back(nl);
						S[comp].perim+=2;
						NP nr(comp,2);
						S[compleft].NPList.push_back(nr);
						S[compleft].perim+=2;
						
					}
					break;				
				case 14:
					if(comp!=compup&&comp!=compul&&comp!=compleft)				
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++S[comp].perim;
						NP nd(comp,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nl(compleft,1);
						S[comp].NPList.push_back(nl);
						++S[comp].perim;
						NP nr(comp,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						NP nk(compul,0);
						S[comp].NPList.push_back(nk);
						NP nb(comp,0);
						S[compul].NPList.push_back(nb);
						
					}
					else if(comp==compul)
					{
						NP nu(compup,1);
						S[comp].NPList.push_back(nu);
						++S[comp].perim;
						NP nd(comp,1);
						S[compup].NPList.push_back(nd);
						++S[compup].perim;
						NP nl(compleft,1);
						S[comp].NPList.push_back(nl);
						++S[comp].perim;
						NP nr(comp,1);
						S[compleft].NPList.push_back(nr);
						++S[compleft].perim;
						
					}
					break;
				default:
					AfxMessageBox("Error happens when initiate 8 connected regions!");
					break;
				}
				++S[comp].perim;
				if(y==h-1)
					++S[comp].perim;
				
				++label;
				continue;		
				
			}
			compd=tag[label-w+1];
			switch(Referee(compleft,compul,compup,compd))
			{
			case 0:
				if(comp!=compup)
				{
					NP nr(comp,2);
					S[compup].NPList.push_back(nr);
					S[compup].perim+=2;
					NP nl(compup,2);
					S[comp].NPList.push_back(nl);
					S[comp].perim+=2;
					
				}
				break;
			case 1:			
				if(comp!=compup&&comp!=compd)					
				{
					NP nu(compup,2);
					S[comp].NPList.push_back(nu);
					S[comp].perim+=2;
					NP nd(comp,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					NP nl(compd,0);
					S[comp].NPList.push_back(nl);
					NP nr(comp,0);
					S[compd].NPList.push_back(nr);
					
					
				}
				else if(comp==compd)
				{
					NP nu(compup,2);
					S[comp].NPList.push_back(nu);
					S[comp].perim+=2;
					NP nd(comp,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					
				}
				break;
			case 8:
			case 9:
				if(comp!=compup&&comp!=compleft)					
				{
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++S[comp].perim;
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
					
					
				}
				else
				{
					NP nd(compleft,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nr(compup,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
					
				}
				break;
			case 2:				
			case 3:
				if(comp!=compup&&comp!=compul)					
				{
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nl(compul,1);
					S[comp].NPList.push_back(nl);
					++S[comp].perim;
					NP nr(comp,1);
					S[compul].NPList.push_back(nr);
					++S[compul].perim;
					
				}
				else
				{
					NP nd(compul,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nr(compup,1);
					S[compul].NPList.push_back(nr);
					++S[compul].perim;
					
				}
				break;
			case 5:				
			case 6:
				if(comp!=compup&&comp!=compul)					
				{
					
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nd(comp,1);
					S[compup].NPList.push_back(nd);
					++S[compup].perim;
					NP nl(compul,0);
					S[comp].NPList.push_back(nl);
					NP nr(comp,0);
					S[compul].NPList.push_back(nr);
					
				}
				else if(comp==compul)
				{
					NP nd(compul,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					NP nl(compup,2);
					S[compul].NPList.push_back(nl);
					S[compul].perim+=2;
					
				}
				break;
			case 4:
			case 10:
				if(comp==compleft)				
				{
					NP nu(compd,0);
					S[compleft].NPList.push_back(nu);
					
					NP nd(compleft,0);
					S[compd].NPList.push_back(nd);
					
					NP nl(compup,1);
					S[compleft].NPList.push_back(nl);
					++S[compleft].perim;
					NP nr(compleft,1);
					S[compup].NPList.push_back(nr);
					++S[compup].perim;
					
				}
				else if(comp==compd)
				{
					NP nu(compd,1);
					S[compleft].NPList.push_back(nu);
					++S[compleft].perim;
					NP nd(compleft,1);
					S[compd].NPList.push_back(nd);
					++S[compd].perim;
					NP nl(compup,1);
					S[compd].NPList.push_back(nl);
					++S[compd].perim;
					NP nr(compd,1);
					S[compup].NPList.push_back(nr);
					++S[compup].perim;
			
				}
				else if(comp==compup)
				{
					NP nl(compleft,1);
					S[compup].NPList.push_back(nl);
					++S[compup].perim;
					NP nr(compup,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				
				}
				else				
				{
					NP nu(compup,1);
					S[comp].NPList.push_back(nu);
					++S[comp].perim;
					NP nb(comp,1);
					S[compup].NPList.push_back(nb);
					++S[compup].perim;
					NP nd(compd,0);
					S[comp].NPList.push_back(nd);
				//	++S[comp].perim;
					NP nk(comp,0);
					S[compd].NPList.push_back(nk);
					//++S[compd].perim;
					NP nl(compleft,1);
					S[comp].NPList.push_back(nl);
					++S[comp].perim;
					NP nr(comp,1);
					S[compleft].NPList.push_back(nr);
					++S[compleft].perim;
				
				}
				break;		
			case 7:
				if(comp==compul||comp==compd)				
				{
					NP nu(compd,0);
					S[compul].NPList.push_back(nu);
				//	++S[compul].perim;
					NP nd(compul,0);
					S[compd].NPList.push_back(nd);
				//	++S[compd].perim;
					NP nl(compup,2);
					S[comp].NPList.push_back(nl);
					S[comp].perim+=2;
					NP nr(comp,2);
					S[compup].NPList.push_back(nr);
					S[compup].perim+=2;
					
				}
				else if(comp!=compd&&comp!=compul&&comp!=compup)
				{
					NP nu(compup,2);
					S[comp].NPList.push_back(nu);
					S[comp].perim+=2;
					NP nd(comp,2);
					S[compup].NPList.push_back(nd);
					S[compup].perim+=2;
					NP nk(compd,0);
					S[comp].NPList.push_back(nk);
				
					NP nr(comp,0);
					S[compd].NPList.push_back(nr);
					
					NP nl(compul,0);
					S[comp].NPList.push_back(nl);
				
					NP nb(comp,0);
					S[compul].NPList.push_back(nb);
				
				}				
				break;						
				
			case 11:
			case 12:
			case 13:
			/*	if(comp==compleft||comp==compup)
				{
					Blocks[compup].NPList.push_back(compleft]+=1;					
					Blocks[compleft].NPList.push_back(compup]+=1;		
				}
				else if(comp==compul)
				{
					Blocks[compup].NPList.push_back(compul]+=1;					
					Blocks[compul].NPList.push_back(compup]+=1;
					Blocks[compleft].NPList.push_back(compul]+=1;					
					Blocks[compul].NPList.push_back(compleft]+=1;
				}
				else
				{
					Blocks[comp].NPList.push_back(compup]+=1;
					Blocks[comp].NPList.push_back(compul]+=0;
					Blocks[comp].NPList.push_back(compleft]+=1;					
					Blocks[compleft].NPList.push_back(comp]+=1;					
					Blocks[compul].NPList.push_back(comp]+=0;											
					Blocks[compup].NPList.push_back(comp]+=1;	
				}*/
				break;			
			case 14:
			/*	if(comp==compleft)					
				{
					Blocks[compleft].NPList.push_back(compd]+=0;						
					Blocks[compd].NPList.push_back(compleft]+=0;	
					Blocks[compleft].NPList.push_back(compup]+=1;						
					Blocks[compup].NPList.push_back(compleft]+=1;	
				}
				else if(comp==compul)					
				{
					Blocks[compul].NPList.push_back(compd]+=0;					
					Blocks[compd].NPList.push_back(compul]+=0;
					Blocks[compul].NPList.push_back(compleft]+=1;					
					Blocks[compleft].NPList.push_back(compul]+=1;
					Blocks[compul].NPList.push_back(compup]+=1;					
					Blocks[compup].NPList.push_back(compul]+=1;
				}
				else if(comp==compup)
				{					
					Blocks[compleft].NPList.push_back(compup]+=1;					
					Blocks[compup].NPList.push_back(compleft]+=1;
				}
				else if(comp==compd)					
				{
					Blocks[compleft].NPList.push_back(compd]+=1;					
					Blocks[compul].NPList.push_back(compd]+=0;						
					Blocks[compd].NPList.push_back(compleft]+=1;
					Blocks[compd].NPList.push_back(compul]+=0;
					Blocks[compd].NPList.push_back(compup]+=1;					
					Blocks[compup].NPList.push_back(compd]+=1;
				} 
				else 				
				{
					Blocks[comp].NPList.push_back(compup]+=1;
					Blocks[comp].NPList.push_back(compul]+=0;
					Blocks[comp].NPList.push_back(compleft]+=1;
					Blocks[comp].NPList.push_back(compd]+=0;					
					Blocks[compd].NPList.push_back(comp]+=0;					
					Blocks[compleft].NPList.push_back(comp]+=1;						
					Blocks[compul].NPList.push_back(comp]+=0;					
					Blocks[compup].NPList.push_back(comp]+=1;						
				}	*/			
				break;
			default:
				AfxMessageBox("Error happens when initiate 8 connected regions on the normal inner place!");
				break;
			}
			if(y==h-1)
				++S[comp].perim;
			compleft=comp;
			compul=compup;
			compup=compd;
			label++;
		}//second loop
	} //first loop 

	comps=buoy;

}


//note that S[len] in graph based seg;
// so EvalQS can only be called after regionLabel and before any change of tag[]
//tag[L] corresponds to S[L] in QT seg
//after conversion of coordination, repair is wanted.
void CHC::EvalQs(BYTE*lpBits,int bitCount)
{	
	//compute the average of each region
	assert(comps>0);
	int x=0,y=0,d=0;
	int w=Width,h=Height,sernum=0,acronym=0,trans=0,ext=0;
	int L=w*h;
	double pedler=0;
	int nByteWidth = BYTE_PER_LINE(Width,bitCount);
	BYTE *rowp=lpBits;
	BYTE *pBits=rowp;
	int*record=new int[comps]; 
	int*pi=record;//record region number has the same size with prededing ones
	//		record[y].first=y;
	int len=S.size();
	eval=0;
		
	double *errsum=new double[len];
	int spp=bitCount>24?3:bitCount/8;
	memset(errsum,0,sizeof(double)*len);
	for (y = 0; y < h; y++) 
	{
		for (x = 0; x < w; x++) 
		{	
			pedler=0;
			acronym=x*spp;
			trans=tag[sernum];
			for(d=0;d<d_;d++)
			{
				pedler+=square(rowp[acronym+d]/Range-S[trans].addition[d]/S[trans].size);
			}
			errsum[trans]+=sqrt(pedler);
			++sernum;
		}
		rowp+=nByteWidth;
	}
	for(x=0;x<len;x++)
	{
		if(errsum[x]==0)continue;
		errsum[x]=(1<<16)*square(errsum[x]);
	}
//	assert(rec==comps);it seems rec==comps does not always make up.
	if(len<L)
	{
		for(x=0;x<len;x++)
		{
			if(S[x].p==x)//reg
			{
				ext=S[x].size;	
				*(pi)=ext;
				++pi;
				errsum[x]/=(1+log(ext));
				eval+=errsum[x];
				//eval+=square(record[ext]/ext);
			}
		}
		assert(pi-record==comps);
		pi=record;
		sort(record,record+comps);
		while((pi-record)<comps)
		{ 
			y=*pi;
			ext=0;
			while(((pi-record)<comps)&&*pi==y)
			{
				++ext;
				++pi;
			}
			eval+=(double)ext*ext*ext/(y*y);
		}
	}
	else
	{
		for(x=0;x<L;x++)
		{
			if(tag[x]==x)//reg
			{
				ext=S[x].size;	
				*(pi)=ext;
				++pi;
				errsum[x]/=(1+log(ext));
				eval+=errsum[x];
			}
		}
		assert(pi-record==comps);
		pi=record;
		sort(record,record+comps);
		while((pi-record)<comps)
		{ 
			y=*pi;
			ext=0;
			while(((pi-record)<comps)&&*pi==y)
			{
				++ext;
				++pi;
			}
			eval+=(double)ext*ext*ext/(y*y);
		}
	}
	eval=eval*sqrt(comps)/(1e4*w*h);
	delete []errsum;
	delete []record;
}

//edgepoints finds edge candidate points in edge channel
//input:edge channel with arbitrary size output: edge map with 255 for edge point
//the edge channel is processed by gaussian filter and then canny filter
void CHC::EdgePoints(int w,int h,float sig, float low, float high)
{
	assert(EM);
	BYTE* pPostEdge = new BYTE[w*h];
	// 调用canny函数进行边界提取
	Canny(EM,w,h, sig,low,high, pPostEdge);
	
	delete []EM;
	EM=pPostEdge;
}

//a bug lies in killminion,after graph2tree4, and immediately usage of killminion may lead to some mysterious failure.

//input: tag2[len] in graph based seg or tag[L] for quadtree based seg recording the immediate father indices for each reg(pixel)
//output: tag[L] recording the ultimate father index for each pixel
void CHC::RegionLabel()
{	
	int width = Width;
	int height=Height;
	int L=width*height;
	int sernum=0,label=0;
	int x,y;
	int rec=0,len=S.size();
	if(len<L)
	{
		int *tag2=new int[len];
		//search father for each region
		for (x = 0; x <len; x++) 
		{
			label=x;
			while(label!=S[label].p)
			{
				label=S[label].p;
			}
			tag2[x]=label;
		}

		//father for each pixel
		sernum=0;
		for (y = 0; y <L; ++y) 
		{
		
				label=tag[sernum];
				tag[sernum]=tag2[label];
				++sernum;		
		}
		delete []tag2;
	}
	else
	{
		
		for (y = 0; y <L; ++y) 
		{
			
				label=sernum;
				
				while(label!=S[label].p)
				{
					label=S[label].p;
				}
				tag[sernum]=label;
				++sernum;					
		}	
	}

/*	ofstream out("label.txt",ios::app);
	out<<"comps:"<<comps<<"\nregion label\t\tsize in pixel\n";
	vector<int> trap;
	sernum=0;
	for (y = 0; y < L; ++y) 
	{
			//check the size distribution
			if(tag[sernum]==sernum)
				trap.push_back(sernum);
			++sernum;
	}
	//export size distribution
	for(y=0;y<comps;++y)
	{
		out<<trap[y]<<"\t\t"<<S[trap[y]].size<<"\n";
	}	*/
}
//input: original image
//output:tag[L] recording each father label for each pixel and father labels are minimized
//data_ and SUM_ are also shrinked.
int CHC::SegGraph4()
{
	assert(data_);
	int w=Width,h=Height;
	int L=w*h;
	int x,y,d;

	int sernum=0,rec=0;
	float alter=K/Range;
	GraphSeg4(data_,tag,d_,w,h,alter,minsize,comps);

	//minimize label value for each pixel stored in tag array
	for(sernum=0;sernum<L;sernum++)
	{
		if(tag[sernum]==sernum)
		{
			tag[sernum]=(-rec);//discrimate from no seed pixels
			++rec;
		}
	}
	assert(rec==comps);
	for(sernum=0;sernum<L;sernum++)
	{
		if(tag[sernum]<=0)
			tag[sernum]=(-tag[sernum]);//seed pixels
		else
			tag[sernum]=abs(tag[tag[sernum]]);//no seed points
	}
	/*check label with 4 connectedness
	ofstream output4("regche.txt",ios::app);
	for(y=0;y<L;++y)
	{
		if((tag[y]!=tag[y-1])&&(tag[y]!=tag[y+1])&&(tag[y]!=tag[y-w])&&(tag[y]!=tag[y+w]))
			output4<<y<<"\t"<<tag[y]<<"\n";
	}*/
	//rebuilt data_ and sSum_ since the previous is too large and not in tandem with tag
	float *data2=new float[comps*d_];
		float *sData2=new float[comps*d_];
	memset(data2,0,comps*d_*sizeof(float));
	memset(sData2,0,comps*d_*sizeof(float));
	for(sernum=0;sernum<L;sernum++)
	{
		x=sernum*d_;
		y=tag[sernum]*d_;
		for(d=0;d<d_;d++)
		{
			data2[y+d]+=data_[x+d];
			sData2[y+d]+=sData_[x+d];
		}
	}
	delete []data_;
	data_=data2;
	delete []sData_;
	sData_=sData2;
	return comps;
}
//this function initiate regions list for pure hierclust
void CHC::InitiateRegionSet()
{
	int height=Height;
	int width=Width;
	int L=height*width;
	comps=L;
	//if 3d data,the sequence of color is blue green red
	int i,j,y;
	int r,u,w;
	int sernum=0;
	S=vector<Region>(L);
	A=vector<tPair>(2*L-width-height);
	grid=new CRect[L];
	//initiate S
	for (  y = 0; y < L; y++)
	{
			//comp =*(alphaptr+tag);
			tag[y]=y;
			S[y].p=y;
			S[y].addition=data_+y*d_;
			S[y].sSum=sData_+y*d_;
			S[y].size=1;
			S[y].perim=4;
			S[y].interdif=(1-wc)*(1+3*wp);
			i=y%width;
			j=y/width;
			S[y].norbox=grid+y;
			grid[y].top=j;
			grid[y].bottom=j+1;
			grid[y].left=i;
			grid[y].right=i+1;
			if(i==0)
			{
				if(j==0)
				{
					NP nr(y+1,0);			
					S[y].NPList.push_back(nr);
					w=L-height;				
					NP nb(y+width,w);			
					S[y].NPList.push_back(nb);
				}
				else if(j==height-1)
				{
					r=j*(width-1)+i;
					NP nr(y+1,r);			
					S[y].NPList.push_back(nr);
					u=(height+j-1)*width-height+i;
					NP nt(y-width,u);			
					S[y].NPList.push_back(nt);
				}
				else
				{
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
			}
			else if(i==width-1)
			{
				if(j==0)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
				else if(j==height-1)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
				}
				else
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
			}
			else
			{
				if(j==0)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
				else if(j==height-1)
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
				}
				else
				{
					NP nl(y-1,j*(width-1)+i-1);			
					S[y].NPList.push_back(nl);
					NP nr(y+1,j*(width-1)+i);			
					S[y].NPList.push_back(nr);
					NP nt(y-width,(height+j-1)*width-height+i);			
					S[y].NPList.push_back(nt);
					NP nb(y+width,(height+j)*width-height+i);			
					S[y].NPList.push_back(nb);
				}
			}			
	}
	//initiate A
	for(y=0;y<L;y++)
	{
		i=y%width;
		j=y/width;
		if(i==width-1)
		{
			if(j==height-1)
			{			
			}
			else
			{				
				w=(height+j)*width-height+i;
				A[w].r[0]=y;
				A[w].r[1]=y+width;
				A[w].bl=1;	
				
			}
		}
		else
		{
			if(j==height-1)
			{				
				r=j*(width-1)+i;
				A[r].r[0]=y;
				A[r].r[1]=y+1;
				A[r].bl=1;
			}
			else
			{
				r=j*(width-1)+i;
				A[r].r[0]=y;
				A[r].r[1]=y+1;
				A[r].bl=1;
				
				w=(height+j)*width-height+i;
				A[w].r[0]=y;
				A[w].r[1]=y+width;
				A[w].bl=1;	
			}			
		}		
	}
	y=2*width*height-width-height;
	InitializeASM(y,L,HC);
}
//input: S[comps],tag[comps]
//output:S[comps] with only loop regions which father regs bear the same label as index,
//tag[comps] new recording labels for each comp
int CHC::HierClust()
{
	sl::iterator iter;
	NPL::iterator lit,ltemp;
	int sernum=0;
	int ql,pl,temp;
	int x,y,i,u;//w,j
	double mini=0;
	NP np;
	_MC mc;
	while(comps>loop)
	{
		//start merge
//		int charlie=MCL.size();
		assert(MCL.size());
		iter=MCL.begin();
		sernum=iter->pInd;
		pl=A[sernum].r[0];
		ql=A[sernum].r[1];
		
//		assert(A[sernum].t>-1);//when using distbenz A.t is not sure to be positive

		if(pl>ql)
		{
			temp=pl;
			pl=ql;
			ql=temp;
		}
		S[pl].size+=S[ql].size;
		for(x=0;x<d_;++x)
		{
			S[pl].addition[x]+=S[ql].addition[x];
			S[pl].sSum[x]+=S[ql].sSum[x];
		}
		S[pl].perim+=S[ql].perim-2*A[sernum].bl;
		S[ql].p=pl;
	//	tag[ql]=pl;
		S[pl].norbox->UnionRect(S[ql].norbox,S[pl].norbox);
		S[pl].interdif+=(A[sernum].t+S[ql].interdif);
		//take care the neighbors affected by merge pl and ql;
		
		//take care neighbors of ql
		lit=S[ql].NPList.begin();
		while(lit!=S[ql].NPList.end())
		{
			if(lit->rInd==pl)
			{
				//to save time remove lit is not performed
				ltemp=lit;				
				//lit++;
				S[ql].NPList.erase(ltemp);
				continue;
			}
			if((ltemp=find(S[pl].NPList.begin(),S[pl].NPList.end(),*lit))!=S[pl].NPList.end())
			{
				A[ltemp->pInd].bl+=A[lit->pInd].bl;
//				A[ltemp->pInd].es+=A[lit->pInd].es;//this line for edge strength
				np.rInd=ql;
				S[lit->rInd].NPList.erase(remove(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np),S[lit->rInd].NPList.end());
			}
			else
			{
				if(A[lit->pInd].r[0]==ql)
					A[lit->pInd].r[0]=pl;
				else //if(A[lit->pInd].r[1]==ql)
					A[lit->pInd].r[1]=pl;
				//else
				//	assert(0);
				S[pl].NPList.push_back(*lit);
				np.rInd=ql;
				ltemp=find(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np);
				ltemp->rInd=pl;
				assert(ltemp->pInd==lit->pInd);
			}			
			lit++;
		}
		//take care neighbors of pl and MCL
		lit=S[pl].NPList.begin();
		while(lit!=S[pl].NPList.end())
		{
			if(lit->rInd==ql)
			{
				ltemp=lit;
				//++lit; this line works for list<NP> but not for vector<NP>
				S[pl].NPList.erase(ltemp);
				continue;
			}
			assert((A[lit->pInd].r[0]==pl&&A[lit->pInd].r[1]==lit->rInd)||(A[lit->pInd].r[0]==lit->rInd&&A[lit->pInd].r[1]==pl));
			DistBenz(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
//			JhHypo(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
			x=S[lit->rInd].bestp;
			if(A[x].r[0]==pl||A[x].r[1]==pl||A[x].r[0]==ql||A[x].r[1]==ql)
			{
				mini=1e20;
				for(ltemp=S[lit->rInd].NPList.begin();ltemp!=S[lit->rInd].NPList.end();++ltemp)
				{
					if(mini>A[ltemp->pInd].t)
					{
						mini=A[ltemp->pInd].t;
						S[lit->rInd].bestp=ltemp->pInd;
					}
					else if(mini==A[ltemp->pInd].t)
					{
						i=S[lit->rInd].bestp;
						//j=A[i].r[0]==lit->rInd?A[i].r[1]:A[i].r[0];
						u=ltemp->pInd;
						//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
						if(i>u)//(j>w)
							S[lit->rInd].bestp=u;//lit->pInd;
					}
				}
			}
			else
			{
				if(A[x].t>A[lit->pInd].t)					
				{
					//it's possible that the pair does not exist in merge candidate list
					//but it's safe to delete a _MC not exist!
					mc.pInd=x;
					mc.t=A[x].t;
					MCL.erase(mc);
					S[lit->rInd].bestp=lit->pInd;
				}
				else if(A[x].t==A[lit->pInd].t)
				{
					
					//j=A[x].r[0]==lit->rInd?A[x].r[1]:A[x].r[0];
					u=lit->pInd;
					//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
					if(x>u)//(j>w)
					{
						mc.pInd=x;
						mc.t=A[x].t;
						MCL.erase(mc);
						S[lit->rInd].bestp=u;//lit->pInd;
					}
				}
			}
			++lit;
		}
		//compute bestp for pl
		mini=1e20;
		for(ltemp=S[pl].NPList.begin();ltemp!=S[pl].NPList.end();ltemp++)
		{
			if(mini>A[ltemp->pInd].t)
			{
				mini=A[ltemp->pInd].t;
				S[pl].bestp=ltemp->pInd;
			}
			else if(mini==A[ltemp->pInd].t)
			{
				i=S[pl].bestp;
				//j=A[i].r[0]==pl?A[i].r[1]:A[i].r[0];
				u=ltemp->pInd;
				//w=A[u].r[0]==pl?A[u].r[1]:A[u].r[0];
				if(i>u)//(j>w)
					S[pl].bestp=u;//lit->pInd;
			}
		}
		//update MCL this procedure might be implemented more efficient but more tricky.
		//for all pl's neighbors AND PL decide the mutual best pairs
		x=S[pl].bestp;
		y=A[x].r[0]==pl?A[x].r[1]:A[x].r[0];
		if(S[y].bestp==x)
		{
			mc.pInd=x;
			mc.t=A[x].t;
			MCL.insert(mc);
		}
		lit=S[pl].NPList.begin();
		while(lit!=S[pl].NPList.end())
		{
			x=S[lit->rInd].bestp;
			y=A[x].r[0]==lit->rInd?A[x].r[1]:A[x].r[0];
			if(S[y].bestp==x)
			{
				mc.pInd=x;
				mc.t=A[x].t;
				MCL.insert(mc);
			}			
			lit++;
		}
		//test
	/*		ofstream output("C://reg9pixel.txt",ios::app);
	if(!output){
		AfxMessageBox("Can't open reg.txt!");
		return 1;
	}
	output<<"label\t"<<"size\t"<<"perim\t"<<"bestp\t"<<"interdif\t"<<"neigh(rInd,pInd)\t\t\t\t"<<"bl\t"<<endl;
	for(x=0;x<Height*Width;x++)
		S[x].Print("C://reg9pixel.txt");
			ofstream output2("C://pairs.txt",ios::app);
	output2<<"reg1\t"<<"reg2\t"<<"bl\t"<<"t\t"<<endl;
	for(x=0;x<(2*Width*Height-(Width+Height));x++)
	{
		output2<<A[x].r[0]<<"	"<<A[x].r[1]<<"	"<<A[x].bl<<"	"<<A[x].t<<endl;
	}*/
	//test
		MCL.erase(iter);
		--comps;
	}

	return 0;
}
//build quad tree for arbitrary size image
void CHC::BQT(int s, QT *parent)
{
	if(s==1)
	{
		++comps;
		return;
	}
	int cx=parent->ulx;
	int cy=parent->uly;
	int m=parent->lm;
	int n=parent->un;

	if(Predicate(m,n,cx,cy))
	{
		++comps;
		return;//predicate
	}
	int shift=0,temp=0;
	int pi=parent->index;
	if(pi==1)
	{
		shift=(int)ceil(log((double)m)/log(2.0));
		temp=(int)ceil(log((double)n)/log(2.0));
		shift=shift>temp?shift:temp;
		s=1<<shift;
	}
	int hs=s>>1;//half s
	int i=0;
	if(hs>=m&&hs<n)
	{
		QT*nw=new QT;
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=m;
		nw->un=hs;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		QT*ne=new QT;		
		ne->index=pi*4;
		ne->ulx=cx+hs;
		ne->uly=cy;
		ne->lm=m;
		ne->un=n-hs;
		ne->parent=parent;
		for(i=0;i<4;i++)
		ne->child[i]=NULL;
		parent->child[0]=ne;
		BQT(hs,ne);
		return;
	}
	else if(hs<m&&hs>=n)
	{
		QT*nw=new QT;
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=hs;
		nw->un=n;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		QT*sw=new QT;
		sw->index=pi*4+2;
		sw->ulx=cx;
		sw->uly=cy+hs;
		sw->lm=m-hs;
		sw->un=n;
		sw->parent=parent;
		for(i=0;i<4;i++)
		sw->child[i]=NULL;
		parent->child[2]=sw;
		BQT(hs,sw);
		return;
	}
	else if(hs>=m&&hs>=n)
	{
		QT*nw=new QT;
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=m;
		nw->un=n;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		return;
	}
	else
	{
		QT*nw=new QT[4];
		nw->index=pi*4+1;
		nw->ulx=cx;
		nw->uly=cy;
		nw->lm=hs;
		nw->un=hs;
		nw->parent=parent;
		for(i=0;i<4;i++)
		nw->child[i]=NULL;
		parent->child[1]=nw;
		BQT(hs,nw);
		QT*sw=new QT;
		sw->index=pi*4+2;
		sw->ulx=cx;
		sw->uly=cy+hs;
		sw->lm=m-hs;
		sw->un=hs;
		sw->parent=parent;
		for(i=0;i<4;i++)
		sw->child[i]=NULL;
		parent->child[2]=sw;
		BQT(hs,sw);
		QT*se=new QT;
		se->index=pi*4+3;
		se->ulx=cx+hs;
		se->uly=cy+hs;
		se->lm=m-hs;
		se->un=n-hs;
		se->parent=parent;
		for(i=0;i<4;i++)
		se->child[i]=NULL;
		parent->child[3]=se;
		BQT(hs,se);
		QT*ne=new QT;
		ne->index=pi*4;
		ne->ulx=cx+hs;
		ne->uly=cy;
		ne->lm=hs;
		ne->un=n-hs;
		ne->parent=parent;
		for(i=0;i<4;i++)
		ne->child[i]=NULL;
		parent->child[0]=ne;
		BQT(hs,ne);
		return;
	}
}
//if sum of max delta is less than episilon, return true
//input: valid corners if only two, then ll and lr are -1, if four, name denotes the locations.
//szi denotes the size of four regions lying in the four quadrants
bool CHC::Predicate(int ul,int sz1,int ur,int sz2,int ll,int sz3,int lr,int sz4)
{
	int d=0;//counter
	float res=0;
	if(ll==-1)
	{
		ul*=d_;
		ur*=d_;
		
		float* avr=new float[2];
		
		for(d=0;d<d_;d++)
		{
			avr[0]=(float)data_[ul+d]/sz1;
			avr[1]=(float)data_[ur+d]/sz2;
			res+=2*abs(avr[1]-avr[0]);
		}
		delete []avr;
		return res<=maxDelta/Range;
	}
	ul*=d_;
	ur*=d_;
	ll*=d_;
	lr*=d_;
	
	float* avr=new float[4];
	for(d=0;d<d_;d++)
	{
		avr[0]=(float)data_[ul+d]/sz1;
		avr[1]=(float)data_[ur+d]/sz2;
		avr[2]=(float)data_[ll+d]/sz3;
		avr[3]=(float)data_[lr+d]/sz4;
		
		sort(avr,avr+4);
		res+=avr[3]+avr[2]-avr[1]-avr[0];
	}
	delete []avr;
	return res<=maxDelta/Range;
}

void CHC::ShowQT(QT*r)
{
	if(r==NULL)return;
	r->Print();
	int i=0;
	for(i=0;i<4;i++)
	{
		if(r->child[i])
		ShowQT(r->child[i]);
	}
}

void CHC::ElimTree(QT *r)
{	
	for(int i=0;i<4;i++)
	{
		if(r->child[i])
			ElimTree(r->child[i]);
	}
	delete r;
}
//input: original image 
//note now this function is paralysed CRect grid[] should be refilled when time allowed!
//output:quadtree segmented image updated comps,data_, sSum_,A initialized
// S and tag with size L for each pixel,A start with 0,tag array stores the father pixel index
void CHC::QTMerge()
{
	int dim=d_;
	int h=Height,w=Width;
	int L=h*w;
	comps=L;
	int x,y,d;
	int sernum=0;
	S=vector<Region>(L);
	A=vector<tPair>(2*L-w-h);
	grid=new CRect[L];//store boundary information for each primitive region
	memset(tag,-1,L*sizeof(int));
	int i=0,j=0,sz=0;
	int rec=0;//current number of segments
	int step=0,hs=0,hs2=0;//edge length of blocks checked in each loop and half step
	
	int flag=1,temp=0;
	int limx,limy;
	int ul,ur,ll,lr;//four upper left corner point index in four quadrant
	static int proxy=0;
	//quadtree merge main section and Regions initiation
	while(flag>0)//some blocks have not yet been assigned to region
	{
		flag=0;
		++temp;
		step=1<<temp;
		hs=step>>1;
		hs2=hs*hs;
		limy=(int)ceil((double)h/step)*step;
		limx=(int)ceil((double)w/step)*step;
		for(y=0;y<limy-step;y+=step)
		{
			for(x=0;x<limx-step;x+=step)
			{
				//	CheckBlock(x,y,step);
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs2,lr,hs2))
				{
					//predicate is met
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled or not pure
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
						S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=grid+ul;
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
						S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=x+step;
						S[ur].norbox=grid+ur;
						S[ur].size=hs2;
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
						S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*step;
						grid[ll].top=y+hs;
						grid[ll].bottom=y+step;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=grid+ll;
						S[ll].size=hs2;
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
						S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*step;
						grid[lr].top=y+hs;
						grid[lr].bottom=y+step;
						grid[lr].left=x+hs;
						grid[lr].right=x+step;
						S[lr].norbox=grid+lr;
						S[lr].size=hs2;
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
			}
		}
		assert(y==limy-step);
		//for blocks lying on the boundary first check the downside boundary and use y 
		//with the same value gotten from above
		for(x=0;x<limx-step;x+=step)
		{
			//if only two component exist
			if(y+hs>=h)
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,(h-y)*hs,ur,(h-y)*hs))
				{
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*(h-y)+step;
						grid[ul].top=y;
						grid[ul].bottom=h;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=grid+ul;
						S[ul].size=(h-y)*hs;
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(h-y)+step;
						grid[ur].top=y;
						grid[ur].bottom=h;
						grid[ur].left=x+hs;
						grid[ur].right=x+step;
						S[ur].norbox=grid+ur;
						S[ur].size=hs*(h-y);
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs*(h-hs-y),lr,hs*(h-hs-y)))
				{
					//predicate is met
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				else//this block contains some component has upper left corner pixel labelled 
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=grid+ul;
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=x+step;
						S[ur].norbox=grid+ur;
						S[ur].size=hs2;
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(h-y-hs)+step;
						grid[ll].top=y+hs;
						grid[ll].bottom=h;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=grid+ll;
						S[ll].size=hs*(h-y-hs);
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
							S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*(h-y-hs)+step;
						grid[lr].top=y+hs;
						grid[lr].bottom=h;
						grid[lr].left=x+hs;
						grid[lr].right=x+step;
						S[lr].norbox=grid+lr;
						S[lr].size=hs*(h-y-hs);
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}//end else four block are assigned region label
			}//end else for y+hs<h
		}//end for each block lying on the bottom from left to right
		assert(x==limx-step);
		//check blocks lying to right side of the boundary and the bottom one is not included just as above
		for(y=0;y<limy-step;y+=step)
		{
			if(x+hs>=w)
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,hs*(w-x)))
				{					
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel or heterogeneous
				else
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=step+2*(w-x);
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=w;
						S[ul].norbox=grid+ul;
						S[ul].size=hs*(w-x);
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(w-x)+step;
						grid[ll].top=y+hs;
						grid[ll].bottom=y+step;
						grid[ll].left=x;
						grid[ll].right=w;
						S[ll].norbox=grid+ll;
						S[ll].size=hs*(w-x);
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-hs-x),ll,hs2,lr,hs*(w-hs-x)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
					
					
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=grid+ul;
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(w-x-hs)+step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=w;
						S[ur].norbox=grid+ur;
						S[ur].size=hs*(w-x-hs);
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*step;
						grid[ll].top=y+hs;
						grid[ll].bottom=y+step;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=grid+ll;
						S[ll].size=hs2;
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
							S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*(w-x-hs)+step;
						grid[lr].top=y+hs;
						grid[lr].bottom=y+step;
						grid[lr].left=x+hs;
						grid[lr].right=w;
						S[lr].norbox=grid+lr;
						S[lr].size=hs*(w-x-hs);
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			
		}
		assert(x==limx-step);
		assert(y==limy-step);
		if(x+hs>=w)
		{
			if(y+hs<h)//two blocks are considered
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,(w-x)*(h-hs-y)))
				{
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=step+2*(w-x);
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=w;
						S[ul].norbox=grid+ul;
						S[ul].size=hs*(w-x);
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(w-x)+2*(h-y-hs);
						grid[ll].top=y+hs;
						grid[ll].bottom=h;
						grid[ll].left=x;
						grid[ll].right=w;
						S[ll].norbox=grid+ll;
						S[ll].size=(h-y-hs)*(w-x);
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else//only one component exists this block must have been check before by predicate
			{//no addition and square sum is needed as both are computed already
				ul=y*w+x;
				if(tag[ul]==-1)
				{
					++flag;
				}
			}
		}		
		else//if x+hs<w
		{
			if(y+hs<h)//four blocks case
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-x-hs),ll,(h-y-hs)*(hs),lr,(h-y-hs)*(w-x-hs)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*step;
						grid[ul].top=y;
						grid[ul].bottom=y+hs;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=grid+ul;
						S[ul].size=hs2;
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=2*(w-x-hs)+step;
						grid[ur].top=y;
						grid[ur].bottom=y+hs;
						grid[ur].left=x+hs;
						grid[ur].right=w;
						S[ur].norbox=grid+ur;
						S[ur].size=hs*(w-x-hs);
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						S[ll].addition=data_+ll*dim;
							S[ll].sSum=sData_+ll*dim;
						S[ll].perim=2*(h-y-hs)+step;
						grid[ll].top=y+hs;
						grid[ll].bottom=h;
						grid[ll].left=x;
						grid[ll].right=x+hs;
						S[ll].norbox=grid+ll;
						S[ll].size=hs*(h-y-hs);
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						S[lr].addition=data_+lr*dim;
							S[lr].sSum=sData_+lr*dim;
						S[lr].perim=2*(w-x-hs)+2*(h-y-hs);
						grid[lr].top=y+hs;
						grid[lr].bottom=h;
						grid[lr].left=x+hs;
						grid[lr].right=w;
						S[lr].norbox=grid+lr;
						S[lr].size=(h-y-hs)*(w-x-hs);
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			else//y+hs>h
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,hs*(h-y),ur,(h-y)*(w-x-hs)))
				{
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				else	//if two blocks contains labeled pixel
				{
					if(tag[ul]==-1)
					{
						S[ul].addition=data_+ul*dim;
							S[ul].sSum=sData_+ul*dim;
						S[ul].perim=2*(h-y)+step;
						grid[ul].top=y;
						grid[ul].bottom=h;
						grid[ul].left=x;
						grid[ul].right=x+hs;
						S[ul].norbox=grid+ul;
						S[ul].size=hs*(h-y);
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						S[ur].addition=data_+ur*dim;
							S[ur].sSum=sData_+ur*dim;
						S[ur].perim=(w-x-hs)*(h-y);
						grid[ur].top=y;
						grid[ur].bottom=h;
						grid[ur].left=x+hs;
						grid[ur].right=w;
						S[ur].norbox=grid+ur;
						S[ur].size=(w-x-hs)*(h-y);
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
				
			}
		}
	}//while loop ended
	
	comps=rec;
	//test 
	/*ofstream bench("mark.txt",ios::app);
	bench<<"region total  number:"<<rec<<endl;
	bench<<"index\tfather\t\tsize\t\tperim"<<endl;
	for(x=0;x<L;x++)
	{
	bench<<x<<"\t"<<tag[x]<<"\t"<<S[tag[x]].size<<"\t"<<S[tag[x]].perim<<endl;
	}*/
	//test ended
	//initiate S's NL and interdif
	proxy=0;
//	double ret=0;
	for(x=0;x<L;++x)
	{

		assert(tag[x]!=-1);
		S[x].p=tag[x];
		if(tag[x]!=x)//belongs to a father and not a region
			continue;
		S[x].InterDiff(d_,wc,wp);
		//initiate NPList
		int lrx=grid[x].right,lry=grid[x].bottom ,ulx=grid[x].left ,uly=grid[x].top;
	//	assert(lrx<=w&&lry<=h);	
		if(lrx==w||lry==h)
		{
			if(lrx==w&&lry!=h)
			{
				NamNeiXEx(lry,ulx,lrx-ulx,x,proxy);
			}
			else if(lrx!=w&&lry==h)
			{	
				NamNeiYEx(lrx,uly,lry-uly,x,proxy);
			}
			else//lrx==w&&lry==h
				continue;
		}
		else
		{
			NamNeiY(lrx,uly,lry-uly,x,proxy);
			NamNeiX(lry,ulx,lrx-ulx,x,proxy);
		}		
	}

	//test
/*	ofstream lever("regtag.txt",ios::app);
	lever<<"region tags are below:\n";
	for(x=0;x<h;++x)
	{
		for(y=0;y<w;++y)
			lever<<"\t"<<tag[x*w+y];
	lever<<endl;
	}
	lever<<endl;
/*	lever<<"region total  number:"<<rec<<endl;
	lever<<"index\tsize\tperim\t\tneighbor list:(rind,pind)"<<endl;
	for(x=0;x<L;x++)
	{
		if(tag[x]!=x)
			continue;
		int chrt=-1;
		lever<<x<<"\t"<<S[x].size<<"\t"<<S[x].perim<<"\t\t";
		for(NPL::iterator dido=S[x].NPList.begin();dido!=S[x].NPList.end();++dido)
		{
				if(chrt==dido->rInd)
				{afxDump<<"region no.\t"<<x<<"\n";
				AfxMessageBox("duplicate neighbor in regions neighbor list!");}
				chrt=dido->rInd;
			//	NP temp(x,0);
			//	NPL::iterator	nplp=find(S[chrt].NPList.begin(),S[chrt].NPList.end(),temp);
			//	assert(nplp!=S[chrt].NPList.end());
			lever<<"("<<dido->rInd<<","<<dido->pInd<<")\t";
		}
		//	for(i=0;i<dim;i++)
		lever<<endl;
	}*/

	//	assert(sernum==h*w-1);
	//delete []grid;
	InitializeASM(proxy,L,QTS);
}
//A's size ,comps current,flag=0 for hc 1 for qt,2 for gbs
//function: initate A.es,A.t,S.bestp,MCL
int CHC::InitializeASM(int ln,int com, enum Appro flag)
{
	int x,y,label,cand,sernum=0;
	int h=Height,w=Width;
	int L=w*h;
	NPL::iterator lit;//,nplp;
	double mini=0;

	//initiate A.es one whole raster scan is performed. check the right and down pixel
	/*if(EM)
	{
		for(y=0;y<h-1;++y)
		{
			sernum=y*w;
			for(x=0;x<w-1;++x)
			{
				label=tag[sernum];
				cand=tag[sernum+1];
				if(label!=cand)
				{
					NP temp(cand,0);
					nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
				//	assert(nplp!=S[label].NPList.end()); // not sure about why sometimes nplp==end while still valid so changed to next line
					assert(nplp->rInd==cand);
					A[nplp->pInd].es+=(EM[sernum]+EM[sernum+1]);
				}
				cand=tag[sernum+w];
				if(label!=cand)
				{
					NP temp(cand,0);
					nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
					assert(nplp!=S[label].NPList.end());
					A[nplp->pInd].es+=(EM[sernum]+EM[sernum+w]);
				}
				++sernum;
			}
			label=tag[sernum];
				cand=tag[sernum+w];
			if(label!=cand)
			{
				NP temp(cand,0);
				nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
				assert(nplp!=S[label].NPList.end());
				A[nplp->pInd].es+=(EM[sernum]+EM[sernum+w]);
			}
			++sernum;
		}
		for(x=0;x<w-1;++x)
		{
			label=tag[sernum];
				cand=tag[sernum+1];
			if(label!=cand)
			{
				NP temp(cand,0);
				nplp=find(S[label].NPList.begin(),S[label].NPList.end(),temp);
				assert(nplp!=S[label].NPList.end());
				A[nplp->pInd].es+=(EM[sernum]+EM[sernum+1]);
			}
			++sernum;
		}
		//	assert(sernum==h*w-1);
	}
	else phi=1;*/
	if(flag==GBS)
		x=1;
	else
		x=0;
	for(;x<ln;x++)
	{
		label=A[x].r[0];
		cand=A[x].r[1];
		DistBenz(S[label],S[cand],A[x].bl,A[x].t);
//		JhHypo(S[label],S[cand],A[x].bl,A[x].t);
	}
	switch (flag)
	{//for S is continuous with regards to index
	case GBS:
	case HC:
		{
			//initiate S[x].bestp
			for(x=0;x<com;x++)
			{
				//find the best merge candidate for each region bestp for S[x]
				lit=S[x].NPList.begin();
				mini=1e20;
				while(lit!=S[x].NPList.end())
				{
					assert(lit->pInd>=0);
					if(mini>A[lit->pInd].t)
					{
						mini=A[lit->pInd].t;
						S[x].bestp=lit->pInd;//record the best pair for this region
					}
					else if(mini==A[lit->pInd].t)
					{
						label=S[x].bestp;
						//j=A[i].r[0]==x?A[i].r[1]:A[i].r[0];
						cand=lit->pInd;
						//w=A[u].r[0]==x?A[u].r[1]:A[u].r[0];
						if(label>cand)//(j>w)
							S[x].bestp=cand;//lit->pInd;
					}
					++lit;
				}
			}
			//initiate MCL
			//if the friendship is mutual, store this pair in MCL
			for(y=0;y<com;y++)
			{
				label=S[y].bestp;//pair index for this pair
				cand=A[label].r[0]==y?A[label].r[1]:A[label].r[0];//region index of the other region in the pair
				if(S[cand].bestp==label)
				{
					_MC mc(label,A[label].t);
					MCL.insert(mc);//ambiguity is avoided as the key is unique
				}
			}
		}
		break;
		
	case QTS:
		{	
			//initiate S[x].bestp
			for(x=0;x<L;x++)
			{
				//find the best merge candidate for each region bestp for S[x]
				if(tag[x]!=x)//belongs to a father and not a region
					continue;
				lit=S[x].NPList.begin();
				mini=1e20;
				while(lit!=S[x].NPList.end())
				{
					if(mini>A[lit->pInd].t)
					{
						mini=A[lit->pInd].t;
						S[x].bestp=lit->pInd;//record the best pair for this region
					}
					else if(mini==A[lit->pInd].t)
					{
						label=S[x].bestp;
						//j=A[i].r[0]==x?A[i].r[1]:A[i].r[0];
						cand=lit->pInd;
						//w=A[u].r[0]==x?A[u].r[1]:A[u].r[0];
						if(label>cand)//(j>w)
							S[x].bestp=cand;//lit->pInd;
					}
					++lit;
				}
			}
			//initiate MCL
			//if the friendship is mutual, store this pair in MCL
			for(y=0;y<L;y++)
			{
				if(tag[y]!=y)//belongs to a father and not a region
					continue;
				label=S[y].bestp;//pair index for this pair
				cand=A[label].r[0]==y?A[label].r[1]:A[label].r[0];//region index of the other region in the pair
				if(S[cand].bestp==label)
				{
					_MC mc(label,A[label].t);
					MCL.insert(mc);//ambiguity is avoided as the key is unique
				}
			}
		}
		break;
		
	default:
		AfxMessageBox("Error with initiate ASM!");
		break;
	}

	return flag;
}

//if origin data format is byte, regularization i.e. division by 256 is used to keep precise
void CHC::BuildData(GDALDataset* m_pDataset)
{
	int spp=m_pDataset->GetRasterCount();//波段的数目
	int d;
	if(bWArray.size()>0)
	{
		assert(bWArray.size()==spp);
		for(d=0,d_=0;d<spp;++d)
			d_+=(bWArray[d]>0?1:0);
	}
	else
	{
		for(d=0;d<spp;++d)
			bWArray.push_back(1);
		d_=spp;
	}
	
	Width=m_pDataset->GetRasterXSize();
	Height=m_pDataset->GetRasterYSize();
	int L=Height*Width;
	comps=L;
	if(tag)
		delete[]tag;
	tag=new int[L];	
	if (data_)
	{
		delete []data_;
		delete []sData_;
	}	
	data_=new float[L*d_];
	sData_=new float[L*d_];
	if(A.size())
	{
		A.clear();
		S.clear();
	}
	int x,y,sernum,temp,cur=0;//cur for current band index
	int nByteWidth=d_*Width;
	float* buf =new float[Width*Height];
	GDALRasterBand  *m_pBand=NULL;
	for(d=0;d<spp;++d)
	{
		if(bWArray[d]==0)
			continue;
		++cur;
		m_pBand= m_pDataset->GetRasterBand(cur);
		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, Width, Height, buf, Width,Height, GDT_Float32, 0, 0 ))
			{
				sernum=cur-1;
				temp=0;
				for(y=0;y<Height;++y)
				{
					for (x = 0; x < Width; ++x) 
					{
						data_[sernum]=buf[temp]/Range;							
						sData_[sernum]=data_[sernum]*data_[sernum];
						sernum+=d_;
						++temp;
					}				
				}
			}
		}
	}
	delete[]buf;

}

void CHC::EdgeMag(int w,int h)
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
void CHC::LamSchd(Region*a,Region*b,unsigned int len,double &ret)
{
	ret=0;
	//compute sigma first
	for(int i=0;i<d_;i++)
	{//here for the quotient of addition sum the fractional part is discarded for small difference
		ret+=square((a->addition[i])/(double)(a->size)-(b->addition[i])/(double)(b->size));
	}
	ret*=(a->size)*(b->size);
	ret/=(a->size+b->size);
	ret/=len;
}
//function: save edge raster file into bmp file 8bit


BYTE* CHC::EdgeSusan(int w,int h,int BT,bool Trixtri)
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
//the class label of rect is required to start from 0 and
// its first object is stored in 0 location 
//THIS FUNCTION is intended for QT hc since minitag only applys to qt
double CHC::BenchMark(int *rect,int ind)
{
	int L=Height*Width,i=0,jude=0;
	assert(ind==L);
	i=MiniTag(tag,L);
	assert(i==comps);
	for(i=0;i<ind;++i)
		if(rect[i]>jude)
			jude=rect[i];
	double ei=0,ri=0,cr=0;
	ri=RandIndex(L,rect,jude+1,tag,comps,ei);
	cr=(ri-ei)/(1-ei);
	RI=ei;
	return ri;
}


//input: indarr is array of labels for each integer from 0 to len-1 
//number of different labels equals number of labels which equals the corresponding integer
//output:label is for checking whether labels number is safe.
int CHC::MiniTag(int*indarr,int len)
{
	//minimize all the tags derelict because elicting bugs
	//only work for qt-hc and conflict with EvalQs
	int label=0,sernum=0;
	for (sernum = 0; sernum< len; ++sernum) 
	{
		if(indarr[sernum]==sernum)
		{
			indarr[sernum]=label;
			++label;					
		}
	}
	for (sernum= 0; sernum< len;++sernum) 
	{
		indarr[sernum]=indarr[indarr[sernum]];
	}	
	return label;
}

//routine imitating multiresolution segmentation using dither matrix to distribute region merging
//and locally homogeneity lest growth
//it's not possible for graph based seg to use pixel order, so in this implementation, only segment
//order is realised. L means de facto size of S required by graph based seg.
//tagBe stores the visit order for regions in S produced by Bayer dithering, loop maximum iterations
void CHC::MRS(int L)
{
	int i=0,j=0;int cycle,indk=0,supl=0,comps0=comps;
	float curdiff=hdiff;
	if (L==0)
		L=Height*Width;
	int l=L;
	int *tagBe=new int[L],*tagp=tagBe;
	indk=1;
	while(l>3)
	{
		l>>=2;
		++indk;
	}
	supl=1<<(2*indk);

	for(i=0;i<supl;++i)
	{
		*tagp=DitherGen(L,indk,i);
		++tagp;
	}

//	bool pixelOrder=0;
	for(cycle=0;cycle<loop;++cycle)
	{		
		curdiff=hdiff/(1<<(loop-cycle-1));
		//begin loop

		//generate dither matrix elements storing in tagBe;
		sl::iterator iter;
		NPL::iterator lit,ltemp;
		int ql,pl,temp,sernum;
		int x,u,v;//w,j
		double mini=0;
		NP np;
		do{
			comps0=comps;
			for(i=0;i<L;++i)
			{
				//assert(tagBe[i]>=0);//checked before 
				pl=tagBe[i];
				
				if(pl!=S[pl].p)//not a segment
					continue;
				sernum=S[pl].bestp;
				ql=A[sernum].r[0]==pl?A[sernum].r[1]:A[sernum].r[0];
				float a=A[S[ql].bestp].t,b=A[sernum].t;
				j=0;
				while((j<4)&&((a<b)||(b>curdiff)))				
				{
					++j;
					//	continue;
					pl=ql;
					sernum=S[ql].bestp;
					ql=A[sernum].r[0]==pl?A[sernum].r[1]:A[sernum].r[0];
					b=a;
					a=A[S[ql].bestp].t;
				}
				if(j==4)continue;
				//merge the two regions
				--comps;
				if(pl>ql)
				{
					temp=pl;
					pl=ql;
					ql=temp;
				}
				S[pl].size+=S[ql].size;
				for(x=0;x<d_;++x)
				{
					S[pl].addition[x]+=S[ql].addition[x];
					S[pl].sSum[x]+=S[ql].sSum[x];
				}
				S[pl].perim+=S[ql].perim-2*A[sernum].bl;
				S[pl].norbox->UnionRect(S[ql].norbox,S[pl].norbox);
				S[ql].p=pl;
				S[pl].interdif+=(A[sernum].t+S[ql].interdif);
				//take care the neighbors affected by merge pl and ql;
				
				//take care neighbors of ql
				lit=S[ql].NPList.begin();
				while(lit!=S[ql].NPList.end())
				{
					if(lit->rInd==pl)
					{
						
						ltemp=lit;				
						//lit++;
						S[ql].NPList.erase(ltemp);
						continue;
					}
					if((ltemp=find(S[pl].NPList.begin(),S[pl].NPList.end(),*lit))!=S[pl].NPList.end())
					{
						A[ltemp->pInd].bl+=A[lit->pInd].bl;
//						A[ltemp->pInd].es+=A[lit->pInd].es;//this line for edge strength
						np.rInd=ql;
						S[lit->rInd].NPList.erase(remove(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np),S[lit->rInd].NPList.end());
					}
					else
					{
						if(A[lit->pInd].r[0]==ql)
							A[lit->pInd].r[0]=pl;
						else //if(A[lit->pInd].r[1]==ql)
							A[lit->pInd].r[1]=pl;
						
						S[pl].NPList.push_back(*lit);
						np.rInd=ql;
						ltemp=find(S[lit->rInd].NPList.begin(),S[lit->rInd].NPList.end(),np);
						ltemp->rInd=pl;
						assert(ltemp->pInd==lit->pInd);
					}			
					lit++;
				}
				//take care neighbors of pl and MCL
				lit=S[pl].NPList.begin();
				while(lit!=S[pl].NPList.end())
				{
					if(lit->rInd==ql)
					{
						ltemp=lit;
						//++lit; this line works for list<NP> but not for vector<NP>
						S[pl].NPList.erase(ltemp);
						continue;
					}
					assert((A[lit->pInd].r[0]==pl&&A[lit->pInd].r[1]==lit->rInd)||(A[lit->pInd].r[0]==lit->rInd&&A[lit->pInd].r[1]==pl));
					DistBenz(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
//					JhHypo(S[pl],S[lit->rInd],A[lit->pInd].bl,A[lit->pInd].t);
					x=S[lit->rInd].bestp;
					if(A[x].r[0]==pl||A[x].r[1]==pl||A[x].r[0]==ql||A[x].r[1]==ql)
					{
						mini=1e20;
						for(ltemp=S[lit->rInd].NPList.begin();ltemp!=S[lit->rInd].NPList.end();++ltemp)
						{
							if(mini>A[ltemp->pInd].t)
							{
								mini=A[ltemp->pInd].t;
								S[lit->rInd].bestp=ltemp->pInd;
							}
							else if(mini==A[ltemp->pInd].t)
							{
								v=S[lit->rInd].bestp;
								//j=A[v].r[0]==lit->rInd?A[v].r[1]:A[v].r[0];
								u=ltemp->pInd;
								//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
								if(v>u)//(j>w)
									S[lit->rInd].bestp=u;//lit->pInd;
							}
						}
					}
					else
					{
						if(A[x].t>A[lit->pInd].t)					
						{
							//it's possible that the pair does not exist in merge candidate list
							//but it's safe to delete a _MC not exist!
							
							S[lit->rInd].bestp=lit->pInd;
						}
						else if(A[x].t==A[lit->pInd].t)
						{
							
							//j=A[x].r[0]==lit->rInd?A[x].r[1]:A[x].r[0];
							u=lit->pInd;
							//w=A[u].r[0]==lit->rInd?A[u].r[1]:A[u].r[0];
							if(x>u)//(j>w)
							{
								
								S[lit->rInd].bestp=u;//lit->pInd;
							}
						}
					}
					++lit;
				}
				//compute bestp for pl
				mini=1e20;
				for(ltemp=S[pl].NPList.begin();ltemp!=S[pl].NPList.end();ltemp++)
				{
					if(mini>A[ltemp->pInd].t)
					{
						mini=A[ltemp->pInd].t;
						S[pl].bestp=ltemp->pInd;
					}
					else if(mini==A[ltemp->pInd].t)
					{
						v=S[pl].bestp;
						//j=A[v].r[0]==pl?A[v].r[1]:A[v].r[0];
						u=ltemp->pInd;
						//w=A[u].r[0]==pl?A[u].r[1]:A[u].r[0];
						if(v>u)//(j>w)
							S[pl].bestp=u;//lit->pInd;
					}
				}
				
				
		}//each segment
		}while(comps<comps0);

	}//each cycle
}


//for each region in S, if size no greater than nsize, 
//merged to the nearest color neighboring region
void CHC::KillMinion(UINT nsize)
{
	
}
//use three strategies to refine borders, ben wuest's border refinement, NIU xutong's region grow 
//frayed boundary removal, and susan edge map confidence decision
void CHC::BorderRefine()
{

}
// the function draws all the squares in the image
void drawSquares( CvSize sz, CvSeq* squares,int*accum,int len )
{
	IplImage* img = cvCreateImage( sz, 8, 3 );
	cvZero(img);
     int i=0,j=0,k=0;
    
 
    //int totum=squares->total;
 
    // read 4 sequence elements at a time (all vertices of a square)
    while(j<len)// <totum)
    {        
        int count = accum[j];
		CvPoint pt0= *CV_GET_SEQ_ELEM(CvPoint, squares, k+count-1);
		for( i = 0; i <count; i++ )
        {
            CvPoint pt =*CV_GET_SEQ_ELEM( CvPoint,squares,k+i) ;
            cvLine( img, pt0, pt, CV_RGB( 0, 255, 0 ), 1, CV_AA, 0 );
            pt0 = pt;
        }
		k+=count;    
		++j;		
    }
     // show the resultant image
    cvShowImage( "result", img );
    cvReleaseImage( &img );
}
//retrieve points on the boundary of an object with random sequence
CvSeq* GetBoundary(int *tag, int Width, int Height,CvMemStorage* storage)
{	
	CvSeq* ptseq;
	CvSeq*hull,*cur;
	CvPoint pt0;
	vector<CvPoint> stk;
	int k,n[9],label,sernum;
	int i,j,s,t,immed,curtag;
	int i1=0,j1=0,cont=0;
	int *g=new int[Width*Height];
	sernum=0;
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{
			label=tag[sernum];
			if(j>0&&i>0&&(j<Width-1)&&(i<Height-1))
			{
				if((label==tag[sernum+1])&&(label==tag[sernum-1])&&(label==tag[sernum-Width])&&(label==tag[sernum+Width]))
				{
					g[sernum]=-1;
				}
				else
					g[sernum]=tag[sernum];
			}
			else
					g[sernum]=tag[sernum];
			++sernum;
		}
	}
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			
			immed=i*Width;
			if(g[immed+j]==-1)continue;
			stk.clear();
			pt0.x=j;
			pt0.y=i;
			stk.push_back(pt0);
			curtag=g[immed+j];
			g[immed+j]=-1;
			
			hull=cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
				sizeof(CvPoint), storage);
			//create simple polygon curve represented by points array
			//cvCreateSeq( CV_SEQ_ELTYPE_POINT|CV_SEQ_KIND_CURVE|CV_SEQ_FLAG_SIMPLE, sizeof(CvContour),
              //                       sizeof(CvPoint), storage );
			if(cont==0)
			{
				ptseq=hull;
				cur=hull;
			}
			else
			{		
				cur->h_next=hull;
				cur=hull;
			}
			++cont;
			cvSeqPush( cur, &pt0 );
		//	printf("%d,%d\t",j,i);
			
			while(!stk.empty())
			{
				s=stk.rbegin()->y;
				t=stk.rbegin()->x;
				i1=s;j1=t;
				
				stk.pop_back();
				GetMask(t,s,g,Width,Height,n);
				for(k=1;k<=8;++k)
				{
					
					if(n[k]==curtag)
					{
						if(k==8)k=0;
						switch(k)
						{
						case 1:s--;t++;break;
						case 2:s--;break;
						case 3:s--;t--;break;
						case 4:t--;break;
						case 5:s++;t--;break;
						case 6:s++;break;
						case 7:s++;t++;break;
						case 0:t++;k=8;break;
						}
						pt0.x=t;
						pt0.y=s;
						stk.push_back(pt0);
						g[s*Width+t]=-1;
						cvSeqPush( cur, &pt0 );
					//	printf("%d,%d\t",t,s);
						s=i1;t=j1;
					}
					
				}
			}
		//	printf("\n");
		
		}
	}
	delete []g;
	return ptseq;
}
//compute  the region features, NDVI compactness length/width(elongation) roundness 
void CHC::RegionProps()
{
	if(!tag)return;
	CvMemStorage* storage= cvCreateMemStorage(0);
	CvSeq*contours;
	CvPoint pt;
	CvBox2D rect;
	int sernum,hullcount=0;
	float area=0,temp1,temp2;
//	float i;
	int label;
	contours=GetBoundary(tag,Width,Height,storage);
	while( contours)
	{		
		rect=cvMinAreaRect2(contours);
//		cvBoxPoints(rect,ptf);
		pt =*CV_GET_SEQ_ELEM( CvPoint, contours, 0 );
		sernum=pt.y*Width+pt.x;
		label=tag[sernum];

		temp1=__max(rect.size.height,rect.size.width);
		temp2=__min(rect.size.height,rect.size.width);
	//	hull = cvConvexHull2(contours, 0, CV_CLOCKWISE, 1 );
	//	area=abs(cvContourArea(hull,CV_WHOLE_SEQ));
	
		if(S[label].attlist.size()==0)//attlist initiated more than once when a object is enclosed
		{
			S[label].attlist.push_back(temp1);
			S[label].attlist.push_back(temp2);
			++hullcount;
		}
		else
			assert(S[label].attlist[maxaxislen]>temp1);
	
		// take the next contour
		contours = contours->h_next;
	}
	assert(hullcount==comps);
	cvClearMemStorage( storage);

	//average width and length
	int len=S.size();
	int x,mW,mH,mx,my,i,j,pos,dest,total;
	BYTE*patch;
	float*pool;
	CvMat Ma, Mb;
	double min,sum=0;
	//search father for each region
	for (x = 0; x <len; x++) 
	{		
		if(x!=S[x].p)			
			continue;		
		mH=S[x].norbox->Height();
		mW=S[x].norbox->Width();
		patch=new BYTE[(mH+2)*(mW+2)];
		pool=new float[(mH+2)*(mW+2)];
		memset(patch,0,sizeof(BYTE)*(mH+2)*(mW+2));
		cvInitMatHeader( &Ma, mH+2, mW+2, CV_8UC1, patch);
		cvInitMatHeader( &Mb, mH+2, mW+2, CV_32FC1, pool);

		mx=S[x].norbox->left;
		my=S[x].norbox->top;
		pos=my*Width+mx;
		dest=(mW+2)+1;
		total=0;
		for(j=0;j<mH;++j)
		{			
			for(i=0;i<mW;++i)
			{
				if(tag[pos]==x)
				{
					patch[dest]=1;	
					++total;
				}
				++pos;
				++dest;
			}
			pos+=Width-mW;
			dest+=2;
		}
		cvDistTransform(&Ma,&Mb,CV_DIST_L2,5);
	//	sum=0;
		cvMinMaxLoc(&Mb,&min,&sum);
	/*	for(i=1;i<mH+1;++i)
		{
			cvGetRow(&Mb,&Mc,i);
			cvMinMaxLoc(&Mc,&min,&max);
			sum+=max;
		}
		sum=sum/mH*2;*/	
		sum*=2;
		assert(S[x].attlist.size()==2);
		S[x].attlist.push_back((float)sum);
	
		assert(total==S[x].size);
		S[x].attlist.push_back((float)S[x].size/sum);
	
		delete[]patch;
		delete[]pool;
	}
	if(!ndv)return;
	int L=Height*Width;
	for(i=0;i<len;++i)
	{
		if(S[i].p!=i)
			continue;
		assert(S[i].attlist.size()==4);
		S[i].attlist.push_back(0);
	}
	for(sernum=0;sernum<L;++sernum)
	{
		label=tag[sernum];		
		S[label].attlist[ndvindex]+=ndv[sernum];	
	}
	for(i=0;i<len;++i)
	{
		if(S[i].p!=i)
			continue;
		S[i].attlist[ndvindex]/=S[i].size;		
	}
}
//press ONLY f5 produces only 1 contour enveloping the whole image, press ctrl+f5 produces contours
//more than 1 except the frame contour,  debug error memory damage is often admonished, with no idea
//this funct   ion binarize array tag,find contours, polygonize and draw out!

void FindContours(int Width,int Height, int*tag,int comps)
{
	int r,c,sernum=0,label,i,j=0;
	CvSize sz = cvSize( Width, Height);
    IplImage* gray = cvCreateImage( sz, 8, 1 ); 

	const char* wndname = "result";
	for(r=0;r<Height;++r)
	{
		for(c=0;c<Width;++c)
		{
			label=tag[sernum];
			if(r>0&&c>0&&(c<Width-1)&&(r<Height-1))
			{
				if(label!=tag[sernum+1]||label!=tag[sernum-1]||label!=tag[sernum-Width]||label!=tag[sernum+Width])
				{
					((uchar*)(gray->imageData + gray->widthStep*r))[c]=255;//must be 255, 0 produces unimaginable thing
				}
			}
			else ((uchar*)(gray->imageData + gray->widthStep*r))[c]=255;
			++sernum;
		}
	}//image upside down
	cvNamedWindow( wndname, 1 );
//	cvShowImage( "result", gray );

	CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* contours,*result;
	CvSeq* squares = cvCreateSeq( 0, sizeof(CvSeq), sizeof(CvPoint), storage );
	int* accum = new int[comps*2];

	   // find contours and store them all as a list
	cvFindContours( gray, storage, &contours, sizeof(CvContour),
		   CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	   // test each contour
	while( contours )
	{		
		CvBox2D rect=cvMinAreaRect2(contours);
		
		if((rect.size.height/rect.size.width>2.f)||(rect.size.width/rect.size.height>2.f))
		{
			// approximate contour with accuracy proportional
			// to the contour perimeter
			result = cvApproxPoly( contours, sizeof(CvContour), storage,
				CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
			
			//write polygon vertices to resultant sequence 
			accum[j]=result->total;
			++j;
			for( i = 0; i < result->total; i++)
				cvSeqPush( squares,(CvPoint*)cvGetSeqElem( result, i ));
		}		 
		// take the next contour
		contours = contours->h_next;
	}
//	CString hun;hun.Format("contours:%d\n",j);
//	AfxMessageBox(hun);
	drawSquares(sz,squares,accum,j);
	cvReleaseImage( &gray );
	delete []accum;
	cvClearMemStorage( storage);
}
//j coordinate in width i in height, g tag array n storage for mask
void GetMask(int j, int i,int *g, int Width,int Height,int*n)
{
			int	s=i,t=j,immed=i*Width;
			if(i==0)
			{
				if(j==0)
				{
					n[3]=-1;n[2]=-1;n[1]=-1;
					n[4]=-1;n[8]=g[immed+t+1];
					n[5]=-1;n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
				else if(j==Width-1)
				{
					n[3]=-1;n[2]=-1;n[1]=-1;
					n[4]=g[immed+t-1];n[8]=-1;
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=-1;
				}
				else
				{
					n[3]=-1;n[2]=-1;n[1]=-1;
					n[4]=g[immed+t-1];n[8]=g[immed+t+1];
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
			}
			else if(i==Height-1)
			{
				if(j==0)
				{
					n[3]=-1;n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=-1;n[8]=g[immed+t+1];
					n[5]=-1;n[6]=-1;n[7]=-1;
				}
				else if(j==Width-1)
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=-1;
					n[4]=g[immed+t-1];n[8]=-1;
					n[5]=-1;n[6]=-1;n[7]=-1;			
				}
				else
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=g[immed+t-1];n[8]=g[immed+t+1];
					n[5]=-1;n[6]=-1;n[7]=-1;
				}
			}
			else
			{
				if(j==0)
				{
					n[3]=-1;n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=-1;n[8]=g[immed+t+1];
					n[5]=-1;n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
				else if(j==Width-1)
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=-1;
					n[4]=g[immed+t-1];n[8]=-1;
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=-1;
				}
				else
				{
					n[3]=g[immed-Width+t-1];n[2]=g[immed-Width+t];n[1]=g[immed-Width+t+1];
					n[4]=g[immed+t-1];n[8]=g[immed+t+1];
					n[5]=g[immed+Width+t-1];n[6]=g[immed+Width+t];n[7]=g[immed+Width+t+1];
				}
			}
}
//my function	3 2 1 j width
//				4 * 8
//		height	5 6	7
//				i 
//freeman coding primitive, for a image tag width*height with distinct label for each region boundary
//and the interior of regions are assigned -1, finding a start pixel and search its 1-8 directions
//for candidate edge pixel
void BorderEncode(int *tag,int Width, int Height)
{
    CvMemStorage* storage = cvCreateMemStorage(0);
	CvSeq* ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
		sizeof(CvPoint), storage );
	CvSeq*hull,*cur=ptseq;
	CvPoint pt0;
	int k,nrn,n[9],label,sernum;
//	long int lend;
//	int thresh=160,thresh2=180;
	int i,j,s,t,immed,curtag,cc;
	long int l=0,l0; 
//	int l2,i1=0,j1=0;
	int *g=tag;
	/*检测出发点*/
	sernum=0;
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{
			label=g[sernum];
			if(j>0&&i>0&&(j<Width-1)&&(i<Height-1))
			{
				if((label==tag[sernum+1])&&(label==tag[sernum-1])&&(label==tag[sernum-Width])&&(label==tag[sernum+Width]))
				{
					g[sernum]=-1;
				}
			}
			++sernum;
		}
	}
	
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			immed=i*Width;
			if(g[immed+j]==-1)continue;
			
			s=i;t=j;
			GetMask(t,s,g,Width,Height,n);
			nrn=0;
			for(k=1;k<=8;++k)
				nrn+=n[k];
			/*孤立点检测*/
			if(nrn==-8)
			{				
				pt0.x =j;
				pt0.y =i;
				cvSeqPush( cur, &pt0 );
				hull=cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
					sizeof(CvPoint), storage );
				cur->h_next=hull;
				cur=hull;
				g[immed+j]=-1;
				continue;
			}
			/*iteration check chains检测*/
			//l length of the loop, cc previous link relative position
			l=1;
			curtag=g[immed+t];
			
			do{
				pt0.x =t;
				pt0.y =s;
				cvSeqPush( cur, &pt0 );
				printf("%d,%d\t",t,s);
				immed=s*Width;
				g[immed+t]=-1;
			
				l0=l;
				for(k=1;k<=8;k++)
				{
					if(n[k]!=curtag)continue;
					if(k==8)k=0;
					//判断是否是前一个点？否继续移向相邻点
					if(l!=1){if(abs(k-cc)==4)continue;}
					cc=k;
					++l;
					/* 将3×3窗口移向相邻点*/
					switch(k)
					{
					case 1:s--;t++;break;
					case 2:s--;break;
					case 3:s--;t--;break;
					case 4:t--;break;
					case 5:s++;t--;break;
					case 6:s++;break;
					case 7:s++;t++;break;
					case 0:t++;break;
					}
					//if((s<1)||(s>height-2)||(t<1)||(t>width-2))break;
					if(l0<l)break;					
					
				}/*转向下一个出发点*/
				if(k==9)break;
				GetMask(t,s,g,Width,Height,n);	
			}while(s!=i||t!=j);
			printf("\n");
			hull=cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32SC2, sizeof(CvContour),
				sizeof(CvPoint), storage );
			
			cur->h_next=hull;
			cur=hull;
		}
	}
	//last iteration to check whether exist nonnegative pixels
	sernum=0;
	for(i=0;i<Height;i++)
	{
		for(j=0;j<Width;j++)
		{
			if(g[sernum]!=-1)
			//	AfxMessageBox("error!");
			printf("error");
			++sernum;
		}
	}
}

void CHC::LenWidR()
{
	CvMemStorage* storage= cvCreateMemStorage(0);
	CvSeq*contours,*hull;
	CvPoint pt0;//,pt;
	CvPoint2D32f ptf[4];
	CvSize sz=cvSize(Width,Height);
	CvBox2D rect;
	int sernum=0,i=0,j=0,hullcount=0;
    IplImage* gray = cvCreateImage( sz, 8, 3 ); 
	
	const char* wndname = "result";
	cvNamedWindow( wndname, 1 );
	
	contours=GetBoundary(tag,Width,Height,storage);
	
	while( contours)
	{	
		//		int  dam=contours->total; 
		rect=cvMinAreaRect2(contours);
		cvBoxPoints(rect,ptf);
	//	if((rect.size.height/rect.size.width>2.f)||(rect.size.width/rect.size.height>2.f))
		{
			//draw the rect in img representation approach 1
		/*	pt0=cvPointFrom32f(ptf[3]);			
			for( i = 0; i < 4; i++ )
			{
				CvPoint pt =cvPointFrom32f(ptf[i]); 
				cvLine( gray, pt0, pt, CV_RGB( 0, 255, 0 ), 3, CV_AA, 0 );
				pt0 = pt;
			}*/
			//highlight the contours of objects representation approach 2
		/*	hullcount=contours->total;
			for( i = 0; i < hullcount; i++ )
			{
				
				pt =*CV_GET_SEQ_ELEM( CvPoint, contours, i );
				uchar* temp_ptr = &((uchar*)(gray->imageData + gray->widthStep*pt.y))[pt.x*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
		}*/
			//draw the convex hull result display approach 3
			hull = cvConvexHull2( contours, 0, CV_CLOCKWISE, 0 );
			hullcount = hull->total;
			pt0 = **CV_GET_SEQ_ELEM( CvPoint*, hull, hullcount - 1 );
			for( i = 0; i < hullcount; i++ )
			{
				CvPoint pt = **CV_GET_SEQ_ELEM( CvPoint*, hull, i );
				cvLine(gray, pt0, pt, CV_RGB( 0, 255, 0 ));
				pt0 = pt;
			}

		}		 
		++j;
		// take the next contour
		contours = contours->h_next;
	}
//		CString hun;hun.Format("contours:%d\n",j);
//		AfxMessageBox(hun);
    

	cvShowImage( wndname,gray ); 
	cvReleaseImage( &gray );
	cvClearMemStorage( storage);	
}
//road pixel to store road map, fisdata for parameter setting default null
//when saving roads one flip operation performed, so before assign roadseed, a counteract flip is performed
void CHC::RoadSeed(BYTE*roadpixel,char *fisData)
{	
	int cpt, avrW,elgAL, elgWL, mnv;//compactness*100,AVR WIDTH,
	//	int i,L=Height*Width;
	if(fisData)
	{
		//ifstream guild(fisData);	
		//guild>>cpt>>avrW>>elgAL>>mnv;
		
		IplImage* img = cvLoadImage(fisData);
		if(img!=0)
		{
			if((Height!=img->height)||(Width!=img->width))
			{
				AfxMessageBox("Image size incongruent, fake road seed map encounted!");
				return;
			}
			
			int i,j,sernum=0,L=Height*Width;
			int alpha=img->nChannels,beta=img->widthStep;
		
			for(i=0;i<Height;++i)
			{
				for(j=0;j<Width;++j)
				{
					roadpixel[sernum]=(((uchar*)(img->imageData + beta*i))[alpha*j]>0?1:0);
					++sernum;
				}				
			}			
			cvReleaseImage( &img);
		}
		return;
	}
	else
	{
		cpt=15;
		avrW=15;
		elgAL=4;
		mnv=15;
		elgWL=3;
	}
	int i,L=Height*Width;
	set<int> cand;//store the road seed region tags
	int ratlen=S.size();
	//thresholding
	for(i=0;i<ratlen;++i)
	{
		if(S[i].p!=i)
			continue;
		//it seems cmpct does not necessary 
	//	if((S[i].attlist[ndvindex]*100<mnv)&&
			if((S[i].attlist[meanwidth]<avrW)&&((S[i].attlist[meanlen]/S[i].attlist[meanwidth])>elgAL)
			&&(S[i].attlist[maxaxislen]>80)
			&&((float)S[i].perim/(grid[i].Height()+grid[i].Width())<3.5f))//&&S[i].MaxAL>10&&(S[i].cmpct<0.15)
		cand.insert(i);
	}
	memset(roadpixel,0,sizeof(BYTE)*Height*Width);
	for(i=0;i<L;++i)
	{
		if(cand.find(tag[i])!=cand.end())
			roadpixel[i]=1;
	}
	cand.clear();

	int x,y;//,perim;
	IplImage*portal;
	CvSize bound=cvSize(Width,Height);
	portal=cvCreateImage(bound, IPL_DEPTH_8U , 1);
	cvNamedWindow("Portal", 0);	
	cvZero(portal);
	for (i= 0; i <L; ++i) 
	{
		if(roadpixel[i])
		{
			x=i%Width;
			y=i/Width;
			((uchar*)(portal->imageData + portal->widthStep*y))[x]=255;			
		}	
	}
	

	cvShowImage("Portal", portal);
	cvSaveImage("roads.bmp",portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);

	//get end points and initial direction
	//template extraction
	//road tracking using profile matching, curve fitting, template matching. 
	//curve fitting extrapolate the  next point, profile matching to adjust the point or weighting between predicted point and profile matching point
}
//output quadtree seg result for paper writing
void CHC::qttest(int md)
{
	int dim=d_;
	int h=Height,w=Width;
	int L=h*w;
	comps=L;
	int x,y,d;
	int sernum=0;

	memset(tag,-1,L*sizeof(int));
	int i=0,j=0,sz=0;
	int rec=0;//current number of segments
	int step=0,hs=0,hs2=0;//edge length of blocks checked in each loop and half step
	
	int flag=1,temp=0;
	int limx,limy;
	int ul,ur,ll,lr;//four upper left corner point index in four quadrant
	static int proxy=0;
	//quadtree merge main section and Regions initiation
	while(step<md)//(flag>0)//some blocks have not yet been assigned to region
	{
		flag=0;
		++temp;
		step=1<<temp;
		hs=step>>1;
		hs2=hs*hs;
		limy=(int)ceil((double)h/step)*step;
		limx=(int)ceil((double)w/step)*step;
		for(y=0;y<limy-step;y+=step)
		{
			for(x=0;x<limx-step;x+=step)
			{
				//	CheckBlock(x,y,step);
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs2,lr,hs2))
				{
					//predicate is met
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled or not pure
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
					
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
					
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
			}
		}
		assert(y==limy-step);
		//for blocks lying on the boundary first check the downside boundary and use y 
		//with the same value gotten from above
		for(x=0;x<limx-step;x+=step)
		{
			//if only two component exist
			if(y+hs>=h)
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,(h-y)*hs,ur,(h-y)*hs))
				{
				for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs2,ll,hs*(h-hs-y),lr,hs*(h-hs-y)))
				{
					//predicate is met
					//predicate is met
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				else//this block contains some component has upper left corner pixel labelled 
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
					
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}//end else four block are assigned region label
			}//end else for y+hs<h
		}//end for each block lying on the bottom from left to right
		assert(x==limx-step);
		//check blocks lying to right side of the boundary and the bottom one is not included just as above
		for(y=0;y<limy-step;y+=step)
		{
			if(x+hs>=w)
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,hs*(w-x)))
				{					
					for(d=0;d<dim;++d)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;					
				}
				//if two blocks contains labeled pixel or heterogeneous
				else
				{
					if(tag[ul]==-1)
					{
						
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else// if y+hs<h, four blocks should be considered just as the beignning
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-hs-x),ll,hs2,lr,hs*(w-hs-x)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
					
					
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						proxy=ll;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
						
						proxy=lr;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			
		}
		assert(x==limx-step);
		assert(y==limy-step);
		if(x+hs>=w)
		{
			if(y+hs<h)//two blocks are considered
			{
				ul=y*w+x;
				ll=(y+hs)*w+x;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ll]==-1&&Predicate(ul,hs*(w-x),ll,(w-x)*(h-hs-y)))
				{
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ll*dim+d];
						sData_[ul*dim+d]+=sData_[ll*dim+d];
					}
					++flag;
					
				}
				//if two blocks contains labeled pixel
				else
				{
					if(tag[ul]==-1)
					{
						
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						
						
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					
				}
			}
			else//only one component exists this block must have been check before by predicate
			{//no addition and square sum is needed as both are computed already
				ul=y*w+x;
				if(tag[ul]==-1)
				{
					++flag;
				}
			}
		}		
		else//if x+hs<w
		{
			if(y+hs<h)//four blocks case
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				ll=(y+hs)*w+x;
				lr=(y+hs)*w+x+hs;
				//if this block is not contaminated, it has chance to be merged
				if(tag[ul]==-1&&tag[ur]==-1&&tag[ll]==-1&&tag[lr]==-1&&Predicate(ul,hs2,ur,hs*(w-x-hs),ll,(h-y-hs)*(hs),lr,(h-y-hs)*(w-x-hs)))
				{
					//predicate is met
					
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=(data_[ur*dim+d]+data_[ll*dim+d]+data_[lr*dim+d]);
						sData_[ul*dim+d]+=(sData_[ur*dim+d]+sData_[ll*dim+d]+sData_[lr*dim+d]);
					}
					++flag;
				}
				//this block contains some component has upper left corner pixel labelled 
				else
				{
					//if tA is -1 which means the quadrant is not yet assigned to region,so do it
					if(tag[ul]==-1)
					{
						
						proxy=ul;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
						
						proxy=ur;
						for(j=0;j<hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ll]==-1)
					{
						proxy=ll;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ll;
							proxy+=w;
						}
						++rec;
					}
					if(tag[lr]==-1)
					{
					
						proxy=lr;
						for(j=0;j<h-y-hs;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=lr;
							proxy+=w;
						}
						++rec;
					}	
				}
				
			}
			else//y+hs>h
			{
				ul=y*w+x;
				ur=y*w+x+hs;
				//if two small blocks are pure, it is possible a new pure block can be founded
				if(tag[ul]==-1&&tag[ur]==-1&&Predicate(ul,hs*(h-y),ur,(h-y)*(w-x-hs)))
				{
					for(d=0;d<dim;d++)
					{
						data_[ul*dim+d]+=data_[ur*dim+d];
						sData_[ul*dim+d]+=sData_[ur*dim+d];
					}
					++flag;
					
				}
				else	//if two blocks contains labeled pixel
				{
					if(tag[ul]==-1)
					{
					
						proxy=ul;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<hs;i++)
								tag[proxy+i]=ul;
							proxy+=w;
						}
						++rec;
					}
					if(tag[ur]==-1)
					{
					
						proxy=ur;
						for(j=0;j<h-y;j++)
						{
							for(i=0;i<w-x-hs;i++)
								tag[proxy+i]=ur;
							proxy+=w;
						}
						++rec;
					}
					
				}
				
			}
		}
	}
}
//process binary image data stored in EM with white background
void CHC::Morph(int opt)
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

void CHC::SetWH(int w, int h)
{
	Width=w;
	Height=h;
}
//this two function is defined for binmorph
BYTE myMax(BYTE dimm[], BYTE num)
{
	BYTE bmax = dimm[0];
	for( BYTE ii = 1; ii <num; ii++)
		if( bmax < dimm[ ii ] )
			bmax = dimm[ ii ];
	return bmax;
}


BYTE myMin(BYTE dimm[], BYTE num)
{
	BYTE bmin = dimm[0];
	for( BYTE ii = 1; ii <num; ii++)
		if( bmin > dimm[ ii ] )
			bmin = dimm[ ii ];
	return bmin;
}
//binmorph at first is intended to compute the average width of an binary object which is supplanted by distance 
//transform in opencv,. whatever, this function can erode objects boundaries iteratively
//lpos the pointer to binary caricature with size (mw)*(mh),mind padding boundary of width 2
// erode white object black background,
//mw width of bounding box mh height of bounding box, total object pixels number,
//mode 46 and 46 for square and rhombus erosion structure element, nrepeat the time of erosion operation

void CHC::BinMorph(LPBYTE	lpos,int mW,int mH, int total,BYTE mode, BYTE&nRepeat)
{
	long	row,col, pos;	

	BYTE	bResult;
	BYTE pb[9];

	BYTE* pool=new BYTE[mW*mH];
	memcpy(pool,lpos,mW*mH*sizeof(BYTE));
	nRepeat=0;
	while(total>0)
	{	
		for(row=1; row<mH-1; row++)
		{
			pos=1+row*mW;
			for(col=1; col<mW-1; col++)
			{						
				switch( mode )
				{
				case MMSquare:
					{
						pb[0]=lpos[pos -mW- 1];
						pb[1]=lpos[pos - mW ];
						pb[2]=lpos[pos - mW + 1];
						pb[3]=lpos[pos - 1];
						pb[4]=lpos[pos + 1];
						pb[5]=lpos[pos + mW - 1];
						pb[6]=lpos[pos + mW ];
						pb[7]=lpos[pos + mW + 1];
						pb[8]=lpos[pos ];
						bResult = myMin(pb , 9);
						break;
					}
				case MMRhombus:
					{
						pb[0]=lpos[pos - mW ];
						pb[1]=lpos[pos - 1];
						pb[2]=lpos[pos ];
						pb[3]=lpos[pos + 1];
						pb[4]=lpos[pos + mW ];
						bResult = myMin(pb , 5);
						break;
					}
				default:
					break;
				}
				if( lpos[pos]==1&&bResult==0)
					--total;
				if(total==0)
					break;	
				pool[pos]= bResult;
				pos++;							
			}
			if(total==0)
					break;
		}
		memcpy(lpos,pool,mW*mH*sizeof(BYTE));
		++nRepeat;
	}
	delete []pool;
}



//at first, regmoment tries to compute the moments of a particular region; 
//later, it displays a specific region in a window on left button down
//note error crashes when try to open two opencv windows.

void CHC::RegMoment(int label)
{
	assert(grid);
	int mH=S[label].norbox->Height(),mW=S[label].norbox->Width();
	int i,j,mx,my,pos,total;
	const char* wndname = "Region";
	CvMemStorage* storage= cvCreateMemStorage(0);
	CvSeq*contours,*result;
//	CvPoint* pt0,*pt;
	CvSize sz = cvSize(mW+2,mH+2);
    IplImage* gray = cvCreateImage( sz, 8,1 );
	cvZero(gray);
	BYTE*patch=(BYTE*)(gray->imageData);
	mx=S[label].norbox->left;
	my=S[label].norbox->top;
	pos=my*Width+mx;
	total=0;
	for(j=1;j<mH+1;++j)
	{			
		for(i=1;i<mW+1;++i)
		{
			if(tag[pos]==label)
			{
				//	((uchar*)(patch + gray->widthStep*j))[i]=1;
				(patch+ gray->widthStep*j)[i]=255;
				++total;
			}				
			++pos;			
		}
		pos+=Width-mW;		
	}
	assert(total==S[label].size);
	cvFindContours( gray, storage, &contours, sizeof(CvContour),
		CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0) );
	float area=(float)cvContourArea(contours, CV_WHOLE_SEQ);
	float	beam=cvArcLength(contours,CV_WHOLE_SEQ,1);
//	CString modem;
//	modem.Format("area of contours:%.2f and arclength %.2f \n",area,beam);	  
//	AfxMessageBox(modem);
	result = cvApproxPoly( contours, sizeof(CvContour), storage,
			  CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0 );
//	assert(!contours->h_next);//note when region has holes, there is more than 1 contour


/*	pt=(CvPoint*)cvGetSeqElem( result, result->total-1 );
	for( i = 0; i < result->total; i ++)
	{
		pt0=pt;
		pt=(CvPoint*)cvGetSeqElem( result, i );
		// draw the square as a closed polyline 
		cvLine( cpy, *pt0, *pt, CV_RGB(0,255,0), 3, 8 );
    }*/
	IplImage* cpy = cvCreateImage( sz, 8,3 ); 
	cvZero(cpy);
	if( contours)
		cvDrawContours(cpy,contours,CV_RGB(0,255,0),CV_RGB(0,255,0),100);
	cvReleaseImage( &gray );
	cvClearMemStorage(storage);
	cvNamedWindow(wndname, 1 );

	cvShowImage( wndname, cpy );
	cvWaitKey(0);
    cvReleaseImage( &cpy );
	cvDestroyWindow( wndname);
}
//save object mean image of segmentation and contour only applies to small image
void CHC::SaveSeg(GDALDataset *m_pDataset,CString pathname)
{
	int L=Height*Width;

	int x,y,d,sernum,label;
	int nByteWidth=d_*Width;
	float* buf =new float[Width*Height];
	GDALRasterBand  *m_pBand=NULL;
	for(d=1;d<d_+1;++d)
	{
		m_pBand= m_pDataset->GetRasterBand(d);
		if (m_pBand)
		{	
			sernum=0;
			
			for(y=0;y<Height;++y)
			{
				for (x = 0; x < Width; ++x) 
				{					
					label=tag[sernum];
					buf[sernum]=(S[label].addition[d-1])/(S[label].size)*Range;							
					if(x>0&&y>0&&(x<Width-1)&&(y<Height-1))
						if(label!=tag[sernum+1]||label!=tag[sernum+Width])//one pixel width boundary
							buf[sernum]=0;
						++sernum;
				}				
			}
			if (CE_None!=m_pBand->RasterIO( GF_Write,0,0, Width, Height, buf, Width,Height,GDT_Float32, 0, 0 ))
			{
				AfxMessageBox("error write mpdataset!");
			}
		}
	}
	delete[]buf;
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	char **papszMetadata;
	GDALDataset* poDstDS;
	int pos=pathname.ReverseFind('\\');
	pathname=pathname.Left(pos);
	pathname+="\\contour seg.tif";
	const char*pszDstFilename=(const char*)pathname;
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	
	if( poDriver == NULL)
		exit( 1 );
	
	papszMetadata = poDriver->GetMetadata();
/*	if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATE, FALSE ) )
	{	herald.Format( "Driver %s supports Create() method.\n", pszFormat );
	AfxMessageBox(herald);}
	if( CSLFetchBoolean( papszMetadata, GDAL_DCAP_CREATECOPY, FALSE ) )
	{	herald.Format( "Driver %s supports CreateCopy() method.\n", pszFormat );
	AfxMessageBox(herald);}*/
	if(poDstDS=poDriver->CreateCopy( pszDstFilename, m_pDataset, FALSE, NULL, NULL, NULL))
		GDALClose( (GDALDatasetH) poDstDS);
}

int CHC::GetSize(int label)
{
	return S[label].size;
}

CRect CHC::GetBoundBox(int miss)
{
	return *(S[miss].norbox);
}
//following freeman code style as introduced in opencv reference
//roadseed points a memory space of size width*height with 1 denotes roads 0 for background 
//after processing, 2 for centerline
//vector<int>terminal returns the end pixels in form of serial number in roads after pruning
void CHC::RoadThin(BYTE*roadseed,vector<int>&terminal)
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
	vector<int> edgecode;
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
	cvReleaseImage(&portal);*/
	
	delete[]roads;

}
//group road ce3nterlines, if end points are within distance of 5pixels, centerlines connected,
//if end points are away from each other by 9 pixels,but about in the same direction, connect them
//in roadmap, 1 for road, 0 for background, in terminal the order index of end point
//in roadmap 2 for end point in this function 
void CHC::RoadLink(BYTE*roadmap,vector<int>&terminal)
{
	//nearest end point search
	//initiate road end pixel 2
	int tsize=terminal.size(),i,j,k,x,y;//x,y coordinate i,j for index incremental
	const int radius=5;//search radius
	int left,right,top,bottom;// search window
	int pos,count;

	CvSize bound=cvSize(Width,Height);
	IplImage*portal=cvCreateImage(bound, IPL_DEPTH_8U , 3);
	cvNamedWindow("Portal", 0);		
	cvZero(portal);

	for(i=0;i<tsize;++i)
	{
		assert(roadmap[terminal[i]]);
		++roadmap[terminal[i]];//assign 2
	}
	for(i=0;i<tsize;++i)
	{
		if(!roadmap[terminal[i]])
			continue;
		x=terminal[i]%Width;
		y=terminal[i]/Width;
		left=0>(x-radius)?0:(x-radius);
		right=(x+radius)>(Width-1)?(Width-1):(x+radius);
		top=0>(y-radius)?0:(y-radius);
		bottom=(y+radius)>(Height-1)?(Height-1):(y+radius);
		pos=top*Width+left;
		for(j=top;j<=bottom;++j)
		{
			for(k=left;k<=right;++k)
			{
				if(roadmap[pos]==2&&pos!=terminal[i])//link the two nodes
				{
					roadmap[pos]=0;
					roadmap[terminal[i]]=0;
					cvLine( portal, cvPoint(x,y), cvPoint(k,j), CV_RGB(0,255,0), 3, CV_AA, 0 );
				}
				++pos;
			}
			pos=pos+Width-(right-left+1);
		}
	}
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{				
			if(roadmap[i*Width+j]>0)//(img2[j][i]==1)
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

	cvShowImage("Portal", portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);
	//edge extrapolation
}
//input line[0] and line[1] line extending direction, line[2][3] x,y coordinate
//len extending length, width of image line section range [-(len-1)/2,(len)/2]
void GetProfile(float line[4],int len,int Width, vector<int>&pontiff)
{
	pontiff.clear();
	int relay,j;
	
	CvPoint pt1;
	
	for(j=-(len-1)/2;j<len/2+1;++j)
	{		
		pt1.x = cvRound(line[2]+line[0]*j);
		pt1.y = cvRound(line[3]+line[1]*j);
		relay=pt1.y*Width+pt1.x;							
		pontiff.push_back(relay);
		
	}
}
void GetBand(GDALDataset* m_pSrc,float*buf,int d)
{

	GDALRasterBand  *m_pBand=NULL;
	int Width=m_pSrc->GetRasterXSize(); //影响的高度，宽度
	int	Height=m_pSrc->GetRasterYSize();
	float *tempbuf=new float[Width*Height];
	int x,y,sernum,temp;
		m_pBand= m_pSrc->GetRasterBand(d);
		if (m_pBand)
		{	
			if (CE_None==m_pBand->RasterIO( GF_Read,0,0, Width, Height, tempbuf, Width,Height, GDT_Float32, 0, 0 ))
			{
				sernum=0;
				temp=Height*Width-Width;
				for(y=0;y<Height;++y)
				{
					for (x = 0; x < Width; ++x) 
					{
						buf[sernum]=tempbuf[temp];						
						++sernum;
						++temp;
					}
					temp-=2*Width;
				}
			}
		}
		delete[]tempbuf;
}
//compute kappa coefficient 
double Eval(int* ts, int*ind,int numd,int rts,int rind)
{//test result, indicator after clustering, number data, range of test sample counting from 1
	//rind range of indicator counting from 1
	if(rts>rind)
	{
		int temp=rind;
		rind=rts;
		rts=temp;
		int *port=ind;
		ind=ts;
		ts=port;
	}
	int i=0,j,k;
	int s=0,t=0;
	int *eu=new int[rts*rind];//count the occurence of each entry
	int *rl=new  int[rts];//record the index for the maximum in each row
	int *maxm=new int[rts];//record maxima for each row
	int *rms=new int[rts];//row sum except the maxima
	int *cms=new int[rind];//column sum except the maxima
	memset(eu,0,sizeof(int)*rts*rind);
	memset(rl,0,sizeof(int)*rts);
	memset(maxm,0,sizeof(int)*rts);
	memset(rms,0,sizeof(int)*rts);	
	memset(cms,0,sizeof(int)*rind);
	int *ptr;
	int num=0;double po=0,pe=0;
	for(i=0;i<numd;i++)
	{
		j=ts[i];
		k=ind[i];
		eu[j*rind+k]++;
	}
	//compute po 
	for(i=0;i<rts;i++)
	{
		ptr=max_element((eu+i*rind),(eu+(i+1)*rind));
		rl[i]=ptr-(eu+i*rind);
		maxm[i]=*ptr;
	}
	//compute pe
	for(i=0;i<rts;i++)
	{
		s=rl[i];
		for(j=0;j<rts;j++)
		{
			t=rl[j];
			cms[s]+=eu[j*rind+s];
			rms[i]+=eu[i*rind+t];
		}
	}
	for(i=0;i<rts;i++)
	{
		s=rl[i];
		pe+=(rms[i]-maxm[i])*(cms[s]-maxm[i]);
		num+=rms[i];
		po+=maxm[i];
	}
	pe/=(num*num);
	po/=num;
	delete []rl;delete []eu;
	delete []maxm;delete []rms;delete []cms;
	return (po-pe)/(1-pe);
}
double Eval(int* eu,int rts,int rind)
{//test result, indicator after clustering, number data, range of test sample counting from 1
	//rind range of indicator counting from 1
	//some modification's needed when rts>rind 
	int i=0,j,k;
	if(rts>rind)
	{
		int temp=rind;
		rind=rts;
		rts=temp;
		int *eut=new int[rts*rind];
		for(j=0;j<rts;j++)
			for(k=0;k<rind;k++)
				eut[j*rind+k]=eu[k*rind+j];
		for(j=0;j<rts*rind;j++)
			eu[j]=eut[j];
		delete []eut;
	}

	int s=0,t=0;
	int *rl=new  int[rts];//record the index for the maximum in each row
	int *maxm=new int[rts];//record maxima for each row
	int *rms=new int[rts];//row sum except the maxima
	int *cms=new int[rind];//column sum except the maxima

	memset(rl,0,sizeof(int)*rts);
	memset(maxm,0,sizeof(int)*rts);
	memset(rms,0,sizeof(int)*rts);	
	memset(cms,0,sizeof(int)*rind);
	int *ptr;
	int num=0;double po=0,pe=0;
	//compute po 
	for(i=0;i<rts;i++)
	{
		ptr=max_element((eu+i*rind),(eu+(i+1)*rind));
		rl[i]=ptr-(eu+i*rind);
		maxm[i]=*ptr;
	}
	//compute pe
	for(i=0;i<rts;i++)
	{
		s=rl[i];
		for(j=0;j<rts;j++)
		{
			t=rl[j];
			cms[s]+=eu[j*rind+s];
			rms[i]+=eu[i*rind+t];
		}
	}
	for(i=0;i<rts;i++)
	{
		s=rl[i];
	
		pe+=(rms[i]-maxm[i])*(cms[s]-maxm[i]);
		num+=rms[i];
		po+=maxm[i];
	}
	pe/=(num*num);
	po/=num;
	delete []rl;
	delete []maxm;delete []rms;delete []cms;
	return (po-pe)/(1-pe);
}
//frc has been normalized, bench contains profile pixels, test contains test profile pixels
float Correlate(float *src, vector<int>&bench, vector<int>&test)
{
	assert(bench.size()==test.size());
	int bs=bench.size(),i;
	float simil=0;
	for(i=0;i<bs;++i)	
		simil+=square(src[bench[i]]-src[test[i]]);	
	return simil;
}
//road tracking starting from valid endpoints stored in terminal
//in roadmap, 0 for background,1 for road, 2 for road centerline,3 for terminal points.
// in terminal the order index of end point
void CHC::RoadExpo(BYTE*roadmap,vector<int>&terminal,GDALDataset* m_pSrc)
{
	int tsize=terminal.size(),i,j,k;//x,y coordinate i,j for index incremental
	vector<int> edgecode;
	int normin[8]={0},dest[8]={0};
	int sernum,count,runlen,next;
	int x,y,hsize;//x,y coordinate for pixel, tsize the size of stack storing terminals, 
	//hsize terminals deleted during pruning
	const int dangle=5;
	bool flag,lof;//flag for detecting small spurs, lof linked or failed for while loop in road tracking	
	CvSize bound=cvSize(Width,Height);
	IplImage*portal=cvCreateImage(bound, IPL_DEPTH_8U , 3);
	cvNamedWindow("Portal", 0);		
	cvZero(portal);
	
	float *vegind=new float[Width*Height];
	GetBand(m_pSrc,vegind,4);
	for(i=0;i<Height;++i)
	{
		for(j=0;j<Width;++j)
		{				
			if(roadmap[i*Width+j]==1)//(img2[j][i]==1)
			{
				uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*i))[j*3];
			temp_ptr[0]=0;
			temp_ptr[1]=255;
			temp_ptr[2]=0;
			}
			if(roadmap[i*Width+j]==2)//(img2[j][i]==1)
			{
				uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*i))[j*3];
			temp_ptr[0]=255;
			temp_ptr[1]=0;
			temp_ptr[2]=0;
			}
		}   
	}
	hsize=0;
	for(i=0;i<tsize;++i)
	{	
		sernum=terminal[i];//position in image array
		if(roadmap[sernum]<2)
			continue;
		runlen=0;
		flag=true;//true for proceeding, false for halt while loop
		edgecode.clear();
		//get the last five pixels connecting the terminal and fit line
		while(runlen<dangle)
		{
			x=sernum%Width;
			y=sernum/Width;
			for(j=0;j<8;++j)
				normin[j]=0;
			if(x==0)
			{
				if(y==0)
				{					
					normin[0]=roadmap[sernum+1];
					normin[6]=roadmap[sernum+Width];
					normin[7]=roadmap[sernum+1+Width];
				}
				else if(y==Height-1)
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-Width];
					normin[2]=roadmap[sernum-Width];
				}
				else
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-Width];
					normin[2]=roadmap[sernum-Width];
					normin[6]=roadmap[sernum+Width];
					normin[7]=roadmap[sernum+1+Width];					
				}
			}
			else if(x==Width-1)
			{
				if(y==0)
				{
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+Width];
					normin[6]=roadmap[sernum+Width];
				}
				else if(y==Height-1)
				{
					normin[2]=roadmap[sernum-Width];
					normin[3]=roadmap[sernum-1-Width];
					normin[4]=roadmap[sernum-1];
				}
				else
				{
					normin[2]=roadmap[sernum-Width];
					normin[3]=roadmap[sernum-1-Width];
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+Width];
					normin[6]=roadmap[sernum+Width];					
				}
			}
			else
			{
				if(y==0)
				{
					normin[0]=roadmap[sernum+1];
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+Width];
					normin[6]=roadmap[sernum+Width];
					normin[7]=roadmap[sernum+1+Width];
				}
				else if(y==Height-1)
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-Width];
					normin[2]=roadmap[sernum-Width];
					normin[3]=roadmap[sernum-1-Width];
					normin[4]=roadmap[sernum-1];
				}
				else
				{
					normin[0]=roadmap[sernum+1];
					normin[1]=roadmap[sernum+1-Width];
					normin[2]=roadmap[sernum-Width];
					normin[3]=roadmap[sernum-1-Width];
					normin[4]=roadmap[sernum-1];
					normin[5]=roadmap[sernum-1+Width];
					normin[6]=roadmap[sernum+Width];
					normin[7]=roadmap[sernum+1+Width];
					
				}
			}					
			count=0;
			for(k=0;k<8;++k)
			{
				if(normin[k]==2)
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
			case 3:
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
			
		}//end while loop for tail trapping
		assert(runlen==edgecode.size());
		if(runlen<dangle)//pruning  pixels in edgecode 
		{
			for(k=0;k<runlen;++k)			
				--roadmap[edgecode[k]];
			++hsize;
			continue;
		}
		//extrapolating using profile matching
		/*	if()//search edgecode[0] radius=3 neighborhood
		{
		continue;
		}*/
		//get template profile perpendicular to the line in the further end to avoid spurs
		
		float line[4],tmpl[4];
		int end[2]={0},t,start,left,right;
		int touch=0,relay,dstep;//decide whether both direction touched the border
		vector<int> profile,pontiff;//template profile and test profile
		float temp,mini;
		CvPoint pt1;
		assert(dangle==edgecode.size());
		CvPoint* points = (CvPoint*)malloc( dangle * sizeof(points[0]));
		CvMat pointMat = cvMat( 1, dangle, CV_32SC2, points );
		for( k=0;k<dangle;++k)
		{
			points[k].x = edgecode[k]%Width;
			points[k].y = edgecode[k]/Width;
		}
		cvFitLine( &pointMat, CV_DIST_L1, 1, 0.001, 0.001, line );
		relay=edgecode[dangle-1];
		
		t=0;
		while(roadmap[relay]>0)
		{
			++t;
			pt1.x = cvRound(points[dangle-1].x+line[1]*t);
			pt1.y = cvRound(points[dangle-1].y- line[0]*t);
			
			relay=pt1.y*Width+pt1.x;
		}
		end[0]=1-t;
		t=0;
		relay=edgecode[dangle-1];
		while(roadmap[relay]>0)
		{
			++t;
			pt1.x = cvRound(points[dangle-1].x - line[1]*t);
			pt1.y = cvRound(points[dangle-1].y + line[0]*t);
			
			relay=pt1.y*Width+pt1.x;
		}
		end[1]=t-1;
		if(end[1]-end[0]>8)//road anormaly fat reject terminal
			continue;
		profile.clear();
		for(j=end[0];j<end[1]+1;++j)
		{
			pt1.x = cvRound(points[dangle-1].x-line[1]*j);
			pt1.y = cvRound(points[dangle-1].y+line[0]*j);
			relay=pt1.y*Width+pt1.x;
			profile.push_back(relay);
		}
		for(j=0;j<profile.size();++j)
		{				
			x=profile[j]%Width;
			y=profile[j]/Width;
			uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*y))[x*3];
			temp_ptr[0]=0;
			temp_ptr[1]=0;
			temp_ptr[2]=255;
		}
		lof=false;
		start=edgecode[0];
		while(!lof)			
		{			
			//predict the next pixel and retrieve teh profile based on the line direction with increment of x=3
			//points[0] is the terminal point and the line direction is from point[0] to point[5] so minus
			k=profile.size();
			dstep=(dangle+1)/2;
			//7.f equals to move ahead with step 3
			if((points[0].x-points[dangle-1].x>0)||((points[0].x-points[dangle-1].x==0)&&(points[0].y-points[dangle-1].y>0)))
			{
				x=start%Width;
					y=start/Width;
					tmpl[2]= cvRound(x+line[0]*dstep);
					tmpl[3]= cvRound(y+line[1]*dstep);
				tmpl[0]=-line[1];
				tmpl[1]=line[0];
					relay=tmpl[3]*Width+tmpl[2];
					GetProfile(tmpl,k,Width,pontiff);
					temp=Correlate(vegind,profile,pontiff);
					mini=temp;
					tmpl[2]= cvRound(x+line[1]);
					tmpl[3] = cvRound(y-line[0]);
					GetProfile(tmpl,k,Width,pontiff);
						temp=Correlate(vegind,profile,pontiff);
						if(temp<mini)
						{	mini=temp;
						relay=tmpl[3]*Width+tmpl[2];
						}
					tmpl[2]= cvRound(x-line[1]);
					tmpl[3] = cvRound(y+line[0]);
				GetProfile(tmpl,k,Width,pontiff);
					Correlate(vegind,profile,pontiff);
						if(temp<mini)
						{	mini=temp;
						relay=tmpl[3]*Width+tmpl[2];
						}
			}
			else
			{
				x=start%Width;
					y=start/Width;
					tmpl[2]= cvRound(x-line[0]*dstep);
					tmpl[3] = cvRound(y-line[1]*dstep);
					tmpl[0]=-line[1];
				tmpl[1]=line[0];
					relay=y*Width+x;
					GetProfile(tmpl,k,Width,pontiff);
					tmpl[2]= cvRound(x+line[1]);
					tmpl[3] = cvRound(y-line[0]);
				left=y*Width+x;
				GetProfile(tmpl,k,Width,pontiff);
					tmpl[2]= cvRound(x-line[1]);
					tmpl[3]= cvRound(y+line[0]);
				right=y*Width+x;
				GetProfile(tmpl,k,Width,pontiff);
			}
			
			for(j=0;j<pontiff.size();++j)
			{				
				x=pontiff[j]%Width;
				y=pontiff[j]/Width;
				uchar* temp_ptr = &((uchar*)(portal->imageData + portal->widthStep*y))[x*3];
				temp_ptr[0]=0;
				temp_ptr[1]=0xee;
				temp_ptr[2]=0xee;	
			}		
			lof=true;
			//categorize the line direction and decide the neighboring two candidate pixels
			//profile correlation to determine the next pixel
			//if profile matching is less than a threshold, then go to next terminal
			//if next position is decided, search its head side half of 7*7 window for nodes,if exist, the iteration 
			//is go to next terminal
		}
	}
	edgecode.clear();
	delete[]vegind;
	/*	count=terminal.size();
	for(k=0;k<count;++k)
	{
		i=terminal[k]/Width;
		j=terminal[k]%Width;
		CvPoint center=cvPoint(j,i);		
		cvCircle( portal, center, 2, CV_RGB (255, 0, 0 ),1);
	}*/

	cvShowImage("Portal", portal);
	cvWaitKey(0);
	cvDestroyWindow("Portal");
	cvReleaseImage(&portal);	
}

void CHC::GaussianFilter(float sigma)
{
	int L=Width*Height;
	BYTE*pUnchSmooth=new BYTE[L];
	assert(EM);
	GaussianSmooth(EM, Width, Height, sigma, pUnchSmooth);
	memcpy(EM,pUnchSmooth,sizeof(BYTE)*L);
	delete[]pUnchSmooth;
}

void CHC::GetBWArray(CString src)
{
	bWArray.clear();
	int npos=src.Find(",");
	if(npos==-1)
		return;
	int size;	
	do
	{
		bWArray.push_back(atof((const char*)src));
		npos=src.Find(",");
		size=src.GetLength();
		src=src.Right(size-npos-1);
	}while(npos!=-1);
}
//index=0,1,2 for spectral band 
//index=11, 12,13 for shape feature
void CHC::GetHistogramData(int *dataBin, float &rmax,float &rmin)
{
	if(S.size()==0)
	{
		AfxMessageBox("Data not available!");
		return;
	}
	int i, length, binnum;
	int index=Type;
	float step,curd;//range max,range min,current data
	length=S.size();
	if(index<10)//use S.addition
	{
		if(index>d_-1)
			index=d_-1;
		
	
		rmax=0;
		rmin=255;
		for(i=0;i<length;++i)
		{
			if(S[i].p!=i)
				continue;
			curd=S[i].addition[index]/S[i].size*Range;
			rmax=__max(rmax,curd);
			rmin=__min(rmin,curd);		
		}
		step=(rmax-rmin)/256.f;
		for(i=0;i<length;++i)
		{
			if(S[i].p!=i)
				continue;
			curd=S[i].addition[index]/S[i].size*Range;
			binnum=(int)floor((curd-rmin)/step);
			++dataBin[binnum];				
		}	
	}
	else
	{		
		switch(index)
		{
		case REGSIZE:
			rmax=0;
			rmin=1e10;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=S[i].size;
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);		
			}
			step=(rmax-rmin)/256.f;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;
				curd=S[i].size;			
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];		
			}
			break;
		case PERIM:
			rmax=0;
			rmin=1e10;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=S[i].perim;
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);		
			}
			step=(rmax-rmin)/256.f;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;
				curd=S[i].perim;			
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];		
			}
			break;
		case CMPCT:
			rmax=0;
			rmin=1;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=sqrt(S[i].size*4/3.1416)/S[i].perim;
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);		
			}
			step=(rmax-rmin)/256.f;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=sqrt(S[i].size*4/3.1416)/S[i].perim;			
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];		
			}
			break;
		case MAXAL:
			rmax=0;
			rmin=1e5;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=S[i].attlist[maxaxislen];
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);		
			}
			step=(rmax-rmin)/256.f;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=S[i].attlist[maxaxislen];		
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];		
			}
			break;
		case MEANW:
			rmax=0;
			rmin=1e5;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=S[i].attlist[meanwidth];
				rmax=__max(rmax,curd);
				rmin=__min(rmin,curd);		
			}
			step=(rmax-rmin)/256.f;
			for (i = 0; i <length; i++) 
			{		
				if(i!=S[i].p)			
					continue;	
				curd=S[i].attlist[meanwidth];		
				binnum=(int)floor((curd-rmin)/step);
				++dataBin[binnum];		
			}
			break;
		default:			
			AfxMessageBox("This index is illegible!");break;
		}
	}
}
//distinguish regions with the type indexed property value larger than thresh 
//those regions locate in wind defined area, pixels concerned forms (left,right-1)x(top,bottom-1)
//use opencv image to display the result
void CHC::RegionThresh(float thresh,CRect wind)
{
	if(S.size()==0)
	{
		AfxMessageBox("Data not available!");
		return;
	}
	int i,j,sernum,label;
	float temp;
	Region*chariot;
	if(wind.top<0)
		wind.top=0;
	if(wind.bottom>Height)
		wind.bottom=Height;
	if(wind.left<0)
		wind.left=0;
	if(wind.right>Width)
		wind.right=Width;
	int mH=wind.bottom-wind.top;
	int mW=wind.right-wind.left;
//	long L=Height*Width;
	const char* wndname = "Region";
	CvSize sz = cvSize(mW,mH);
	IplImage* cpy = cvCreateImage( sz, 8,3 ); 
	cvZero(cpy);
	if(Type<10)//use S.addition
	{
		if(Type>d_-1)
			Type=d_-1;
		sernum=wind.top*Width+wind.left;
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{			
				chariot=&S[tag[sernum]];
				assert(chariot->p==tag[sernum]);
				if((chariot->addition[Type]/(chariot->size)*Range)>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}			
				++sernum;
			}
			sernum+=(Width-mW);
		}
	}
	else
	{
		switch(Type)
		{
		case REGSIZE:
		sernum=wind.top*Width+wind.left;
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{
				chariot=&S[tag[sernum]];
				assert(chariot->p==tag[sernum]);
				if(chariot->size>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}			
				++sernum;
			}
			sernum+=(Width-mW);
		}
		break;
		case PERIM:
		sernum=wind.top*Width+wind.left;
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{
				chariot=&S[tag[sernum]];
				assert(chariot->p==tag[sernum]);
				if(chariot->perim>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}			
				++sernum;
			}
			sernum+=(Width-mW);
		}
		break;
		case CMPCT:
		sernum=wind.top*Width+wind.left;
		
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{
				label=tag[sernum];
				assert(S[label].p==label);
				temp=sqrt(S[label].size*4/3.1416)/S[label].perim;
				
				if(temp>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}			
				++sernum;
			}
			sernum+=(Width-mW);
		}
		break;
		case MAXAL:
		sernum=wind.top*Width+wind.left;
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{
				label=tag[sernum];
				assert(S[label].p==label);
			
				if(S[label].attlist[maxaxislen]>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}
				++sernum;
			}
			sernum+=(Width-mW);
		}
		break;
		case MEANW:
		sernum=wind.top*Width+wind.left;
		for(i=0;i<mH;++i)
		{
			for(j=0;j<mW;++j)
			{
				label=tag[sernum];
				assert(S[label].p==label);
			
				if(S[label].attlist[meanwidth]>=thresh)
				{
				uchar* temp_ptr = &((uchar*)(cpy->imageData + cpy->widthStep*i))[j*3];
				temp_ptr[0]=0;
				temp_ptr[1]=255;
				temp_ptr[2]=0;
				}
				++sernum;
			}
			sernum+=(Width-mW);
		}
		break;
		case MINAL:		
		default:
			AfxMessageBox("this type information not available!");
			break; return;
		}
	}
	cvNamedWindow(wndname, 1 );

	cvShowImage( wndname, cpy );
	cvWaitKey(0);
    cvReleaseImage( &cpy );
	cvDestroyWindow( wndname);	
}
//process large dataset, when image dataset is as large as 1300x1300, createcopy to save seg result
//does not work. then I turn to writeblock using origin dataset, always writing faults, for writeblock
//using new dataset, blocksizes are not compatible. at last, rasterio with create is applied.
/*GDALDataType { 
  GDT_Unknown = 0, GDT_Byte = 1, GDT_UInt16 = 2, GDT_Int16 = 3, 
  GDT_UInt32 = 4, GDT_Int32 = 5, GDT_Float32 = 6, GDT_Float64 = 7, 
  GDT_CInt16 = 8, GDT_CInt32 = 9, GDT_CFloat32 = 10, GDT_CFloat64 = 11, 
  GDT_TypeCount = 12 
}*/
//input  BN band number indicates which band to save, 0 corresponds to 1st band in gdaldataset, and so on.
//if bwarray[i]==0 which means band i+1 in gdaldataset is not processed in S,save operation will be ignored
//11- corresponds to features such as perimeter, bn==-1 to save all the processed mean object layers
//BUG: projection and wkt cannot be written to the output files.
//pathname is the full path for the original image data
void CHC::SaveSeg2(GDALDataset *m_pDataset,CString pathname,int bn)
{
	if(bn>-1&&bn<m_pDataset->GetRasterCount())
	{
		if(bWArray[bn]==0)
		{
			AfxMessageBox("The band is not processed, so not necessary to save!");
			return;
		}
	}
	GDALRasterBand  *poBand=NULL;
	poBand= m_pDataset->GetRasterBand(1);
	GDALDataType dataType=poBand->GetRasterDataType();//数据类型
	int x,y,i,j;
	int d,sernum,label;
	const char *pszFormat = "GTiff";
	GDALDriver *poDriver;
	
	poDriver = GetGDALDriverManager()->GetDriverByName(pszFormat);
	
	if( poDriver == NULL)
	{
		AfxMessageBox("This format is not able to be created!");
		exit( 1 );	
	}
	//using create
	GDALDataset *poDstDS;       
    char **papszOptions = NULL;
	const char*pszDstFilename;
	const char *pszSRS_WKT;
	double adfGeoTransform[6],backup[6] = { 444720, 30, 0, 3751320, 0, -30 };
	float* buf =new float[Width*Height];
	if(bn==-1)
	{
		pszDstFilename="panbands.tif";
		poDstDS = poDriver->Create( pszDstFilename,Width,Height, d_,dataType, 
			papszOptions );
		
		if(CE_None==m_pDataset->GetGeoTransform( adfGeoTransform ))	
			poDstDS->SetGeoTransform( adfGeoTransform );
		else
			poDstDS->SetGeoTransform(backup); 
		
		pszSRS_WKT=m_pDataset->GetProjectionRef();
		poDstDS->SetProjection( pszSRS_WKT );
		CPLFree( (void*)pszSRS_WKT );
		for(d=1;d<d_+1;++d)
		{
			
			poBand = poDstDS->GetRasterBand(d);	
			if (poBand)
			{	
				sernum=0;
				
				for(y=0;y<Height;++y)
				{
					for (x = 0; x < Width; ++x) 
					{
						label=tag[sernum];
						buf[sernum]=(S[label].addition[d-1])/(S[label].size)*Range;							
						
						++sernum;
					}
					
				}
				if (CE_None!=poBand->RasterIO( GF_Write,0,0, Width, Height, buf, Width,Height,GDT_Float32, 0, 0 ))
				{
					AfxMessageBox("error write mpdataset!");
				}
			}
		}
		delete[]buf; 
		GDALClose( (GDALDatasetH) poDstDS );
		return;
	}
	
	CString name;
	name.Format("\\Band%d.tif",bn);
	int pos=pathname.ReverseFind('\\');
	pathname=pathname.Left(pos);
	pathname+=name;
	pszDstFilename=(const char*)pathname;
    poDstDS = poDriver->Create( pszDstFilename,Width,Height, 1,dataType, 
		papszOptions );

	if(CE_None==m_pDataset->GetGeoTransform( adfGeoTransform ))	
		poDstDS->SetGeoTransform( adfGeoTransform );
	else
		poDstDS->SetGeoTransform(backup); 
	
	pszSRS_WKT=m_pDataset->GetProjectionRef();
    poDstDS->SetProjection( pszSRS_WKT );
 
	poBand = poDstDS->GetRasterBand(1);	
	if(!poBand)
	{
		AfxMessageBox("Error fetching the 1st band in poDstDS!");
		return;
	}
	if(bn<10)
	{		
		sernum=0;
		
		for(y=0;y<Height;++y)
		{
			for (x = 0; x < Width; ++x) 
			{
				label=tag[sernum];
				buf[sernum]=(S[label].addition[bn])/(S[label].size)*Range;							
				
				++sernum;
			}
			
		}
		if (CE_None!=poBand->RasterIO( GF_Write,0,0, Width, Height, buf, Width,Height,GDT_Float32, 0, 0 ))
		{
			AfxMessageBox("error write mpdataset!");
		}		
	}
	else
	{
		if((bn>13)&&(S[tag[0]].attlist.size()<4))
		{
			AfxMessageBox("Compute region attributes before saving!");
			return;
		}
		int mH=Height,mW=Width;
		
		sernum=0;
		switch(bn)
		{
		case REGSIZE:
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{	
					
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].size;	
					++sernum;
				}
				
			}
			break;
		case PERIM:
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].perim;	
					++sernum;
				}
				
			}
			break;
		case CMPCT:
			
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=sqrt(S[label].size*4/3.1416)/S[label].perim;			
					++sernum;
				}
			}
			break;
		case MAXAL:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].attlist[maxaxislen];	
					++sernum;
				}
			}
			break;
		case MEANW:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].attlist[meanwidth];	
					++sernum;
				}
			}
			break;
		case MINAL:	
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].attlist[minaxislen];	
					++sernum;
				}
			}
			break;
		case MEANL:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].attlist[meanlen];	
					++sernum;
				}
			}
			break;
		case ELONG:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].attlist[maxaxislen]/S[label].attlist[minaxislen];	
					++sernum;
				}
			}
			break;
		case MLWR:
			for(i=0;i<mH;++i)
			{
				for(j=0;j<mW;++j)
				{
					label=tag[sernum];
					assert(S[label].p==label);
					buf[sernum]=S[label].attlist[meanlen]/S[label].attlist[meanwidth];	
					++sernum;
				}
			}
			break;
		default:
			AfxMessageBox("this type information not available!");
			break;
		}
		
		if (CE_None!=poBand->RasterIO( GF_Write,0,0, Width, Height, buf, Width,Height,GDT_Float32, 0, 0 ))
		{
			AfxMessageBox("error write mpdataset!");
		}
		
	}
	delete[]buf; 
//	CPLFree( (void*)pszSRS_WKT );
    GDALClose( (GDALDatasetH) poDstDS );	
}

void CHC::GetPreviewMask(CRect &rect)
{
	if(Height==0)
	{
		AfxMessageBox("CHC not yet initiated!");return;
	}
	if(Height<=256||Width<=256)
	{
		rect.top=0;
		rect.bottom=Height;
		rect.left=0;
		rect.right=Width;
		return;
	}
	rect.left=(Width-256)/2+1;
	rect.right=rect.left+256;
	rect.top=(Height-256)/2+1;
	rect.bottom=rect.top+256;
}
