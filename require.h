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

#ifndef REQUIRE_H
#define REQUIRE_H
#include <afx.h>
inline void require(bool requirement, const CString& msg = "Requirement failed"){
	if (!requirement) {
		char bane[100]={0};
		sprintf(bane,"%s\n",msg);
		AfxMessageBox(bane);
		
		exit(1);
	}
}
inline void assure(void* in,const CString& filename = "") {
	if(!in) {
		char bane[100]={0};
		sprintf(bane,"Could not open file %s\n",filename);
		AfxMessageBox(bane);
		exit(1);
	}
}
#endif
