// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "StatsComponent.h"
#include "Net/UnrealNetwork.h"

//Helper Macros

#if 0
float MacroDuration = 2.f;
#define SLOG(x,...) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Yellow, FString::Printf(TEXT(x), ##__VA_ARGS__));}
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHalfHeight(), CapRadius(), FQuat::Identity, c, !MacroDuration, MacroDuration);
#else
#define SLOG(x,...)
#define POINT(x, c)x
#define LINE(x1, x2, c)
#define CAPSULE(x, c)
#endif

#pragma region SavedMove

UCustomCharacterMovementComponent::FSavedMove_Custom::FSavedMove_Custom()
{
	Saved_bWantsToSprint=0;
	Saved_bWantsToProne = 0;
	Saved_bWantsToWalk = 0;
	Saved_bPrevWantsToCrouch=0;
	Saved_bPressedCustomJump=0;
	Saved_bHadAnimRootMotion=0;
	Saved_bTransitionFinished=0;
}

bool UCustomCharacterMovementComponent::FSavedMove_Custom::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Custom* NewCustomMove = static_cast<FSavedMove_Custom*>(NewMove.Get());
	if(Saved_bWantsToSprint != NewCustomMove->Saved_bWantsToSprint)
	{
		return false;
	}

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToSprint = 0;

	Saved_bWantsToProne = 0;
	Saved_bPrevWantsToCrouch = 0;

	Saved_bPressedCustomJump = 0;
	Saved_bHadAnimRootMotion = 0;
	Saved_bTransitionFinished = 0;
}

uint8 UCustomCharacterMovementComponent::FSavedMove_Custom::GetCompressedFlags() const
{
	uint8 Result = Super::FSavedMove_Character::GetCompressedFlags();

	if(Saved_bWantsToSprint) Result |= FLAG_Custom_0;
	//High-jacking the Jump flag to lighten network bandwidth
	if(Saved_bPressedCustomJump) Result |= FLAG_JumpPressed;

	return Result;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;

	Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;

	Saved_bWantsToProne = CharacterMovement->Safe_bWantsToProne;

	Saved_bPressedCustomJump = CharacterMovement->CustomCharacterOwner->bPressedCustomJump;

	Saved_bHadAnimRootMotion = CharacterMovement->Safe_bHadAnimRootMotion;
	Saved_bTransitionFinished = CharacterMovement->Safe_bTransitionFinished;
}

void UCustomCharacterMovementComponent::FSavedMove_Custom::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UCustomCharacterMovementComponent* CharacterMovement = Cast<UCustomCharacterMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;

	CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;

	CharacterMovement->Safe_bWantsToProne = Saved_bWantsToProne;

	CharacterMovement->CustomCharacterOwner->bPressedCustomJump = Saved_bPressedCustomJump;

	CharacterMovement->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
	CharacterMovement->Safe_bTransitionFinished = Saved_bTransitionFinished;
}

#pragma endregion

#pragma region Client Network Prediction Data
UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{ }

FSavedMovePtr UCustomCharacterMovementComponent::FNetworkPredictionData_Client_Custom::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Custom());
}

#pragma endregion

#pragma region Custom Movement Component

void UCustomCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	CustomCharacterOwner = Cast<ACustomCharacter>(GetOwner());

	StatsComponent = CustomCharacterOwner->Stats;
}


// Networking
void UCustomCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) !=0;
}

FNetworkPredictionData_Client* UCustomCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr)

	if (ClientPredictionData == nullptr)
	{
		UCustomCharacterMovementComponent* MutableThis = const_cast<UCustomCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Custom(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

//Getters and Helper functions
bool UCustomCharacterMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide) || IsCustomMovementMode(CMOVE_Prone);
}

bool UCustomCharacterMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UCustomCharacterMovementComponent::GetMaxSpeed() const
{
	if(IsMovementMode(MOVE_Walking) && Safe_bWantsToSprint && !IsCrouching()) return Sprint_MaxSpeed;

	if(MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return Slide_MaxSpeed;
	case CMOVE_Prone:
		return Prone_MaxSpeed;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}

float UCustomCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if(MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return Slide_BrakingDeceleration;
	case CMOVE_Prone:
		return Prone_BrakingDeceleration;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
		return -1.f;
	}
}

