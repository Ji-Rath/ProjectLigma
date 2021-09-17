#include "FogOfWar.h"

DEFINE_LOG_CATEGORY(FogOfWar);

#define LOCTEXT_NAMESPACE "FFogOfWar"

void FFogOfWar::StartupModule()
{
	UE_LOG(FogOfWar, Warning, TEXT("FogOfWar module has been loaded"));
}

void FFogOfWar::ShutdownModule()
{
	UE_LOG(FogOfWar, Warning, TEXT("FogOfWar module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFogOfWar, FogOfWar)