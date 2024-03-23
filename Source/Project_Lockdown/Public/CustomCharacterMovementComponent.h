// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComponent.generated.h"

/**
 * 
 */

UENUM()
enum ECustomMovementMode
{
	CMOVE_None UMETA(Hidden),
	CMOVE_Slide UMETA(DisplayName="Slide"),
	CMOVE_Prone UMETA(DisplayName="Prone"),
	CMOVE_MAX UMETA(Hidden),
};


UCLASS()
class PROJECT_LOCKDOWN_API UCustomCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	class FSavedMove_Custom : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;
		//Flag
		uint8 Saved_bWantsToSprint:1;
		uint8 Saved_bWantsToWalk:1;
		uint8 Saved_bPressedCustomJump:1; //"PseudoFlag" for mantle mechanic

		//Non-Flag
		uint8 Saved_bPrevWantsToCrouch:1;
		uint8 Saved_bWantsToProne:1;
		uint8 Saved_bHadAnimRootMotion:1;
		uint8 Saved_bTransitionFinished:1;

	public:
	
		FSavedMove_Custom();
	
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Custom : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	//Parameters

	//Stamina (Temporary until moved into its own component)
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Stamina") float Stamina = 100.f;
private:
	UPROPERTY(EditDefaultsOnly) float StaminaDecay = 0.5f;
	UPROPERTY(EditDefaultsOnly) float StaminaRegen = 0.5f;
	UPROPERTY(EditDefaultsOnly) float StaminaRegen_Rate = 0.25f;
	UPROPERTY(EditDefaultsOnly) float StaminaRegen_Delay = 1.f;

	//Sprint
	UPROPERTY(EditDefaultsOnly) float Sprint_MaxSpeed = 1200.f;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxSpeed = 600.f;

	//Slide
	UPROPERTY(EditDefaultsOnly) float Slide_MinSpeed=500.f;
	UPROPERTY(EditDefaultsOnly) float Slide_MaxSpeed=500.f;
	UPROPERTY(EditDefaultsOnly) float Slide_EnterImpulse=300.f;
	UPROPERTY(EditDefaultsOnly) float Slide_GravityForce=100.f;
	UPROPERTY(EditDefaultsOnly) float Slide_FrictionFactor=0.5f;
	UPROPERTY(EditDefaultsOnly) float Slide_BrakingDeceleration=1000.f;

	//Prone
	UPROPERTY(EditDefaultsOnly) float Prone_EnterHoldDuration = .2f;
	UPROPERTY(EditDefaultsOnly) float Prone_SlideEnterImpulse = 300.f;
	UPROPERTY(EditDefaultsOnly) float Prone_MaxSpeed = 300.f;
	UPROPERTY(EditDefaultsOnly) float Prone_BrakingDeceleration = 2500.f;

	//Mantle
	UPROPERTY(EditDefaultsOnly) float Mantle_MaxDistance = 200.f;
	UPROPERTY(EditDefaultsOnly) float Mantle_ReachHeight = 50.f;
	UPROPERTY(EditDefaultsOnly) float Mantle_MinDepth = 30.f;
	UPROPERTY(EditDefaultsOnly) float Mantle_MinWallSteepnessAngle = 75.f;
	UPROPERTY(EditDefaultsOnly) float Mantle_MaxSurfaceAngle = 40.f;
	UPROPERTY(EditDefaultsOnly) float Mantle_MaxAlignmentAngle = 45.f;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionTallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ProxyTallMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ShortMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* TransitionShortMantleMontage;
	UPROPERTY(EditDefaultsOnly) UAnimMontage* ProxyShortMantleMontage;

	//Transient
	UPROPERTY(Transient) ACustomCharacter* CustomCharacterOwner;

	//Flags
	bool Safe_bWantsToSprint;
	bool Safe_bWantsToProne;

	bool Safe_bPrevWantsToCrouch;
	bool Safe_bHadAnimRootMotion;
	FTimerHandle TimerHandle_EnterProne;
	FTimerHandle TimerHandle_StaminaRegen;

	bool Safe_bTransitionFinished;
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;
	float TransitionQueuedMontageSpeed;
	int TransitionRMS_ID;
	
	//Replication
	UPROPERTY(ReplicatedUsing=OnRep_ShortMantle) bool Proxy_bShortMantle;
	UPROPERTY(ReplicatedUsing=OnRep_TallMantle) bool Proxy_bTallMantle;
	
public:
	UCustomCharacterMovementComponent();

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) bool IsSprinting() const;
	
	UFUNCTION(BlueprintCallable) void CrouchPressed();
	UFUNCTION(BlueprintCallable) void CrouchReleased();

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;
	bool CanSlide() const;

public:
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	
protected:
	virtual void InitializeComponent() override;
	
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	//Slide
private:
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSliderSurface(FHitResult& Hit) const;

	//Prone
private:
	void TryEnterProne() { Safe_bWantsToProne = true; }
	void RegenStamina() {Stamina = Stamina + StaminaRegen;}
	UFUNCTION(Server, Reliable) void Server_EnterProne();
	
	void EnterProne(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitProne();
	bool CanProne() const;
	void PhysProne(float deltaTime, int32 Iterations);

	//Mantle
private:
	bool TryMantle();
	FVector GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const;

	//Helpers
private:
	bool IsServer() const;
	float CapRadius() const;
	float CapHalfHeight() const;

	//Proxy Replication
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UFUNCTION() void OnRep_ShortMantle();
	UFUNCTION() void OnRep_TallMantle();
};
