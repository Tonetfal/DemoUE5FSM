#include "DemoUE5FSM/Public/AI/Demo_BossCharacter.h"

#include "AI/Demo_BossController.h"
#include "AI/StateMachine/Demo_BossState.h"
#include "Components/BoxComponent.h"
#include "DemoUE5FSM/DemoUE5FSMCharacter.h"
#include "FiniteStateMachine/FiniteStateMachine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ADemo_BossCharacter::ADemo_BossCharacter()
{
	GrabCollision = CreateDefaultSubobject<UBoxComponent>("GrabCollision");
	GrabCollision->SetupAttachment(RootComponent);

	GrabCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	GrabCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetGrabCollisionEnabled(false);

	GrabPoint = CreateDefaultSubobject<USceneComponent>("GrabPoint");
	GrabPoint->SetupAttachment(RootComponent);
}

void ADemo_BossCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GrabbedPlayerCharacter);
}

void ADemo_BossCharacter::PlayAnimation_Implementation(UAnimMontage* Animation)
{
	PlayAnimMontage(Animation);
}

bool ADemo_BossCharacter::PlayAnimation_Validate(UAnimMontage* Animation)
{
	return true;
}


void ADemo_BossCharacter::SetGrabCollisionEnabled(bool bIsEnabled) const
{
	GrabCollision->SetCollisionEnabled(bIsEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
}

void ADemo_BossCharacter::SetGrabbedPlayer(ADemoUE5FSMCharacter* PlayerCharacter)
{
	if (HasAuthority())
	{
		ADemoUE5FSMCharacter* OldValue = GrabbedPlayerCharacter.Get();
		GrabbedPlayerCharacter = PlayerCharacter;
		OnRep_GrabbedPlayerCharacter(OldValue);

		if (GrabbedPlayerCharacter.IsValid())
		{
			OnPlayerGrabbed.Broadcast();
		}
	}
}

ADemoUE5FSMCharacter* ADemo_BossCharacter::GetGrabbedPlayer() const
{
	return GrabbedPlayerCharacter.Get();
}

void ADemo_BossCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	GrabCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnGrabCollionBeginOverlap);
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

void ADemo_BossCharacter::OnRep_GrabbedPlayerCharacter(TWeakObjectPtr<ADemoUE5FSMCharacter> OldValue)
{
	if (OldValue.IsValid())
	{
		OldValue->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		OldValue->GetCharacterMovement()->SetDefaultMovementMode();
	}

	if (GrabbedPlayerCharacter.IsValid())
	{
		GrabbedPlayerCharacter->GetCharacterMovement()->DisableMovement();
		GrabbedPlayerCharacter->AttachToComponent(GrabPoint, FAttachmentTransformRules::SnapToTargetIncludingScale);
	}
}

void ADemo_BossCharacter::OnGrabCollionBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor->IsA(ADemoUE5FSMCharacter::StaticClass()))
	{
		SetGrabbedPlayer(Cast<ADemoUE5FSMCharacter>(OtherActor));
	}
}