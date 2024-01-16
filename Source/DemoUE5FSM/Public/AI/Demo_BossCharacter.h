#pragma once

#include "GameFramework/Character.h"

#include "Demo_BossCharacter.generated.h"

class UBoxComponent;
class ADemoUE5FSMCharacter;
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

	void SetGrabCollisionEnabled(bool bIsEnabled) const;
	void SetGrabbedPlayer(ADemoUE5FSMCharacter* PlayerCharacter);
	ADemoUE5FSMCharacter* GetGrabbedPlayer() const;

protected:
	//~ACharacter Interface
	virtual void PostInitializeComponents() override;
	virtual void PossessedBy(AController* NewController) override;
	//~End of ACharacter Interface

	UFUNCTION()
	void OnRep_GrabbedPlayerCharacter(TWeakObjectPtr<ADemoUE5FSMCharacter> OldValue);

	UFUNCTION()
	void OnGrabCollionBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	FSimpleMulticastDelegate OnPlayerGrabbed;

protected:
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UBoxComponent> GrabCollision = nullptr;

	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<USceneComponent> GrabPoint = nullptr;

	TWeakObjectPtr<ADemo_BossController> BossController = nullptr;
	TWeakObjectPtr<UFiniteStateMachine> ControllerStateMachine = nullptr;
	TWeakObjectPtr<UDemo_BossState_Global> GlobalState = nullptr;
	TWeakObjectPtr<UDemo_GlobalBossStateData> GlobalStateData = nullptr;

	UPROPERTY(ReplicatedUsing="OnRep_GrabbedPlayerCharacter")
	TWeakObjectPtr<ADemoUE5FSMCharacter> GrabbedPlayerCharacter = nullptr;
};
