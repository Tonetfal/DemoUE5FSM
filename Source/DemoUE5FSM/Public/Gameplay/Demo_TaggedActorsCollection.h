#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"

#include "Demo_TaggedActorsCollection.generated.h"

class ADemo_TaggedActor;

UENUM(BlueprintType)
enum class EDemo_GameplayTagQueryType : uint8
{
	AnyTagsMatch,
	AllTagsMatch,
};

/**
 *
 */
UCLASS()
class DEMOUE5FSM_API UDemo_TaggedActorsCollection
	: public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void RegisterActor(ADemo_TaggedActor* Actor);
	void UnregisterActor(ADemo_TaggedActor* Actor);

	UFUNCTION(BlueprintCallable)
	TArray<ADemo_TaggedActor*> QueryActors(const FGameplayTagContainer& Tags,
		EDemo_GameplayTagQueryType QueryType) const;

private:
	bool CompareTags(const FGameplayTagContainer& QueryRequest, const FGameplayTagContainer& SourceTags,
		EDemo_GameplayTagQueryType QueryType) const;

private:
	TArray<TWeakObjectPtr<ADemo_TaggedActor>> Data;
};
