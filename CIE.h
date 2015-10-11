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

#pragma once

class CCIE
{
public:

	CCIE(void);
	~CCIE(void);
	void HSV2RGB_Cylinder(float h,float s,float v,int &r,int &g,int &b);
	void RGB2HSV_Cylinder(int r,int g,int b,float &h,float &s,float &v);
	void HSV2RGB_Sphere(float h,float s,float v,int &ir,int &ig,int &ib);
	void RGB2HSV_Sphere(int r,int g,int b,float &h,float &s,float &v);
	void Lab2LHC(float L, float a,float b,float &c,float &h);
	void RGB2Luv(float r, float g, float b, float &L, float &u, float &v);
	void Luv2RGB(float L, float u, float v, int &r, int &g, int &b);
	
	void Luv2XYZ(float L, float u, float v, float &x, float &y, float &z);
	void XYZ2RGB(float x, float y, float z, int &r, int &g, int &b);
	void XYZ2Luv(float x, float y, float z, float &L, float &u, float &v);
	void RGB2XYZ(float r, float g, float b, float &x, float &y, float &z);
};
