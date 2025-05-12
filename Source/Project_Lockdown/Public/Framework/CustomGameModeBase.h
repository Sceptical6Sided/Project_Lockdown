// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameLiftServerSDK.h"
#include "CustomGameModeBase.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(GameServerLog, Log, All);

UCLASS()
class PROJECT_LOCKDOWN_API ACustomGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	ACustomGameModeBase();
protected:
	virtual void BeginPlay() override;
private:
	//parameters that need to stay consistent during gameplay
	FProcessParameters m_params;

	void InitGameLift();
};
