// Author: Antonio Sidenko (Tonetfal), January 2024

#pragma once

#include "Perception/AISense_Prediction.h"

#include "AISensePredictionPlus.generated.h"

/**
 * Custom prediction AI sense.
 *
 * Predicts the player movement using a trace, meaning that it will not predict a location inside/behind a wall.
 */
UCLASS(Config="Engine", ClassGroup="AI", MinimalAPI)
class UAISense_PredictionPlus
	: public UAISense_Prediction
{
	GENERATED_BODY()

public:
	/**
	 * Request prediction event to exactly this sense.
	 */
	UFUNCTION(BlueprintCallable, Category="AI|Perception", DisplayName="Request Pawn Prediction+ Event")
	static PREDICTIONSENSEPLUS_API void RequestPawnPredictionPlusEvent(
		APawn* Requestor, AActor* PredictedActor, float PredictionTime);

protected:
	//~UAISense_Prediction Interface
	virtual float Update() override;
	//~End of UAISense_Prediction Interface

	/**
	 * Get location the player would probably go.
	 * @param	Event data describing prediction event.
	 */
	PREDICTIONSENSEPLUS_API virtual FVector GetPredictionLocation(const FAIPredictionEvent& Event) const;

protected:
	/** Value to multiply impact normal by to add to a predicted location that hit a wall. */
	UPROPERTY(Config, BlueprintReadOnly)
	float BounceOffWall = 20.f;

	/** Collision channel to use for traces. */
	UPROPERTY(Config, BlueprintReadOnly)
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Visibility;
};
