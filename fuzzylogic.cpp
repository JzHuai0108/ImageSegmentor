#include"stdafx.h"
#include"fuzzylogic.h"
float LinearGT(float d,float e, float f, float x)
{
	if(x<=d)
		return 0.f;
	else if(x>=f)
		return 1.0f;
	else if(x>=e)
		return 0.5f+0.5f*(x-e)/(f-e);
	else return (x-d)/(e-d);
}
float LinearLT(float a,float b, float c, float x)
{
	if(x<=a)
		return 1.0f;
	else if(x>=c)
		return 0.f;
	else if(x>=b)
		return 0.5f-0.5f*(x-b)/(c-b);
	else return 0.5f+0.5f*(b-x)/(b-a);
}
float LinearIB(float a,float b, float c, float d, float e, float f, float x)
{
	if(x>=f||x<=a)
		return 0.f;
	else if(x>=c&&x<=d)
		return 1.0f;
	else if(x>a&&x<c)
		return LinearGT(a,b,c,x);
	else return LinearLT(d,e,f,x);

}