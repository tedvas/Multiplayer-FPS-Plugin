// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#define SURFACE_HEAD SurfaceType1
#define SURFACE_CHEST SurfaceType2
#define SURFACE_TORSO SurfaceType3
#define SURFACE_LEG SurfaceType4

class FMultiplayerFPSModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
