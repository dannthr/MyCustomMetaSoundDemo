// Copyright Dan Reynolds, 2026

#include "MyMetaSound.h"
#include "Interfaces/IPluginManager.h"
#include "MetasoundDataTypeRegistrationMacro.h"


#define LOCTEXT_NAMESPACE "FMyMetaSoundModule"

void FMyMetaSoundModule::StartupModule()
{
	// Early out startup if Metasound plugin is not enabled
	if (!IPluginManager::Get().FindEnabledPlugin(TEXT("Metasound")).IsValid())
	{
		return;
	}
	
	// Register MetaSound
	METASOUND_REGISTER_ITEMS_IN_MODULE
	
	// Flag MetaSound as registered
	bMetaSoundsRegistered = true;
}

void FMyMetaSoundModule::ShutdownModule()
{
	// Only unregister if needed
	if (!bMetaSoundsRegistered)
	{
		METASOUND_UNREGISTER_ITEMS_IN_MODULE
	}
}

#undef LOCTEXT_NAMESPACE

// This macro registers all queued MetaSound nodes
METASOUND_IMPLEMENT_MODULE_REGISTRATION_LIST
IMPLEMENT_MODULE(FMyMetaSoundModule, MyMetaSound)