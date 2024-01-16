// Author: Antonio Sidenko (Tonetfal), January 2024

#include "Senses/AISensePredictionPlus.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionListenerInterface.h"
#include "Perception/AIPerceptionSystem.h"

void UAISense_PredictionPlus::RequestPawnPredictionPlusEvent(
	APawn* Requestor, AActor* PredictedActor, float PredictionTime)
{
	UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(Requestor);
	AController* Controller = Requestor->GetController();
	if (IsValid(PerceptionSystem) && IsValid(Controller))
	{
		const FAIPredictionEvent Event(Controller, PredictedActor, PredictionTime);
		PerceptionSystem->OnEvent<FAIPredictionEvent, UAISense_PredictionPlus>(Event);
	}
}

float UAISense_PredictionPlus::Update()
{
	// Don't call the base implementation
	// return Super::Update();

	AIPerception::FListenerMap& ListenersMap = *GetListeners();

	for (const FAIPredictionEvent& Event : RegisteredEvents)
	{
		if (!IsValid(Event.Requestor))
		{
			continue;
		}

		if (!IsValid(Event.PredictedActor))
		{
			continue;
		}

		auto* PerceptionListener = Cast<IAIPerceptionListenerInterface>(Event.Requestor);
		if (!PerceptionListener)
		{
			continue;
		}

		const UAIPerceptionComponent* PerceptionComponent = PerceptionListener->GetPerceptionComponent();
		if (!IsValid(PerceptionComponent))
		{
			continue;
		}

		const FPerceptionListenerID ListenerID = PerceptionComponent->GetListenerId();
		if (!ListenersMap.Contains(ListenerID))
		{
			continue;
		}

		FPerceptionListener& Listener = ListenersMap[ListenerID];
		if (!Listener.HasSense(GetSenseID()))
		{
			continue;
		}

		const FVector PredictedLocation = GetPredictionLocation(Event);
		const FAIStimulus Stimulus(*this, 1.f, PredictedLocation, Listener.CachedLocation);
		Listener.RegisterStimulus(Event.PredictedActor, Stimulus);
	}

	RegisteredEvents.Reset();

	return SuspendNextUpdate;
}

FVector UAISense_PredictionPlus::GetPredictionLocation(const FAIPredictionEvent& Event) const
{
	const FVector TargetLocation = Event.PredictedActor->GetActorLocation();
	const FVector TargetVelocity = Event.PredictedActor->GetVelocity();
	const FVector Displacement = TargetVelocity * Event.TimeToPredict;
	const FVector DefaultPredictLocation = TargetLocation + Displacement;

	const UWorld* World = GetWorld();

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Event.PredictedActor);

	FHitResult Hit;
	World->LineTraceSingleByChannel(Hit, TargetLocation, DefaultPredictLocation, CollisionChannel, QueryParams);

	if (!Hit.bBlockingHit)
	{
		return DefaultPredictLocation;
	}

	// Don't predict if anything is on the way
	const FVector ScaledNormal = Hit.ImpactNormal * BounceOffWall;
	const FVector PredictLocation = Hit.Location + ScaledNormal;
	return PredictLocation;
}
