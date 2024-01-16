// Author: Antonio Sidenko (Tonetfal), January 2024

using UnrealBuildTool;

public class PredictionSensePlus : ModuleRules
{
	public PredictionSensePlus(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"AIModule",
				"Core",
				"CoreUObject",
				"Engine",
			}
		);
	}
}
