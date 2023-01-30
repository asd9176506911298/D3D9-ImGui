#pragma once
#include <Windows.h>
class Helpers
{
public:
	static void HookFunction(PVOID* oFunction, PVOID Function);
	static void UnHookFunction(PVOID* oFunction, PVOID Function);
};

