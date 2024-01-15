// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DemoUE5FSM : ModuleRules
{
	public DemoUE5FSM(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
