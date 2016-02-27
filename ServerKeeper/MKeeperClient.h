#pragma once

#include <map>
using namespace std;
#include "MUID.h"
#include "MObject.h"



class MKeeperClient : public MObject {
protected:

public:
	MKeeperClient(const MUID& uid) : MObject(uid) { 
	}
	virtual ~MKeeperClient() {};
};

class MKeeperClientMap : public map<MUID, MKeeperClient*>{};


