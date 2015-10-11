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
