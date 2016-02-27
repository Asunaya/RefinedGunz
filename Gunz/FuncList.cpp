#include "stdafx.h"
#include "funclist.h"

void *FuncList::Func::GetObject(){
	return 0;
}

void *FuncList::Func::GetAddress(){
	return pvFunc;
}

bool FuncList::Func::Call(){
	return pFunc();
}

bool FuncList::Func::Call(void *pvParam){
	return pArgFunc(pvParam);
}

void *FuncList::Method::GetObject(){
	return (void *)pObject;
}

void *FuncList::Method::GetAddress(){
	return pvMethod;
}

bool FuncList::Method::Call(){
	return (pObject->*pMethod)();
}

bool FuncList::Method::Call(void *pvParam){
	return (pObject->*pArgMethod)(pvParam);
}

void *FuncList::StdMethod::GetObject(){
	return (void *)pObject;
}

void *FuncList::StdMethod::GetAddress(){
	return pvMethod;
}

bool FuncList::StdMethod::Call(){
	return (pObject->*pMethod)();
}

bool FuncList::StdMethod::Call(void *pvParam){
	return (pObject->*pArgMethod)(pvParam);
}

void FuncList::CallAll(){
	for (unsigned long i = 0; i < vFuncs.size(); i++)
		vFuncs.at(i)->Call();
}

void FuncList::CallAll(void *pvParam){
	for (unsigned long i = 0; i < vFuncs.size(); i++)
		vFuncs.at(i)->Call(pvParam);
}

bool FuncList::Call(unsigned long i){
	return vFuncs.at(i)->Call();
}