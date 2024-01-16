#pragma once

#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"

#include "Demo_TaggedActor.generated.h"

class UBillboardComponent;

UCLASS(Blueprintable, BlueprintType)
class DEMOUE5FSM_API ADemo_TaggedActor
	: public AActor
	, public IGameplayTagAssetInterface
{
	GENERATED_BODY()

public:
	ADemo_TaggedActor();

	//~IGameplayTagAssetInterface Interface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface Interface

protected:
	//~AActor Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor Interface

public:
	UPROPERTY(EditAnywhere, Category="Tagged Actor")
	FGameplayTagContainer AssetTags;

protected:
	UPROPERTY(VisibleDefaultsOnly)
	TObjectPtr<UBillboardComponent> BillboardComponent = nullptr;
};
