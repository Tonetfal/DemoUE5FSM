#include "DemoUE5FSM/Public/AI/StateMachine/Demo_BossState.h"

#include "AI/Demo_BossCharacter.h"
#include "AI/Demo_BossController.h"
#include "DemoUE5FSM/DemoUE5FSMCharacter.h"
#include "FiniteStateMachine/FiniteStateMachine.h"
#include "Gameplay/Demo_TaggedActor.h"
#include "Gameplay/Demo_TaggedActorsCollection.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"
#include "Senses/AISensePredictionPlus.h"

/**
 * Unify the GetMovePoint() under one single function.
 * Don't make direct use of the function, but use it inside your own GetMovePoint(), as at some point you might want to
 * change the implementation of a particular GetMovePoint().
 */
static AActor* GetMovePoint_Base(const ACharacter* Character, FGameplayTagContainer Tags, float MinDistance)
{
	const auto& TaggedActorsCollection = *Character->GetWorld()->GetSubsystem<UDemo_TaggedActorsCollection>();
	TArray<ADemo_TaggedActor*> MovePoints = TaggedActorsCollection.QueryActors(
		Tags, EDemo_GameplayTagQueryType::AnyTagsMatch);

	const int32 TotalNum = MovePoints.Num();
	if (!ensure(TotalNum > 0))
	{
		return nullptr;
	}

	// Filter out the ones that are too close
	TArray<ADemo_TaggedActor*> ValidMovePoints;
	const FVector Origin = Character->GetActorLocation();
	for (ADemo_TaggedActor* MovePoint : MovePoints)
	{
		const FVector MovePointPosition = MovePoint->GetActorLocation();
		const FVector ToMovePoint = Origin - MovePointPosition;
		const float DistanceSq = ToMovePoint.SquaredLength();
		const float MinimumDistanceSq = FMath::Square(MinDistance);
		if (DistanceSq > MinimumDistanceSq)
		{
			ValidMovePoints.Add(MovePoint);
		}
	}

	// We might've filter out everything
	const int32 ValidNum = ValidMovePoints.Num();
	if (ensureMsgf(ValidNum > 0, TEXT("There's no valid move point at [%s] using [%s] tags."),
		*Origin.ToString(), *Tags.ToString()))
	{
		const int32 RandIndex = FMath::RandRange(0, ValidNum - 1);
		return ValidMovePoints[RandIndex];
	}

	return nullptr;
}

void UDemo_BossState::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	if (bStunRestartsAI && IsValid(OldState))
	{
		// Filter out stun classes that we don't want to be restarted by
		if (StunBlocklist.ContainsByPredicate([OldState] (TSubclassOf<UDemo_BossState_Stun>)
			{
				return OldState->IsChildOf(UDemo_BossState_Stun::StaticClass());
			}))
		{
			return;
		}

		if (OldState->IsChildOf(UDemo_BossState_Stun::StaticClass()))
		{
			// Don't run any label as we're about to reset the AI
			GotoLabel(FGameplayTag::EmptyTag);

			// Restart the AI upon being stunned
			GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this] { RestartAI(); }));
		}
	}
}

void UDemo_BossState::OnAddedToStack(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnAddedToStack(StateAction, OldState);

	Controller = GetOwnerChecked<ADemo_BossController>();
	Character = Controller->GetPawn<ADemo_BossCharacter>();
	GlobalStateData = StateMachine->GetStateDataChecked<UDemo_GlobalBossStateData, UDemo_BossState_Global>();

	check(Character.IsValid());
}

void UDemo_BossState::OnRemovedFromStack(EStateAction StateAction, TSubclassOf<UMachineState> NewState)
{
	Controller.Reset();
	Character.Reset();
	GlobalStateData.Reset();

	Super::OnRemovedFromStack(StateAction, NewState);
}

void UDemo_BossState::RestartAI()
{
	// Make sure that there's no other action running while we're stunned
	StopLatentExecution();

	// Stopping the latent function doesn't prevent MoveTo to abort the movement, so we have to stop it ourselves
	Controller->StopMovement();

	// Release the player
	Character->SetGrabbedPlayer(nullptr);

	// Remove any current state
	ClearStack();

	// Start off by patrolling
	GotoState(UDemo_BossState_Patrolling::StaticClass());
}

void UDemo_GlobalBossStateData::SetTargetActor(AActor* InTarget)
{
	TargetActor = InTarget;

	if (TargetActor.IsValid())
	{
		TargetPosition = TargetActor->GetActorLocation();
	}
}

void UDemo_GlobalBossStateData::SetTargetLocation(FVector InLocation)
{
	TargetActor.Reset();
	TargetPosition = InLocation;
}

void UDemo_BossState_Global::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	UAIPerceptionComponent* PerceptionComponent = Controller->GetPerceptionComponent();
	check(IsValid(PerceptionComponent));

	PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnActorPerceptionUpdated);
}

