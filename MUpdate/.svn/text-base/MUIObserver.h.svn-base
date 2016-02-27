#pragma once


enum MUICATEGORY {
	MUICATEGORY_INFO,
	MUICATEGORY_FILE,
	MUICATEGORY_END
};


class MUIObserver {
public:
	MUIObserver()			{}
	virtual ~MUIObserver()	{}

	virtual void UpdateMsg(int nCategory, char* pszMsg) = 0;
	virtual void UpdateProgress(char* pszFileName, unsigned long dwRead, unsigned long nTransSumBytes, unsigned long nMaxBytes) = 0;
};
