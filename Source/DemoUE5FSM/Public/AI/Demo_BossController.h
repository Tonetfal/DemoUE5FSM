#pragma once

#include "AIController.h"

#include "Demo_BossController.generated.h"

class UDemo_GlobalBossStateData;
class UDemo_BossState_Global;
class UFiniteStateMachine;

UCLASS()
class DEMOUE5FSM_API ADemo_BossController
	: public AAIController
{
	GENERATED_BODY()

public:
	ADemo_BossController();

	UFiniteStateMachine* GetStateMachine() const;

protected:
	//~AAIController Interface
	virtual void OnPossess(APawn* InPawn) override;
	//~End of AAIController Interface

	UFUNCTION()
	void OnProgression();

	UFUNCTION()
	void OnFlashlight();

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="AI")
	TObjectPtr<UFiniteStateMachine> StateMachine = nullptr;

	TWeakObjectPtr<UDemo_BossState_Global> GlobalState = nullptr;
	TWeakObjectPtr<UDemo_GlobalBossStateData> GlobalStateData = nullptr;
};