void UDemo_BossState_Global::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AActor* ClosestActor = GetClosestActor();
	if (IsValid(ClosestActor))
	{
		GlobalStateData->SetTargetActor(ClosestActor);
	}
}

void UDemo_BossState_Global::OnActorPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	const TSubclassOf<UAISense> PerceptionClass = UAIPerceptionSystem::GetSenseClassForStimulus(this, Stimulus);
	if (UAISense_Sight::StaticClass() == PerceptionClass)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			GlobalStateData->SeenActors.AddUnique(Actor);
			TryToPushChasingPlayerState();
		}
		else
		{
			GlobalStateData->SeenActors.RemoveSingle(Actor);
			UAISense_PredictionPlus::RequestPawnPredictionPlusEvent(Character.Get(), Actor, 1.f);
		}
	}
	else if (UAISense_PredictionPlus::StaticClass() == PerceptionClass)
	{
		GlobalStateData->SetTargetLocation(Stimulus.StimulusLocation);
		TryToPushChasingPlayerState();
	}
}

AActor* UDemo_BossState_Global::GetClosestActor() const
{
	AActor* Winner = nullptr;
	float ClosestDistanceSq = FLT_MAX;
	const FVector OurPosition = Character->GetActorLocation();

	for (const TWeakObjectPtr<AActor> Actor : GlobalStateData->SeenActors)
	{
		const FVector TargetPosition = Actor->GetActorLocation();
		const FVector ToTarget = OurPosition - TargetPosition;
		const float DistanceSq = ToTarget.SquaredLength();
		if (ClosestDistanceSq > DistanceSq)
		{
			Winner = Actor.Get();
			ClosestDistanceSq = DistanceSq;
		}
	}

	return Winner;
}

void UDemo_BossState_Global::TryToPushChasingPlayerState()
{
	if (!StateMachine->IsInState(UDemo_BossState_ChasingPlayer::StaticClass(), true) && !PushRequestHandle.IsPending())
	{
		StateMachine->PushStateQueued(
			PushRequestHandle, UDemo_BossState_ChasingPlayer::StaticClass(), TAG_StateMachine_Label_Default);
	}
}

void UDemo_BossState_Patrolling::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	GlobalStateData->bIsInvincible = true;
	DelaySeekingState();
}

void UDemo_BossState_Patrolling::OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState)
{
	MovePoint.Reset();
	WaitTime = WaitStartTime = 0.f;

	GlobalStateData->bIsInvincible = false;
	GetTimerManager().ClearTimer(SeekingTransitionTimer);

	Super::OnDeactivated(StateAction, NewState);
}

FString UDemo_BossState_Patrolling::GetDebugData() const
{
	FString ReturnValue;

	const float RemainingSeekingTransitionTime = GetTimerManager().GetTimerRemaining(SeekingTransitionTimer);
	if (RemainingSeekingTransitionTime > 0.f)
	{
		ReturnValue.Appendf(TEXT("\t- Remaining seeking transition time (%.2fs)\n"), RemainingSeekingTransitionTime);
	}

	if (MovePoint.IsValid())
	{
		ReturnValue.Appendf(TEXT("\t- Move point (%s)\n"), *MovePoint->GetName());
	}

	const float RemainingWaitTime = WaitTime - TimeSince(WaitStartTime);
	if (RemainingWaitTime > 0.f)
	{
		ReturnValue.Appendf(TEXT("\t- Remaining wait time (%.2fs)\n"), RemainingWaitTime);
	}

	if (!ReturnValue.IsEmpty())
	{
		ReturnValue = FString::Printf(TEXT("\n%s"), *ReturnValue);
	}

	return ReturnValue;
}

TCoroutine<> UDemo_BossState_Patrolling::Label_Default()
{
	while (true)
	{
		// Get to a patrol point
		MovePoint = GetMovePoint();
		RUN_LATENT_EXECUTION(AI::AIMoveTo, Controller.Get(), MovePoint.Get(), -1.f, EAIOptionFlag::Disable);
		MovePoint.Reset();

		// Stay idle for a while
		WaitStartTime = GetTime();
		WaitTime = FMath::FRandRange(MinWaitTimeUponMove, MaxWaitTimeUponMove);
		RUN_LATENT_EXECUTION(Latent::Seconds, WaitTime);
	}
}

void UDemo_BossState_Patrolling::DelaySeekingState()
{
	GetTimerManager().SetTimer(SeekingTransitionTimer, [this]
	{
		StateMachine->PushState(UDemo_BossState_Seeking::StaticClass());
	}, SeekingTransitionDelay, false);
}

AActor* UDemo_BossState_Patrolling::GetMovePoint() const
{
	return GetMovePoint_Base(Character.Get(), GlobalStateData->AvailablePatrollingMovePointTags, MinimumMovePointDistance);
}

