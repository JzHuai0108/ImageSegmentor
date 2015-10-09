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
