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

// NP.h: interface for the NP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_)
#define AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class NP  
{
public:

	int rInd;//region index in line with tag[]
    int pInd;//pair index in A
	
	NP(int r=-1,int p=0):rInd(r),pInd(p){}
	NP(const NP&pass):rInd(pass.rInd),pInd(pass.pInd){}
	NP&operator=(const NP&pass){
		if(this==&pass)
			return*this;
		rInd=pass.rInd;
		pInd=pass.pInd;
		return *this;
	}
	virtual ~NP(){}
	bool operator ==(const NP &R)const {return rInd==R.rInd;}
	bool operator <(const NP &R)const {return rInd<R.rInd;}
};

#endif // !defined(AFX_NP_H__FF460978_0D64_41EA_937F_E28DDB0AA543__INCLUDED_)
