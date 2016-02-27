#pragma once
#include <vector>
#undef GetObject // DURRRRRRRRRR windows headers

class FuncList {
	class DummyClass { };

	struct FuncBase {
		virtual ~FuncBase() { };

		virtual void *GetObject() = 0;
		virtual void *GetAddress() = 0;
		virtual bool Call() = 0;
		virtual bool Call(void *pvParam) = 0;
	};

	struct Func : public FuncBase {
		union {
			void *pvFunc;
			bool(*pFunc)();
			bool(*pArgFunc)(void *);
		};

		virtual void *GetObject();
		virtual void *GetAddress();
		virtual bool Call();
		virtual bool Call(void *pvParam);
	};

	struct Method : public FuncBase {
		DummyClass *pObject;
		union {
			void *pvMethod;
			bool(__thiscall DummyClass:: *pMethod)();
			bool(__thiscall DummyClass:: *pArgMethod)(void *);
		};

		virtual void *GetObject();
		virtual void *GetAddress();
		virtual bool Call();
		virtual bool Call(void *pvParam);
	};

	struct StdMethod : public FuncBase {
		DummyClass *pObject;
		union {
			void *pvMethod;
			bool(__stdcall DummyClass:: *pMethod)();
			bool(__stdcall DummyClass:: *pArgMethod)(void *);
		};

		virtual void *GetObject();
		virtual void *GetAddress();
		virtual bool Call();
		virtual bool Call(void *pvParam);
	};

	typedef void(*VoidFunc)();
	typedef void(*ArgFunc)(void *);
	typedef bool(*BoolFunc)();
	typedef bool(__thiscall DummyClass:: *VoidMethod)();
	typedef bool(__stdcall DummyClass:: *VoidStdMethod)();
	typedef bool(__thiscall DummyClass:: *ArgMethod)(void *);

private:
	std::vector<FuncBase *> vFuncs;

public:
	template<class F>
	void AddFunc(F pFunc);
	template<class T, class F>
	void AddMethod(T *pObject, F pMethod);
	template<class T, class F>
	void AddStdMethod(T *pObject, F pMethod);

	template<class F>
	void RemoveFunc(F pFunc);
	template<class F>
	void RemoveMethod(void *pObject, F pMethod);

	void CallAll();
	void CallAll(void *pvParam);

	bool Call(unsigned long i);
	unsigned long Size() { return vFuncs.size(); }
};

template<class F>
void FuncList::AddFunc(F pFunc){
	Func *pf = new Func;
	pf->pFunc = (BoolFunc)pFunc;
	vFuncs.push_back(pf);
}

template<class T, class F>
void FuncList::AddMethod(T *pObject, F pMethod){
	Method *pm = new Method;
	pm->pObject = (DummyClass *)pObject;
	pm->pMethod = (VoidMethod)pMethod;
	vFuncs.push_back(pm);
}

template<class T, class F>
void FuncList::AddStdMethod(T *pObject, F pMethod){
	StdMethod *psm = new StdMethod;
	psm->pObject = (DummyClass *)pObject;
	psm->pMethod = (VoidStdMethod)pMethod;
	vFuncs.push_back(psm);
}

template<class F>
void FuncList::RemoveFunc(F pFunc){
	for (unsigned long i = 0; i < vFuncs.size(); i++)
	if (vFuncs.at(i)->GetAddress() == (void *)pFunc){
		delete vFuncs.at(i);
		vFuncs.erase(vFuncs.begin() + i);
		break;
	}
}

template<class T>
union MethodPtr{
	void *pv;
	unsigned long dw;
	T pMethod;

	MethodPtr(T mptr){
		pMethod = mptr;
	}
};

template<class T>
void *GetMethodPtr(T mptr){
	return MethodPtr<T>(mptr).pv;
}

template<class F>
void FuncList::RemoveMethod(void *pObject, F pMethod){
	for (unsigned long i = 0; i < vFuncs.size(); i++)
	if (vFuncs.at(i)->GetObject() == pObject && vFuncs.at(i)->GetAddress() == MethodPtr<F>(pMethod).pv){
		delete vFuncs.at(i);
		vFuncs.erase(vFuncs.begin() + i);
		break;
	}
}

/*#include <vector>
#include <functional>

template<typename T>
class FuncList;

template<typename Ret, typename ... Args>
class FuncList<Ret(Args...)>
{
private:
	std::vector<std::function<Ret(Args...)>> vFuncs;

public:
	void AddFunction(std::function<Ret(Args...)> a) {
		vFuncs.push_back(a);
	}

	template<typename Function>
	void AddStaticFunction(Function pfn) {
		vFuncs.push_back(pfn);
	}

	template<typename Object, typename Method>
	void AddMethod(Object obj, Method pfn) {
		vFuncs.push_back(std::bind(pfn, obj, Args...)); // 2x virtual dispatch uwu
	}

	template<typename Object, typename Method>
	void RemoveMethod(Object obj, Method pfn) {
		for (auto it = vFuncs.begin(); it != vFuncs.end(); it++)
		{
			if (it->target() == pfn)
			{
				vFuncs.erase(it);
			}
		}
	}

	void operator()(Args...) {
		for (auto &Func : vFuncs)
		{
			Func(Args...);
		}
	}
};*/