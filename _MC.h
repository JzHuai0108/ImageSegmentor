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

// _MC.h: interface for the _MC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_)
#define AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class _MC  
{
public:

	int pInd;
	double t;

	_MC(int p=-1,double c=0):pInd(p),t(c){}
	_MC(const _MC&pass):pInd(pass.pInd),t(pass.t){}
	_MC&operator=(const _MC&pass){
		if(this==&pass)
			return*this;
		pInd=pass.pInd;
		t=pass.t;
		return*this;
	}
	virtual ~_MC(){}
	bool operator<(const _MC &b) const{		
		if(t==b.t)
			return pInd<b.pInd;
		else 
			return t<b.t;
	}
};

#endif // !defined(AFX__MC_H__864D4BE1_6A01_4531_BB41_D332A9EF68BB__INCLUDED_)
