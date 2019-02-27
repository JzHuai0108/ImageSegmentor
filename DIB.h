/*/////////////////////////////////////////////////////////////
这是一个 DIB位图操作类，用下面几个内部数据来保存 DIB位图信息：
	BYTE *m_lpBits; ---------- 像素
	BYTE *m_lpPalette; ------- 调色板；对于24位位图，此指针是NULL
	int m_nWidth; ------------ 位图宽度
	int m_nHeight; ----------- 位图高度
	int m_nBitCount; --------- 每像素所用的位数

读入位图的函数有：
	BOOL LoadFromFile(LPCTSTR filename);
	        ------ 从 BMP文件中读入图像
	BOOL ImportPcx(char *fn);
	        ------ 从 PCX文件中读入图像；即对 pcx格式解码

对DIB 位图处理的函数有：
	BOOL Grey();			//转换成灰度图像
	BOOL HFlip();			//图像横向翻转
	BOOL VFlip();			//图像纵向翻转
	BOOL Rotate();			//顺时针旋转90度
	BOOL Negative();		//转成负像
/*////////////////////////////////////////////////////////////*/

#ifndef __DIB_H
#define __DIB_H
//#include <windows.h>
#include <afx.h>
//#include <stdlib.h>


#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
// 计算图像每行象素所占的字节数目 
#define BYTE_PER_LINE(w, c) ((((w)*(c)+31)/32)*4)
// 根据表示颜色的位数来确定能表示的颜色数
// 如果是24位颜色，为0；如果是8位颜色，为256；如果是4位颜色，为16
#define PALETTESIZE(b) (((b)==8)?256:(((b)==4)?16:0))

class CDIB
{
//	friend class CHC;
public:	
	BYTE *m_lpBits;		// 图象数据指针
	BYTE *bpBits;//back up image data
	BYTE *m_lpPalette;	// 调色板指针
	int m_nBitCount;	// 图像中表示每像素所用的位数
	int m_nWidth;		// 图象的宽度，单位是象素
	int m_nHeight;		// 图象的高度，单位是象素
	int m_nColors;//number of colors in palette 
	int m_nPlanes; //
public:	
	void SetPixelColor2(int x,int y,BYTE color);
	void SetPixelIndex(int x,int y,BYTE index);
	BYTE GetNearestIndex(RGBQUAD color);
	void SetPixelColor(int x,int y,RGBQUAD color);
	bool CreateDIBFromBits(int nWidth,int nHeight,BYTE * buf,int=24);
	RGBQUAD GetPaletteColor(BYTE idx);
	BYTE GetPixelIndex(int x,int y);
	BYTE GetPixelColor2(int x,int y);
	RGBQUAD GetPixelColor(int x,int y);
	int CreateDIBFromIPL(int, int ,char*,int bits);
	int LoadDIBToIPL(char*imageData, int bits);
	int LoadDIBToBuf(BYTE* buf);
	int LoadDIBToBuf(float *buf);
	bool SaveToFile(const CString&,CString&);
	int lookregion(int label,int*tag, CRect rect,int area);
	void ShowReg(int*tag=0,int step=32);
	// 将Dib位图数据填充到到CDib类的数据结构
	BOOL CreateDIB(int r, int g, int b,const CString&);
	BOOL LoadDib(BYTE* lpDib);
	// 将CDib类的数据结构中的数据转换为Dib位图格式数据
	BOOL SaveToDib(BYTE *lpDib);
	//void PlotRegions();
	BOOL SaveEdge(LPCTSTR fn,BYTE*EM);
	void LookRegions(int*tag,int option=0);
	void Origin();

//	void MapSegImage();
	void MakeGrayPalette(int BitCount);
	void histovector(float*hive);
	BOOL InteEqualize();
	CDIB();		// CDib类的构造函数

	CDIB( const char *pszFileName);
	~CDIB();	// CDib类的析构函数

	// 清除CDib类中有关位图的数据
	void Clear();
	// 检测位图数据是否空（空图像）
	// 获取图像宽度
	int GetWidth()	{return m_nWidth;}
	// 获取图像高度
	int GetHeight()	{return m_nHeight;}
	int GetLength()	{return m_nHeight*m_nWidth;}
	// 获取图像中表示每像素所用的位数
	int GetBitCount()	{return m_nBitCount;}
	int GetWidthInBytes( int nBits, int nWidth );

	// 根据位图数据画出位图
	int Stretch(HDC,int,int,int,int,UINT,DWORD);
	// 从位图文件中读取位图数据（根据打开的文件指针）
	BOOL LoadFromFile(CFile *pf);
	// 从位图文件中读取位图数据（根据文件名）

	BOOL LoadFromFile(LPCTSTR filename);
	// 将位图数据保存到位图文件（根据打开的文件指针）
	BOOL SaveToFile(CFile *pf);
	// 将位图数据保存到位图文件（根据文件名）
	BOOL SaveToFile(LPCTSTR fn);
	// 按照指定文件名读入PCX格式的图像文件的数据

	// 将图像数据复制到剪贴板
	BOOL Copy();
	// 复制剪贴板中的数据
	BOOL Paste();

	// 将图像转换为灰度图像
	BOOL Grey();
	// 横向翻转图像
	BOOL HFlip();
	// 纵向翻转图像	
	BOOL VFlip();
	// 将图像顺时针旋转90度
	BOOL Rotate();
	// 将图像转换为负像
	BOOL Negative();
private:
	CDIB(const CDIB&);
	CDIB& operator=(const CDIB&);
};

void ImageProps(int&, CString&,const CString&fn);
//input:allocated EM 
//output: EM initiated with data for edge detection
BYTE* GetEM(const CString&fn, BYTE *EM,int eB);
#endif //dib.h
