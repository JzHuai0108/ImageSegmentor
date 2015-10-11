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

#include<vector>
enum SynSegWay{ConnectedMerge, DiscreteMerge, SimuSeg};
class MultiDiff{
private:
	MultiDiff(const MultiDiff&);
public:
	int GetComps() const{	return regCount[2*storey];}
	void CalcDist(std::vector<int>&,float*);
	void Reallocate(int);
	void EvalFeat(const char* dataset, const char* bale,std::vector<float>&);
	void Init(int l, float min, float max);

	MultiDiff(const CString&,const SynSegWay&opt=SimuSeg);	
	~MultiDiff();
	int SynMultiSeg(const CString& fn1,const CString &fn2, std::vector<float>&);

	void CompFeat(const CString&,std::vector<float>&,float *storage,int);
	void CreateChangeMask(int*label);
	int EuclidDist(float*feat1,float*feat2,const int count,const int dim, float* points)const;
	void SaveChange(const CString&fn, const CString&dst) const;

	std::vector<int> index0;//the parent index for each region
	int*tag;//label matrix for the merged segmentation, tag[i] identify the region index to which pixel i belongs
	std::vector<CRect> grid;//each region's bounding box
	int storey;//levels of segmentation
	int*regCount;//region count in segmentation 1,2 and merged one in storey levels
	int width, height;//image size
	int bandCount;//number of bands
	float mindiff;
	float maxdiff;//max and min value for multilevel segmentation
	const CString dirName;//the directory of the images, added //
	const SynSegWay method;//the method to segment two time images
	BYTE*buf;//store the change results
};
int ReadFeat(const char *ft1, const char *ft2,float*feat1,float*feat2,int count,int dim,std::vector<int>&dimUse);
int ReadSeg(LPCTSTR fn1,LPCTSTR fn2,std::vector<CRect>&grid,int*tagArray,std::vector<int>&index=std::vector<int>(0));
