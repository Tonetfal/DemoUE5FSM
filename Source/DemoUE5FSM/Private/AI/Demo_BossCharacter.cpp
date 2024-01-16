#include "DemoUE5FSM/Public/AI/Demo_BossCharacter.h"

#include "AI/Demo_BossController.h"
#include "AI/StateMachine/Demo_BossState.h"
#include "FiniteStateMachine/FiniteStateMachine.h"

ADemo_BossCharacter::ADemo_BossCharacter()
{
}

void ADemo_BossCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Wait a tick as it takes that to finish FSM initialization
	GetWorldTimerManager().SetTimerForNextTick([this]
	{
		// Sanity check
		if (IsValid(Controller))
		{
			// Cache this data so that we can easily interact our controller's FSM
			BossController = CastChecked<ADemo_BossController>(Controller);
			ControllerStateMachine = BossController->GetStateMachine();
			GlobalState = ControllerStateMachine->GetStateChecked<UDemo_BossState_Global>();
			GlobalStateData = ControllerStateMachine->GetStateDataChecked<UDemo_GlobalBossStateData, UDemo_BossState_Global>();
		}
	});
}
