#pragma once

#include "FiniteStateMachine/MachineState.h"
#include "FiniteStateMachine/MachineStateData.h"

#include "Demo_BossState.generated.h"

class ADemo_BossCharacter;
class ADemo_BossController;

UCLASS(Abstract)
class DEMOUE5FSM_API UDemo_BossState
	: public UMachineState
{
	GENERATED_BODY()

protected:
	//~UMachineState Interface
	virtual void OnAddedToStack(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnRemovedFromStack(EStateAction StateAction, TSubclassOf<UMachineState> NewState) override;
	//~End of UMachineState Interface

protected:
	TWeakObjectPtr<ADemo_BossController> Controller = nullptr;
	TWeakObjectPtr<ADemo_BossCharacter> Character = nullptr;
	TWeakObjectPtr<class UDemo_GlobalBossStateData> GlobalStateData = nullptr;
};

UCLASS()
class DEMOUE5FSM_API UDemo_GlobalBossStateData
	: public UMachineStateData
{
	GENERATED_BODY()

public:
	bool bIsInvincible = false;
};

UCLASS()
class DEMOUE5FSM_API UDemo_BossState_Global
	: public UDemo_BossState
	, public IGlobalMachineStateInterface
{
	GENERATED_BODY()

public:
	// Empty for now
};
