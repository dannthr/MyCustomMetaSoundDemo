// Copyright Dan Reynolds, 2026

#pragma once

#include "Modules/ModuleManager.h"

class FMyMetaSoundModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	bool bMetaSoundsRegistered = false;
};
