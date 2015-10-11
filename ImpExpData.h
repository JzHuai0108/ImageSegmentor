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
