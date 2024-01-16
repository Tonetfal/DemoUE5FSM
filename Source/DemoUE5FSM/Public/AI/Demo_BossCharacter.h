#pragma once

#include "GameFramework/Character.h"

#include "Demo_BossCharacter.generated.h"

class ADemo_BossController;
class UDemo_BossState_Global;
class UDemo_GlobalBossStateData;
class UFiniteStateMachine;

UCLASS()
class DEMOUE5FSM_API ADemo_BossCharacter
	: public ACharacter
{
	GENERATED_BODY()

public:
	ADemo_BossCharacter();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void PlayAnimation(UAnimMontage* Animation);

protected:
	//~ACharacter Interface
	virtual void PossessedBy(AController* NewController) override;
	//~End of ACharacter Interface

protected:
	TWeakObjectPtr<ADemo_BossController> BossController = nullptr;
	TWeakObjectPtr<UFiniteStateMachine> ControllerStateMachine = nullptr;
	TWeakObjectPtr<UDemo_BossState_Global> GlobalState = nullptr;
	TWeakObjectPtr<UDemo_GlobalBossStateData> GlobalStateData = nullptr;
};
