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
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	FGameplayTagContainer AvailablePatrollingTags;

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

UCLASS()
class DEMOUE5FSM_API UDemo_BossState_Patrolling
	: public UDemo_BossState
{
	GENERATED_BODY()

protected:
	//~UDemo_BossState Interface
	virtual void OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState) override;
	//~End of UDemo_BossState Interface

	//~Labels
	virtual TCoroutine<> Label_Default() override;
	//~End of Labels

	void DelaySeekingState();
	AActor* GetMovePoint() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Movement", meta=(ClampMin="0.0"))
	float MinWaitTimeUponMove = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Movement", meta=(ClampMin="0.0"))
	float MaxWaitTimeUponMove = 3.f;

	UPROPERTY(EditDefaultsOnly, Category="Seeking Transition", meta=(ClampMin="0.0"))
	float SeekingTransitionDelay = 10.f;

	/** Minimal distance the move point have to be at in order to consider them valid. */
	UPROPERTY(EditDefaultsOnly, Category="Movement", meta=(ClampMin="0.0"))
	float MinimumMovePointDistance = 1000.f;

private:
	FTimerHandle SeekingTransitionTimer;
};

UCLASS()
class DEMOUE5FSM_API UDemo_BossState_Seeking
	: public UDemo_BossState
{
	GENERATED_BODY()

protected:
	//~Labels
	virtual TCoroutine<> Label_Default() override;
	//~End of Labels

	AActor* GetMovePoint() const;
};
