#include "Gameplay/Demo_TaggedActorsCollection.h"

#include "Gameplay/Demo_TaggedActor.h"

void UDemo_TaggedActorsCollection::RegisterActor(ADemo_TaggedActor* Actor)
{
#ifdef WITH_EDITOR
	ensure(!Data.Contains(Actor));
#endif

	Data.AddUnique(Actor);
}

void UDemo_TaggedActorsCollection::UnregisterActor(ADemo_TaggedActor* Actor)
{
#ifdef WITH_EDITOR
	ensure(Data.Contains(Actor));
#endif

	Data.RemoveSingle(Actor);
}

TArray<ADemo_TaggedActor*> UDemo_TaggedActorsCollection::QueryActors(
	const FGameplayTagContainer& Tags, EDemo_GameplayTagQueryType QueryType) const
{
	TArray<ADemo_TaggedActor*> ReturnValue;

	for (TWeakObjectPtr<ADemo_TaggedActor> Actor : Data)
	{
		if (ensure(Actor.IsValid()))
		{
			FGameplayTagContainer ActorTags;
			Actor->GetOwnedGameplayTags(ActorTags);

			if (CompareTags(Tags, ActorTags, QueryType))
			{
				ReturnValue.Add(Actor.Get());
			}
		}
	}

	return ReturnValue;
}

bool UDemo_TaggedActorsCollection::CompareTags(const FGameplayTagContainer& QueryRequest,
	const FGameplayTagContainer& SourceTags, EDemo_GameplayTagQueryType QueryType) const
{
	switch (QueryType)
	{
	case EDemo_GameplayTagQueryType::AnyTagsMatch: return SourceTags.HasAnyExact(QueryRequest);
	case EDemo_GameplayTagQueryType::AllTagsMatch: return SourceTags.HasAllExact(QueryRequest);

	default:
		checkNoEntry();
		return false;
	}
}