TCoroutine<> UDemo_BossState_Seeking::Label_Default()
{
	// Push the rage stun; the code after this operation will only take place after we become active
	PUSH_STATE(UDemo_BossState_RageStun);

	while (true)
	{
		AActor* MovePoint = GetMovePoint();

		// Get to a point
		RUN_LATENT_EXECUTION(AI::AIMoveTo, Controller.Get(), MovePoint, -1.f, EAIOptionFlag::Disable);
	}
}

AActor* UDemo_BossState_Seeking::GetMovePoint() const
{
	return GetMovePoint_Base(Character.Get(), GlobalStateData->AvailableSeekingMovePointTags, MinimumMovePointDistance);
}

UDemo_BossState_Stun::UDemo_BossState_Stun()
{
	StatesBlocklist.Add(UDemo_BossState_ChasingPlayer::StaticClass());
}

void UDemo_BossState_Stun::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	// Sanity check
	if (!ensure(IsValid(StunAnimation)))
	{
		// If we don't have any animation, we no longer want to be in this state
		PopState();
		return;
	}

	// Disallow attacking the boss while stunned as it's unfair
	GlobalStateData->bIsInvincible = true;

	// Make sure that there's no other action running while we're stunned
	StopLatentExecution();

	// Stopping the latent function doesn't prevent MoveTo to abort the movement, so we have to stop it ourselves
	Controller->StopMovement();

	// Release the player
	Character->SetGrabbedPlayer(nullptr);
}

void UDemo_BossState_Stun::OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState)
{
	GlobalStateData->bIsInvincible = false;

	Super::OnDeactivated(StateAction, NewState);
}

TCoroutine<> UDemo_BossState_Stun::Label_Default()
{
	// In the article PlayAnimMontage() is used to avoid some uneccessary multiplier complexity, however, in reality
	// we need to send a multicast RPC to notify all players about the stun state
	Character->PlayAnimation(StunAnimation);

	// Wait until the stun animation ends
	const UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	while (AnimInstance->Montage_IsPlaying(StunAnimation))
	{
		// Wait until the animation doesn't end
		RUN_LATENT_EXECUTION(Latent::NextTick);
	}

	POP_STATE();
}

void UDemo_BossState_ChasingPlayer::OnAddedToStack(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnAddedToStack(StateAction, OldState);

	OnPlayerGrabbedDelegateHandle = Character->OnPlayerGrabbed.AddLambda([this]
	{
		GotoState(UDemo_BossState_CarryingPlayer::StaticClass());
	});

	// Allow grabbing player. Do not allow that when resuming the state, as it might accidentally grab another
	// player that's standing in front of us
	Character->SetGrabCollisionEnabled(true);
}

void UDemo_BossState_ChasingPlayer::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	Controller->ReceiveMoveCompleted.AddDynamic(this, &ThisClass::OnMoveCompleted);
}

void UDemo_BossState_ChasingPlayer::OnRemovedFromStack(EStateAction StateAction, TSubclassOf<UMachineState> NewState)
{
	Character->OnPlayerGrabbed.Remove(OnPlayerGrabbedDelegateHandle);

	Super::OnRemovedFromStack(StateAction, NewState);
}

void UDemo_BossState_ChasingPlayer::OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState)
{
	Controller->ReceiveMoveCompleted.RemoveDynamic(this, &ThisClass::OnMoveCompleted);

	Super::OnDeactivated(StateAction, NewState);
}

void UDemo_BossState_ChasingPlayer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Don't make a new MoveTo request unless the target position has changed
	if (GlobalStateData->TargetPosition != LastTargetPosition)
	{
		Controller->MoveToLocation(GlobalStateData->TargetPosition, -1.f, false);
		LastTargetPosition = GlobalStateData->TargetPosition;
	}
}

void UDemo_BossState_ChasingPlayer::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (Result == EPathFollowingResult::Success || Result == EPathFollowingResult::Blocked)
	{
		PopState();
	}
}

UDemo_BossState_CarryingPlayer::UDemo_BossState_CarryingPlayer()
{
	StatesBlocklist.Add(UDemo_BossState_ChasingPlayer::StaticClass());
}

void UDemo_BossState_CarryingPlayer::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	// Make sure that there's no running MoveTo
	Controller->StopMovement();
}

TCoroutine<> UDemo_BossState_CarryingPlayer::Label_Default()
{
	AActor* MovePoint = GetMovePoint();
	RUN_LATENT_EXECUTION(AI::AIMoveTo, Controller.Get(), MovePoint);

	// Ignore the player for some time
	Character->GetGrabbedPlayer()->TemporarilyUnregisterAsStimuliSource(5.f);

	RestartAI();
}

AActor* UDemo_BossState_CarryingPlayer::GetMovePoint() const
{
	return GetMovePoint_Base(Character.Get(), GlobalStateData->AvailableKillMovePointTags, MinimumMovePointDistance);
}
