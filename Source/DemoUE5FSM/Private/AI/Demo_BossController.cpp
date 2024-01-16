#include "DemoUE5FSM/Public/AI/Demo_BossController.h"

#include "AI/StateMachine/Demo_BossState.h"
#include "FiniteStateMachine/FiniteStateMachine.h"

ADemo_BossController::ADemo_BossController()
{
	StateMachine = CreateDefaultSubobject<UFiniteStateMachine>("FiniteStateMachine");
	StateMachine->bAutoActivate = false;
}

UFiniteStateMachine* ADemo_BossController::GetStateMachine() const
{
	return StateMachine;
}

void ADemo_BossController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Activate the machine only after possessing the pawn it'll be controlling
	StateMachine->Activate(true);

	// Cache this data so that we can easily interact with our FSM
	GlobalState = StateMachine->GetStateChecked<UDemo_BossState_Global>();
	GlobalStateData = StateMachine->GetStateDataChecked<UDemo_GlobalBossStateData, UDemo_BossState_Global>();
}