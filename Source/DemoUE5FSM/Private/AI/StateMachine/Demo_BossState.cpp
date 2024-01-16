#include "DemoUE5FSM/Public/AI/StateMachine/Demo_BossState.h"

#include "AI/Demo_BossCharacter.h"
#include "AI/Demo_BossController.h"
#include "FiniteStateMachine/FiniteStateMachine.h"
#include "Gameplay/Demo_TaggedActor.h"
#include "Gameplay/Demo_TaggedActorsCollection.h"

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

void UDemo_BossState_Patrolling::OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState)
{
	Super::OnActivated(StateAction, OldState);

	GlobalStateData->bIsInvincible = true;
	DelaySeekingState();
}

void UDemo_BossState_Patrolling::OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState)
{
	GlobalStateData->bIsInvincible = false;
	GetTimerManager().ClearTimer(SeekingTransitionTimer);

	Super::OnDeactivated(StateAction, NewState);
}

TCoroutine<> UDemo_BossState_Patrolling::Label_Default()
{
	while (true)
	{
		AActor* MovePoint = GetMovePoint();

		// Get to a patrol point
		RUN_LATENT_EXECUTION(AI::AIMoveTo, Controller.Get(), MovePoint, -1.f, EAIOptionFlag::Disable);

		// Stay idle for a while
		RUN_LATENT_EXECUTION(Latent::Seconds, FMath::FRandRange(MinWaitTimeUponMove, MaxWaitTimeUponMove));
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
	return GetMovePoint_Base(Character.Get(), GlobalStateData->AvailablePatrollingTags, MinimumMovePointDistance);
}

TCoroutine<> UDemo_BossState_Seeking::Label_Default()
{
	// @todo implement rage stun
	// Push the rage stun; the code after this operation will only take place after we become active
	// PUSH_STATE(UDemo_BossState_RageStun);

	while (true)
	{
		AActor* MovePoint = GetMovePoint();

		// Get to a point
		RUN_LATENT_EXECUTION(AI::AIMoveTo, Controller.Get(), MovePoint, -1.f, EAIOptionFlag::Disable);
	}
}

AActor* UDemo_BossState_Seeking::GetMovePoint() const
{
	return GetMovePoint_Base(Character.Get(), GlobalStateData->AvailableSeekingTags, MinimumMovePointDistance);
}
