// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CustomPlayerController.h"

void ACustomPlayerController::ClientShowNotification_Implementation(const FText& Message)
{
	ShowNotification(Message);
}