//Movement Pipeline
void UCustomCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if(MovementMode == MOVE_Walking && Safe_bWantsToSprint && StatsComponent->GetStamina()<=0)
	{
		Safe_bWantsToSprint = false;
	}
	if (MovementMode == MOVE_Walking && Safe_bWantsToSprint && Safe_bPrevWantsToCrouch)
	{
		StatsComponent->StopRegenStamina();
		if(CanSlide())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Slide);
		}
	}
	
	if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}
	
	if(Safe_bWantsToProne)
	{
		if(CanProne())
		{
			SetMovementMode(MOVE_Custom,CMOVE_Prone);
			if(!CharacterOwner->HasAuthority()) Server_EnterProne();
		}
		Safe_bWantsToProne = false;
	}

	if(IsCustomMovementMode(CMOVE_Prone) && !bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}

	//Try Mantle
	if (CustomCharacterOwner->bPressedCustomJump)
	{
		if(TryMantle())
		{
			CustomCharacterOwner->StopJumping();
		}
		else
		{
			SLOG("Mantle Failed, Reverting to Jump")
			//If mantle is failed reverting to the source code jump function
			CustomCharacterOwner->bPressedCustomJump = false;
			CharacterOwner->bPressedJump = true;
			CharacterOwner->CheckJumpInput(DeltaSeconds);
		}
	}

	//Transition Mantle
	if (Safe_bTransitionFinished)
	{
		SLOG("Transition Finished")
		UE_LOG(LogTemp, Warning, TEXT("FINISHED ROOTMOTION"))
		if (IsValid(TransitionQueuedMontage))
		{
			SetMovementMode(MOVE_Flying);
			CharacterOwner->PlayAnimMontage(TransitionQueuedMontage, TransitionQueuedMontageSpeed);
			TransitionQueuedMontageSpeed = 0.f;
			TransitionQueuedMontage = nullptr;
		}
		else
		{
			SetMovementMode(MOVE_Walking);
		}
		Safe_bTransitionFinished = false;
	}
	
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UCustomCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	if(MovementMode == MOVE_Walking && Safe_bWantsToSprint)
	{
		StatsComponent->DecayStamina();
	}
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	if(!HasAnimRootMotion() && Safe_bHadAnimRootMotion && IsMovementMode(MOVE_Flying))
	{
		UE_LOG(LogTemp, Warning, TEXT("Endign Anim Root Motion"));
		SetMovementMode(MOVE_Walking);
	}
	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		Safe_bTransitionFinished = true;
	}

	Safe_bHadAnimRootMotion = HasAnimRootMotion();
}

void UCustomCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime,Iterations);
		break;
	case CMOVE_Prone:
		PhysProne(deltaTime,Iterations);
		break;
	default:
		UE_LOG(LogTemp,Fatal,TEXT("Invalid Movement Mode"))
	}
}

void UCustomCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	
	Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

void UCustomCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide) ExitSlide();
	if(PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Prone) ExitProne();
	if(IsCustomMovementMode(CMOVE_Slide)) EnterSlide(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);
	if(IsCustomMovementMode(CMOVE_Slide)) EnterSlide(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);
}
#pragma endregion

#pragma region Interface

void UCustomCharacterMovementComponent::SprintPressed()
{
	Safe_bWantsToSprint = true;
}

void UCustomCharacterMovementComponent::SprintReleased()
{
	Safe_bWantsToSprint = false;
	StatsComponent->StartRegenStamina();
}

void UCustomCharacterMovementComponent::CrouchPressed()
{
	bWantsToCrouch = !bWantsToCrouch;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_EnterProne, this, &UCustomCharacterMovementComponent::TryEnterProne, Prone_EnterHoldDuration);
}

void UCustomCharacterMovementComponent::CrouchReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_EnterProne);
}

bool UCustomCharacterMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UCustomCharacterMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

#pragma endregion

#pragma region Slide

bool UCustomCharacterMovementComponent::CanSlide() const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.5f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	bool bValidSurface = GetWorld()->LineTraceTestByProfile(Start, End, ProfileName, CustomCharacterOwner->GetIgnoreCharacterParams());
	bool bEnoughSpeed = Velocity.SizeSquared() > pow(Slide_MinSpeed, 2);
	
	return bValidSurface && bEnoughSpeed;
}

void UCustomCharacterMovementComponent::EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;
	Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
	SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void UCustomCharacterMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
	FHitResult Hit;
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation,true,Hit);
	SetMovementMode(MOVE_Walking);
}

void UCustomCharacterMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
    // Restore any root motion velocity added in a previous frame
    RestorePreAdditiveRootMotionVelocity();

    // Check if the character is on a valid sliding surface and moving fast enough
    FHitResult SurfaceHit;
    if (!GetSliderSurface(SurfaceHit) || Velocity.SizeSquared() < FMath::Square(Slide_MinSpeed))
    {
        ExitSlide();
        StartNewPhysics(deltaTime, Iterations);
        return;
    }

    // Apply gravity force in the downward direction
    Velocity += Slide_GravityForce * FVector::DownVector * deltaTime;

    // If no animation root motion or override velocity is present, calculate movement velocity
    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        CalcVelocity(deltaTime, Slide_FrictionFactor, true, GetMaxBrakingDeceleration());
    }

    // Apply root motion to the velocity if needed
    ApplyRootMotionToVelocity(deltaTime);

    // Prepare for movement iteration
    Iterations++;
    bJustTeleported = false;

    // Store the current location and rotation for post-movement velocity calculation
    const FVector OldLocation = UpdatedComponent->GetComponentLocation();
    const FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();

    // Calculate the intended movement vector for this frame
    FHitResult Hit(1.f);
    const FVector Adjusted = Velocity * deltaTime;

    // Determine the direction to slide along the surface
    const FVector VelocityPlaneDirection = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
    const FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelocityPlaneDirection, SurfaceHit.Normal).ToQuat();

    // Attempt to move the component along the slide
    SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

    // If we hit something, handle the impact and continue sliding along the new surface
    if (Hit.Time < 1.f)
    {
        HandleImpact(Hit, deltaTime, Adjusted);
        SlideAlongSurface(Adjusted, 1.f - Hit.Time, Hit.Normal, Hit, true);
    }

    // Check if we should exit the slide after the movement
    FHitResult NewSurfaceHit;
    if (!GetSliderSurface(NewSurfaceHit) || Velocity.SizeSquared() < FMath::Square(Slide_MinSpeed))
    {
        ExitSlide();
    }

    // Update velocity based on actual movement, if not overridden or teleported
    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }
}

bool UCustomCharacterMovementComponent::GetSliderSurface(FHitResult& Hit) const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start+CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*2.f*FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	return GetWorld()->LineTraceSingleByProfile(Hit,Start,End,ProfileName,CustomCharacterOwner->GetIgnoreCharacterParams());
}

#pragma endregion

#pragma region Prone

void UCustomCharacterMovementComponent::Server_EnterProne_Implementation()
{
	Safe_bWantsToProne = true;
}

void UCustomCharacterMovementComponent::EnterProne(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;

	if(PrevMode == MOVE_Custom && PrevCustomMode == CMOVE_Slide)
	{
		Velocity += Velocity.GetSafeNormal2D() * Prone_SlideEnterImpulse;
	}

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);
}

void UCustomCharacterMovementComponent::ExitProne()
{
}

bool UCustomCharacterMovementComponent::CanProne() const
{
	return IsCustomMovementMode(CMOVE_Slide) || IsMovementMode(MOVE_Walking) && IsCrouching();
}

