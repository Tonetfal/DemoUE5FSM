// Author: Antonio Sidenko (Tonetfal), January 2024

#pragma once

#include "Perception/AISenseConfig_Prediction.h"

#include "AISenseConfig_PredictionPlus.generated.h"

/**
 * Custom config for prediction AI sense.
 */
UCLASS(MinimalAPI, DisplayName="AI Prediction+ sense config")
class UAISenseConfig_PredictionPlus
	: public UAISenseConfig_Prediction
{
	GENERATED_BODY()

public:
	UAISenseConfig_PredictionPlus();

	//~UAISenseConfig_Prediction Interface
	virtual TSubclassOf<UAISense> GetSenseImplementation() const override;
	//~End of UAISenseConfig_Prediction Interface

protected:
	/** Prediction implementation to use. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Sense")
	TSubclassOf<UAISense> Implementation = nullptr;
};
