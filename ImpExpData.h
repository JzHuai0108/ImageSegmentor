
#include "stdafx.h"   
#include <iostream>   
#include <fstream>
#include <assert.h>

template <class T>
int ImportData(const char* pathName,T*label,int count)
{
	ifstream inp(pathName);
	if(!inp.is_open())
	{
		return 0;
	}
	int i,j;
	inp>>i>>j;

	assert(i*j==count);
	for(i=0;i<count;++i)
		inp>>label[i];
	inp.close();
	return 1;
}
//store data to a text file with each datum of dimension dimen, 
//make sure each column is a std::vector of dimen, return size of exported data
template <class T>
void Export(T *data,int dimen, int count,const char* fn)
{	
	ofstream out(fn,ios::out|ios::trunc);
	out<<dimen<<"\t"<<count<<endl;
	T*pf=data;
	for(int i=0;i<count;++i)
	{
		for(int j=0;j<dimen;++j)
		{
			out<<*pf<<"\t";
			++pf;
		}
		out<<endl;
	}
	out<<endl;
}