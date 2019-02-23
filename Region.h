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

// Region.h: interface for the Region class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_REGION_H__5C2B5D5D_02AB_4004_99C1_3F7AE3C28297__INCLUDED_)
#define AFX_REGION_H__5C2B5D5D_02AB_4004_99C1_3F7AE3C28297__INCLUDED_
//#pragma warning (disable:4786)

#include "NP.h"
#include<vector> 
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
typedef std::vector<NP> NPL;

typedef struct tPair
{
	int r[2];
	float t;
	unsigned int bl;//boundary length between the pair region
	//	float es;//edge strength, note that when a pixel belong to P abuts two neighbors which merge to Q later,
	//then the edge strength of this pixel is added twice in the edge strength between region P and Q.
	//since intuitively, the chance the pixel is an edge point is bigger.
	tPair():t(0.f),bl(0){}
	tPair(const tPair&pass):t(pass.t),bl(pass.bl){
		r[0]=pass.r[0];
		r[1]=pass.r[1];
	}
	tPair& operator=(const tPair&agn){
		if(&agn==this) 
			return*this;
		r[0]=agn.r[0];
		r[1]=agn.r[1];
		t=agn.t;
		bl=agn.bl;
		return *this;
	}
} tPair;


typedef struct exRegion
{
public:
	int label;//indicate the father pixel index of this region in tagMatrix
	float* attList;//attribute list according to prop
	bool isVisited;
	exRegion():label(-1),attList(NULL),isVisited(false){}
	exRegion(const exRegion&cpy):label(cpy.label),attList(cpy.attList),isVisited(cpy.isVisited){}
	exRegion&operator=(const exRegion&agn){
		if(&agn==this)
			return*this;
		label=agn.label;
		attList=agn.attList;
		isVisited=agn.isVisited;
		return *this;
	}
	bool operator<(const exRegion&b) const
	{
		return label<b.label;
	}
	bool operator==(const exRegion&b) const
	{
		return label==b.label;
	}
}exRegion;

enum PROP
{
	centerX=0,//x coordinate of bounding box center
	centerY,//y coord
	minRectWid,//bounding box width
	minRectHeg,//bounding box height
	boxAngle,//bounding box angle
	eigRatio,//the major eigenvalue of the point coordinate covariance matrix to the minor one
	meanThick,//the average width of a region computed from erosion
	changeRatio,//the rate of changed pixels in one region
	shadeRatio,//the ratio of shade pixels in one region
	neiShadeRatio,//the maximum ratio of shade pixels in one regions' neighboring regions
};
//texture intensity color
enum TIC//terms as defined in Vincent Tao's integrating intensity texture color
{
	PC1=0,//for diff in image 1
	PC2,//for diff in image 2
	ROUC,//for color hue and saturation
	GMEAN1,//MEAN INTENSITY FOR IMAGE 1 IN REGION 
	GMEAN2,//FOR image 2
	ROUI,//for intensity rou
	PT1,//for var threshold in image 1
	PT2,//in image 2
	ROUT//for LBP AND VAR (texture)
};

class Region  
{
public:

	//void DeleteNeighbor(const int tag);
	unsigned int size; 
	float *addition; // each entry is the sum of intensity values in one
	    // channel of all pixels in the region normalized by the intensity
		// range limit, e.g., 256
	float *sSum; // each entry is the sum of squares of intensity values in 
	    // one channel of all pixels in the region normalized by the squared
		// intensity range limit, e.g., 256^2
	NPL NPList;
	int perim;
	int bestp;// best merge candidate pair index
	int p;//parent region index

/* The heterogeneity of the region defined following equations (2-6) of 
Benz, U. C., Hofmann, P., Willhauck, G., Lingenfelder, I., & Heynen, M. (2004).
 Multi-resolution, object-oriented fuzzy analysis of remote sensing data for 
 GIS-ready information. ISPRS Journal of photogrammetry and remote sensing, 
 58(3-4), 239-258." 
For clarity, the exact equation is given below.
$ heterogeneity = w_{color} h_{color} + (1 - w_{color}) h_{shape} \\
h_{color} = \sum_{c=1}^{d} w_c \sqrt{\frac{1}{n-1}\left( \sum^n I_{c,j}^2 - 
  \frac{(\sum^n I_{c,j})^2}{n} \right )} \\
h_{shape} = w_{compt} h_{compt} +  (1 - w_{compt}) h_{smooth} \\
h_{smooth} = n l/b \\
h_{compt} = l\sqrt{n} $ 
where d is the number of channels of the image, 
 n the number of pixels in the region, l the perimeter of the region, 
 b the perimeter of the region's bounding box
*/
	float interdif; 


	bool isChecked;
	CRect *norbox;//regular box bounding the region
public:
	/** compute the heterogeneity of the region
	 * param: d number of channels
	 * param: wcolor weight of color heterogeneity, $w_{color}$ in equation (2)
	 *  of Benz 04, note it is not $w_c$
	 * param: wp weight of compactness heterogeneity, implying that 
	 *  smoothness heterogeneity weight == 1 - wp
	 * param: range is the right range limit of the intensity values, a power 
	 *  of 2. For a intensity range [0, 255], it ought to be 256
	 * return the computed heterogeneity combining color and shape info
	 */
	float InterDiff(int d=1,float wcolor=0.9,float wp=0.5,float range=256.f);
	Region(unsigned int sz=0):size(sz),addition(NULL),sSum(NULL),perim(0),
		bestp(-1),p(-1),interdif(0),isChecked(false),norbox(NULL){}
	Region(const Region&cpy):size(cpy.size),addition(cpy.addition),sSum(cpy.sSum),perim(cpy.perim),
		bestp(cpy.bestp),p(cpy.p),interdif(cpy.interdif),isChecked(cpy.isChecked),norbox(cpy.norbox){}
	Region&operator=(const Region&cpy){
		if(&cpy==this)
			return *this;
		size=cpy.size;
		addition=cpy.addition;
		sSum=cpy.sSum;
		perim=cpy.perim;
		bestp=cpy.bestp;
		p=cpy.p;
		interdif=cpy.interdif;
		isChecked=cpy.isChecked;
		norbox=cpy.norbox;
		return *this;
	}
	
	~Region();
}; 
//Actually,the result of compute dist is the internaldif(a&b)


#endif // !defined(AFX_REGION_H__5C2B5D5D_02AB_4004_99C1_3F7AE3C28297__INCLUDED_)
