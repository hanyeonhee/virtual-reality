// Copyright yeonheehan. All Rights Reserved.

using UnrealBuildTool;

public class yeonheehan : ModuleRules
{
    public yeonheehan(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",

            // Needed for UExponentialHeightFogComponent API
            "RenderCore",
            "Renderer",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "HeadMountedDisplay",   // VR template requirement
            "XRBase",
        });
    }
}
