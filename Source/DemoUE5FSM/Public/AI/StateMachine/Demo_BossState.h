#pragma once

#include "FiniteStateMachine/FiniteStateMachine.h"
#include "FiniteStateMachine/MachineState.h"
#include "FiniteStateMachine/MachineStateData.h"

#include "Demo_BossState.generated.h"

class ADemo_BossCharacter;
class ADemo_BossController;
struct FAIStimulus;

UCLASS(Abstract)
class DEMOUE5FSM_API UDemo_BossState
	: public UMachineState
{
	GENERATED_BODY()

protected:
	//~UMachineState Interface
	virtual void OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnAddedToStack(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnRemovedFromStack(EStateAction StateAction, TSubclassOf<UMachineState> NewState) override;
	//~End of UMachineState Interface

	void RestartAI();

protected:
	TWeakObjectPtr<ADemo_BossController> Controller = nullptr;
	TWeakObjectPtr<ADemo_BossCharacter> Character = nullptr;
	TWeakObjectPtr<class UDemo_GlobalBossStateData> GlobalStateData = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Stun")
	bool bStunRestartsAI = false;

	UPROPERTY(EditDefaultsOnly, Category="Stun")
	TArray<TSubclassOf<class UDemo_BossState_Stun>> StunBlocklist;
};

UCLASS()
class DEMOUE5FSM_API UDemo_GlobalBossStateData
	: public UMachineStateData
{
	GENERATED_BODY()

public:
	void SetTargetActor(AActor* InTarget);
	void SetTargetLocation(FVector InLocation);

public:
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	FGameplayTagContainer AvailablePatrollingMovePointTags;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	FGameplayTagContainer AvailableSeekingMovePointTags;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	FGameplayTagContainer AvailableKillMovePointTags;

	TWeakObjectPtr<AActor> TargetActor = nullptr;
	FVector TargetPosition = FVector::ZeroVector;
	bool bIsInvincible = false;

	TArray<TWeakObjectPtr<AActor>> SeenActors;
};

UCLASS()
class DEMOUE5FSM_API UDemo_BossState_Global
	: public UDemo_BossState
	, public IGlobalMachineStateInterface
{
	GENERATED_BODY()

protected:
	//~UDemo_BossState Interface
	virtual void OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void Tick(float DeltaSeconds) override;
	//~End of UDemo_BossState Interface

	private:
	UFUNCTION()
	void OnActorPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	AActor* GetClosestActor() const;
	void TryToPushChasingPlayerState();

private:
	FFSM_PushRequestHandle PushRequestHandle;
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
	virtual FString GetDebugData() const override;
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

	TWeakObjectPtr<AActor> MovePoint = nullptr;
	float WaitTime = 0.f;
	float WaitStartTime = 0.f;
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

protected:
	/** Minimal distance the move point have to be at in order to consider them valid. */
	UPROPERTY(EditDefaultsOnly, Category="Movement", meta=(ClampMin="0.0"))
	float MinimumMovePointDistance = 1000.f;
};

/**
 * Base class for a stun state containing minimum logic.
 *
 * There can be different reasons to subclass this class: one might want to add stun counter for a specific stun type
 * for some statistics, or a specific stun type might apply some debuff upon deactivation.
 *
 * In our case, however, we're subclassing it to have different animations based on stun type, and also to allow
 * combining multiple stuns together, as pushing a state that is already present on the stack is prohibited, meaning
 * that the boss cannot be hard stunned while being soft stunned.
 */
UCLASS(Abstract)
class DEMOUE5FSM_API UDemo_BossState_Stun
	: public UDemo_BossState
{
	GENERATED_BODY()

public:
	UDemo_BossState_Stun();

protected:
	//~UDemo_BossState Interface
	virtual void OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState) override;
	//~End of UDemo_BossState Interface

	//~Labels
	virtual TCoroutine<> Label_Default() override;
	//~End of Labels

protected:
	UPROPERTY(EditDefaultsOnly, Category="Stun")
	TObjectPtr<UAnimMontage> StunAnimation = nullptr;
};

/**
 * Stun to apply anytime players lowers boss health up to 0 using their flashlights.
 */
UCLASS()
class DEMOUE5FSM_API UDemo_BossState_SoftStun
	: public UDemo_BossState_Stun
{
	GENERATED_BODY()

public:
	// Empty
};

/**
 * Stun to apply anytime players do something to progress, as it hurts the boss a lot.
 */
UCLASS()
class DEMOUE5FSM_API UDemo_BossState_HardStun
	: public UDemo_BossState_Stun
{
	GENERATED_BODY()

public:
	// Empty
};

/**
 * Stun to apply anytime boss transits from Patrolling to Seeking state, as the boss becomes annoyed by the players.
 */
UCLASS()
class DEMOUE5FSM_API UDemo_BossState_RageStun
	: public UDemo_BossState_Stun
{
	GENERATED_BODY()

public:
	// Empty
};

UCLASS()
class DEMOUE5FSM_API UDemo_BossState_ChasingPlayer
	: public UDemo_BossState
{
	GENERATED_BODY()

protected:
	//~UDemo_BossState_Patrolling Interface
	virtual void OnAddedToStack(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	virtual void OnRemovedFromStack(EStateAction StateAction, TSubclassOf<UMachineState> NewState) override;
	virtual void OnDeactivated(EStateAction StateAction, TSubclassOf<UMachineState> NewState) override;
	virtual void Tick(float DeltaSeconds) override;
	//~End of UDemo_BossState_Patrolling Interface

private:
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

private:
	FVector LastTargetPosition = FVector::ZeroVector;
	FDelegateHandle OnPlayerGrabbedDelegateHandle;
};

UCLASS()
class DEMOUE5FSM_API UDemo_BossState_CarryingPlayer
	: public UDemo_BossState
{
	GENERATED_BODY()

public:
	UDemo_BossState_CarryingPlayer();

protected:
	//~UDemo_BossState Interface
	virtual void OnActivated(EStateAction StateAction, TSubclassOf<UMachineState> OldState) override;
	//~End of UDemo_BossState Interface

	//~Labels
	virtual TCoroutine<> Label_Default() override;
	//~End of Labels

	AActor* GetMovePoint() const;

protected:
	/** Minimal distance the move point have to be at in order to consider them valid. */
	UPROPERTY(EditDefaultsOnly, Category="Movement", meta=(ClampMin="0.0"))
	float MinimumMovePointDistance = 1000.f;
};
