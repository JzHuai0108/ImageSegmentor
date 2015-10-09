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