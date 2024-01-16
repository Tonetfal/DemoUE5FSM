// Author: Antonio Sidenko (Tonetfal), January 2024

#include "Senses/Configs/AISenseConfig_PredictionPlus.h"

#include "Senses/AISensePredictionPlus.h"

UAISenseConfig_PredictionPlus::UAISenseConfig_PredictionPlus()
{
	Implementation = UAISense_PredictionPlus::StaticClass();
}

TSubclassOf<UAISense> UAISenseConfig_PredictionPlus::GetSenseImplementation() const
{
	return Implementation;
}
