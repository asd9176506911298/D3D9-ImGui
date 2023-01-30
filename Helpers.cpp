#include "Helpers.h"
#include <detours.h>

void Helpers::HookFunction(PVOID* oFunction, PVOID Function)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(oFunction, Function);
    DetourTransactionCommit();
}

void Helpers::UnHookFunction(PVOID* oFunction, PVOID Function)
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(oFunction, Function);
    DetourTransactionCommit();
}