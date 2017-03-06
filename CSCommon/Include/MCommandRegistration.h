#pragma once

#include "MSharedCommandTable.h"
#include "MCommandManager.h"

// Introduces some helper functions into the scope.
// C(ID, Name, Description, Flags) -- Adds a new command description.
// P(Type, Description) -- Adds a new parameter to a command description most recently added with C.
// CA(Name, Text) -- Adds a command alias.
// P_MINMAX(Type, Description, Min, Max) -- Adds a new parameter, except with the min and max condition.
// IsTypeAnyOf(SharedTypes...) -- Checks if the shared type is any of the arguments.
#define BEGIN_CMD_DESC(CommandManager, SharedType)\
	MCommandDesc* pCD4m = nullptr;\
	\
	auto C = [&](int ID, const char* Name, const char* Description, int Flags) {\
		pCD4m = new MCommandDesc{ ID, Name, Description, Flags };\
		CommandManager->AddCommandDesc(pCD4m);\
	};\
	\
	auto P = [&](MCommandParameterType Type, const char* Description) {\
		pCD4m->AddParamDesc(new MCommandParameterDesc{ Type, Description });\
	};\
	\
	auto CA = [&](const char* Name, const char* Text) {\
		CommandManager->AddAlias(Name, Text);\
	};\
	\
	auto P_MINMAX = [&](MCommandParameterType Type, const char* Description, int Min, int Max) {\
		auto* pNewDesc = new MCommandParameterDesc{ Type, Description };\
		pNewDesc->AddCondition(new MCommandParamConditionMinMax{ Min, Max });\
		pCD4m->AddParamDesc(pNewDesc);\
	};\
	\
	auto IsTypeAnyOf = [&](auto&&... Vals) {\
		u32 Sum = 0;\
		for (auto&& Val : { Vals... })\
			Sum |= Val;\
		return (SharedType & Sum) != 0;\
	};\
	\
	using namespace MSharedCommandType
