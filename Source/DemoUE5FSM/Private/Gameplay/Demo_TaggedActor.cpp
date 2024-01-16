#include "Gameplay/Demo_TaggedActor.h"

#include "Components/BillboardComponent.h"
#include "Gameplay/Demo_TaggedActorsCollection.h"

ADemo_TaggedActor::ADemo_TaggedActor()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	BillboardComponent = CreateDefaultSubobject<UBillboardComponent>("Billboard");
	SetRootComponent(BillboardComponent);
}

void ADemo_TaggedActor::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = AssetTags;
}

void ADemo_TaggedActor::BeginPlay()
{
	Super::BeginPlay();

	auto& TaggedActorsCollection = *GetWorld()->GetSubsystem<UDemo_TaggedActorsCollection>();
	TaggedActorsCollection.RegisterActor(this);
}

void ADemo_TaggedActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	auto& TaggedActorsCollection = *GetWorld()->GetSubsystem<UDemo_TaggedActorsCollection>();
	TaggedActorsCollection.UnregisterActor(this);

	Super::EndPlay(EndPlayReason);
}
