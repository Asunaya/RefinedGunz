#ifndef MRESOURCE_H
#define MRESOURCE_H

#pragma warning(disable: 4786)

#include <map>
#include <string>
#include <list>

using namespace std;

#include "MTypes.h"

// 위젯 정보
struct MWIDGETINFO{
	char	szWidgetClass[256];
	char	szWidgetName[256];
	int		nResourceID;
};

// 위젯 리소스
struct MWIDGETRESOURCE{
	MRECT				Bounds;		// 경계
	bool				bBounds;	// 경계가 바뀌었는가?

	class MWIDGETINFOLIST : public list<MWIDGETINFO*>{
	public:
		virtual ~MWIDGETINFOLIST(void){
			// Delete Automatically
			while(empty()==false){
				delete (*begin());
				erase(begin());
			}
		}
	} Children;	// 자식 위젯 리스트
};

class MResourceMap : public map<string, MWIDGETRESOURCE*>{
public:
	virtual ~MResourceMap(void){
		// Delete Automatically
		while(empty()==false){
			delete (*begin()).second;
			erase(begin());
		}
	}
};


#endif