void UCustomCharacterMovementComponent::PhysProne(float deltaTime, int32 Iterations)
{
	if(deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if(!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	//Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		//Save current values
		UPrimitiveComponent * const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		//Ensure velocity is horizontal
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;
		Acceleration.Z = 0.f;

		//Apply acceleration
		CalcVelocity(timeTick, GroundFriction, false, GetMaxBrakingDeceleration()); // friction and breaking deceleration are similar but not the same, because friction is always applied to the capsule component (for example on release of the forward key BreakingDeceleration ramps up exponentially)

		//Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity; // dx = v * dt
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if( IsFalling() )
			{
				//Pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if(DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation()-OldLocation).Size2D();
					remainingTime += timeTick*(1.f - FMath::Min(1.f, ActualDist/DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming())
			{
				//Pawn entered water
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		//Update floor.
		//StepUp might have already done it for us but this is here in case of stairs or shallow ledges (from the EpicGames Manual)
		if(StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}

		//Check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if(bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			//calculate possible alternate movement
			const FVector GravDir = FVector(0.f, 0.f,-1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if(!NewDelta.IsZero())
			{
				//First revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				//Avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				//Try new movement direction
				Velocity = NewDelta/timeTick; //v = dx/dt
				remainingTime += timeTick;
				continue;
			}
			else
			{
				//see if its OK to jump
				// TODO: collision only thing that can be a problem is that OldBase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase -> IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ( (bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				//Revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				//The floor check failed, because it started in penetration
				//We don't want to try moving downward because the down sweep failed, rather we would like to try and pop out of the floor (resolve the penetration)
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			//Check if entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			//Check if we started falling
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		//Allow overlap event and uch to change phys state and velocity
		if(IsMovingOnGround())
		{
			//Make velocity reflect actual move
			if(!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				//TODO-RootMotionSource: Allow this to happen during partial override velocity, but only on allowed axes
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
				MaintainHorizontalGroundVelocity();
			}
		}

		//If we didn't move this iteration, abort since future iterations would be stuck too
		if(UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}

	//Semi-redundant Fail-safe (ported over from the official source code from Epic Games' previous titles)
	if(IsMovingOnGround())
	{
		MaintainHorizontalGroundVelocity();
	}
}
#pragma endregion

#pragma region Mantle

bool UCustomCharacterMovementComponent::TryMantle()
{
	if (!(IsMovementMode(MOVE_Walking) && !IsCrouching()) && !IsMovementMode(MOVE_Falling)) return false;

	//Helper variables
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHalfHeight();
	FVector ForwardSafeNormal = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = CustomCharacterOwner->GetIgnoreCharacterParams();
	float MaxHeight = CapHalfHeight() * 2 + Mantle_ReachHeight;
	float CosMantleMinWallSteepnessAngle = FMath::Cos(FMath::DegreesToRadians(Mantle_MinWallSteepnessAngle));
	float CosMantleMaxSurfaceAngle = FMath::Cos(FMath::DegreesToRadians(Mantle_MaxSurfaceAngle));
	float CosMantleMaxAlignmentAngle = FMath::Cos(FMath::DegreesToRadians(Mantle_MaxAlignmentAngle));

	SLOG("Starting Mantle Attempt")

	//Check Front Face
	FHitResult FrontHit;
	float CheckDistance = FMath::Clamp(Velocity|ForwardSafeNormal, CapRadius() + 30, Mantle_MaxDistance);
	FVector FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight -1);
	for (int i = 0; i < 6; i++)
	{
		LINE(FrontStart, FrontStart+ForwardSafeNormal*CheckDistance, FColor::Red)
		if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, FrontStart+ForwardSafeNormal*CheckDistance, "BlockAll" , Params)) break;
		FrontStart += FVector::UpVector * (2.f * CapHalfHeight() - (MaxStepHeight - 1)) / 5;
	}
	if (!FrontHit.IsValidBlockingHit()) return false;
	
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	if(FMath::Abs(CosWallSteepnessAngle) > CosMantleMinWallSteepnessAngle || (ForwardSafeNormal | -FrontHit.Normal) < CosMantleMaxAlignmentAngle) return false;
	POINT(FrontHit.Location, FColor::Red);

	//Check Height
	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal();
	float WallCos = FVector::UpVector | FrontHit.Normal;
	float WallSin = FMath::Sqrt(1-WallCos * WallCos);
	FVector TraceStart = FrontHit.Location + ForwardSafeNormal + WallUp * (MaxHeight - (MaxStepHeight - 1)) / WallSin;
	LINE(TraceStart, FrontHit.Location+ForwardSafeNormal, FColor::Orange);
	if (!GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontHit.Location + ForwardSafeNormal, "BlockAll", Params)) return false;
	for (const FHitResult& Hit : HeightHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			SurfaceHit = Hit;
			break;
		}
	}
	if (!SurfaceHit.IsValidBlockingHit() || (SurfaceHit.Normal | FVector::UpVector) < CosMantleMaxSurfaceAngle) return false;
	float Height = (SurfaceHit.Location - BaseLoc) | FVector::UpVector;
	SLOG("Height: %f", Height);
	POINT(SurfaceHit.Location, FColor::Blue);

	if (Height > MaxHeight) return false;

	//Check Clearance
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);
	FVector ClearCapsuleLocation = SurfaceHit.Location+ForwardSafeNormal*CapRadius() + FVector::UpVector * (CapHalfHeight() + 1 + CapRadius() * 2 * SurfaceSin);
	FCollisionShape CapShape = FCollisionShape::MakeCapsule(CapRadius(), CapHalfHeight());

	if (GetWorld()->OverlapAnyTestByProfile(ClearCapsuleLocation, FQuat::Identity, "BlockAll", CapShape, Params))
	{
		CAPSULE(ClearCapsuleLocation, FColor::Red);
		return false;
	}
	else
	{
		CAPSULE(ClearCapsuleLocation, FColor::Green);
	}

	SLOG("Can Mantle")

	//Mantle Selection
	FVector ShortMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, false);
	FVector TallMantleTarget = GetMantleStartLocation(FrontHit, SurfaceHit, true);

	bool bTallMantle = false;
	if(IsMovementMode(MOVE_Walking) && Height > CapHalfHeight() * 2)
		bTallMantle = true;
	else if (IsMovementMode(MOVE_Falling) && (Velocity | FVector::UpVector) < 0)
	{
		if(!GetWorld()->OverlapAnyTestByProfile(TallMantleTarget, FQuat::Identity, "BlockAll", CapShape, Params))
			bTallMantle = true;
	}
	FVector TransitionTarget = bTallMantle ? TallMantleTarget : ShortMantleTarget;
	CAPSULE(TransitionTarget, FColor::Yellow)
	//Perform Transition to Mantle
	CAPSULE(UpdatedComponent->GetComponentLocation(), FColor::Red)

	float UpSpeed = Velocity | FVector::UpVector;
	float TransitionDistance = FVector::Dist(TransitionTarget, UpdatedComponent->GetComponentLocation());
	TransitionQueuedMontageSpeed = FMath::GetMappedRangeValueClamped(FVector2D(-500, 750), FVector2D(.9f, 1.2f), UpSpeed);
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_MoveToForce>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;

	TransitionRMS->Duration = FMath::Clamp(TransitionDistance/500.f, .1f, .25f);
	SLOG("Duration: %f", TransitionRMS->Duration)
	TransitionRMS->StartLocation = UpdatedComponent->GetComponentLocation();
	TransitionRMS->TargetLocation = TransitionTarget;

	//Apply Transition Root Motion Source
	Velocity = FVector::ZeroVector;
	SetMovementMode(MOVE_Flying);
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);

	//Animations
	if(bTallMantle)
	{
		TransitionQueuedMontage = TallMantleMontage;
		CharacterOwner->PlayAnimMontage(TransitionTallMantleMontage, 1 / TransitionRMS->Duration);
		if(IsServer()) Proxy_bTallMantle = !Proxy_bTallMantle;
	}
	else
	{
		TransitionQueuedMontage = ShortMantleMontage;
		CharacterOwner->PlayAnimMontage(TransitionShortMantleMontage, 1 / TransitionRMS->Duration);
		if (IsServer()) Proxy_bShortMantle = !Proxy_bShortMantle;
	}
	
	return true;
}

