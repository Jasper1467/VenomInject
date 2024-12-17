#include "Injector.h"

#include "LoadLibraryA.h"
#include "ManualMapping.h"

#include <stdexcept>

void Injector::InjectInternal(HANDLE hProcess, const char* szDllPath, Method_e nInjectionMethod)
{
    switch (nInjectionMethod)
    {
    case Injector::METHOD_LOADLIBRARYA:
        Injector::LoadLibraryA::Inject(hProcess, szDllPath);
        break;
    case Injector::METHOD_MANUALMAPPING:
        Injector::ManualMapping::Inject(hProcess, szDllPath);
        break;
    default:
        throw std::runtime_error("Invalid injection method!");
        break;
    }
}

void Injector::Inject(HANDLE hProcess, std::vector<std::string> szDllPaths, Method_e nInjectionMethod)
{
	for (const auto& szDllPath : szDllPaths)
		InjectInternal(hProcess, szDllPath.c_str(), nInjectionMethod);
}