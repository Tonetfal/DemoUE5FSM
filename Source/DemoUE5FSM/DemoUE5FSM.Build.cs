// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DemoUE5FSM : ModuleRules
{
	public DemoUE5FSM(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
			{
				"AIModule",
				"Core",
				"CoreUObject",
				"Engine",
				"EnhancedInput" ,
				"GameplayTags",
				"InputCore",
				"PredictionSensePlus",
				"UE5Coro",
				"UE5CoroAI",
				"UE5FSM",
			});
	}
}
