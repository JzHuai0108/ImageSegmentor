#include<vector>

//when using the bestp, the coverage limit becomes of not much use. 
//the parameter gap and gThresh has some effect on outcome, the parameters for segmentation 
//have not yet been studied.
typedef struct Parcel{
	static int dim;
	int pIndex;//the index of the first pixel belong to this parcel in tagarray, 
	int size;//area
	CRect boundBox;
	float *sum1;//sum of spectral intensity
	float *sum2;//sum of squares of spectral intensity
	Parcel():pIndex(-1),size(0),sum1(0),sum2(0){}
	Parcel(const Parcel&pass):pIndex(pass.pIndex),size(pass.size),sum1(pass.sum1),sum2(pass.sum2){}
	Parcel&operator=(const Parcel&pass){
		if(this==&pass)
			return*this;
		pIndex=pass.pIndex;
		size=pass.size;
		sum1=pass.sum1;
		sum2=pass.sum2;
		return*this;
	}

}Parcel;
typedef struct RefPair{
	int bestp;//parcel label of the biggest intersection parcel in the other segmentation
	//if bestp==-1, it means the parcel cannot be used, especially to avoid duplicated checking of region pairs
	int section;//parcel intersection area
	float sDist;//spectral difference
	float gDist;//geometric difference
	RefPair():bestp(-1),section(0),sDist(-0.f),gDist(-0.f) {}
	RefPair(const RefPair&pass):bestp(pass.bestp),section(pass.section),
		sDist(pass.sDist),gDist(pass.gDist) {}
	RefPair& operator=(const RefPair&right){
		if(this==&right)
			return *this;
		bestp=right.bestp;
		section=right.section;
		sDist=right.sDist;
		gDist=right.gDist;
		return *this;
	}
}RefPair;//each refence pair corresponds to one parcel

class ChangeDetector{
	std::vector<RefPair> refList1;//each refpair is associated with the parcel in segmentation 1 of the same index in vector
	std::vector<RefPair> refList2;
	ChangeDetector(const ChangeDetector&);
	ChangeDetector& operator=(const ChangeDetector&);
public:
	void PrepLabel(const CString&tagf1,const CString &tagf2);
	void PrepRegList(const CString&seg1,const CString&seg2);
	int *tag1;//label matrix for segmentation 1
	int *tag2;
	BYTE *buf;//result with 0 indicating unchanged area
	float *sMean1;//spectral mean,
	float *sMean2;//spectral mean 
	float *sVar1;//spectral variance,
	float *sVar2;
	int dim;//dimension of mean and variance, i.e., band number
	int width;
	int height;
	std::vector<Parcel> regList1;//segmentation 1, tag[regList1[i].pIndex]==i holds
	std::vector<Parcel> regList2;

	ChangeDetector();
	~ChangeDetector();
	void InitPairs();//for each pixel in segmentation 1, floodfill with matching pixel in segmentation 2
	void RadioDiff();//spectral difference
	void MorphDiff(float,float);//geometric difference
	void Detect(float,float);//for each region pair in pairList, decide the no changed one
	void SaveChange(const CString&,const CString&)const;//save the change detection array
};