FVector UCustomCharacterMovementComponent::GetMantleStartLocation(FHitResult FrontHit, FHitResult SurfaceHit, bool bTallMantle) const
{
	float CosWallSteepnessAngle = FrontHit.Normal | FVector::UpVector;
	float DownDistance = bTallMantle ? CapHalfHeight() * 2.f : MaxStepHeight - 1;
	FVector EdgeTangent = FVector::CrossProduct(SurfaceHit.Normal, FrontHit.Normal).GetSafeNormal();
	FVector MantleStart = SurfaceHit.Location;

	MantleStart += FrontHit.Normal.GetSafeNormal2D() * (2.f + CapRadius());
	MantleStart += UpdatedComponent->GetForwardVector().GetSafeNormal2D().ProjectOnTo(EdgeTangent) * CapRadius() * .3f;
	MantleStart += FVector::UpVector * CapHalfHeight();
	MantleStart += FVector::DownVector * DownDistance;
	MantleStart += FrontHit.Normal.GetSafeNormal2D() * CosWallSteepnessAngle * DownDistance;

	return MantleStart;
}

void UCustomCharacterMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UCustomCharacterMovementComponent, Proxy_bShortMantle, COND_SkipOwner)
	DOREPLIFETIME_CONDITION(UCustomCharacterMovementComponent, Proxy_bTallMantle, COND_SkipOwner)
}

void UCustomCharacterMovementComponent::OnRep_ShortMantle()
{
	CharacterOwner->PlayAnimMontage(ProxyShortMantleMontage);
}

void UCustomCharacterMovementComponent::OnRep_TallMantle()
{
	CharacterOwner->PlayAnimMontage(ProxyTallMantleMontage);
}

#pragma endregion

#pragma region Helpers

bool UCustomCharacterMovementComponent::IsServer() const
{
	return CharacterOwner->HasAuthority();
}

float UCustomCharacterMovementComponent::CapRadius() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UCustomCharacterMovementComponent::CapHalfHeight() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

bool UCustomCharacterMovementComponent::IsSprinting() const
{
	return IsMovementMode(MOVE_Walking) && Safe_bWantsToSprint;
}

#pragma endregion

UCustomCharacterMovementComponent::UCustomCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
}