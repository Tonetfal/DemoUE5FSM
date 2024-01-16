#include "DemoUE5FSM/Public/AI/StateMachine/Demo_BossState.h"

#include "AI/Demo_BossCharacter.h"
#include "AI/Demo_BossController.h"
#include "FiniteStateMachine/FiniteStateMachine.h"

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
		// @todo define UDemo_BossState_Seeking
		// StateMachine->PushState(UDemo_BossState_Seeking::StaticClass());
	}, SeekingTransitionDelay, false);
}

AActor* UDemo_BossState_Patrolling::GetMovePoint() const
{
	// @todo implement
	return nullptr;
}
