using UnrealBuildTool;
 
public class FogOfWar : ModuleRules
{
	public FogOfWar(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "RenderCore", "RHI" });
 
		PublicIncludePaths.AddRange(new string[] {"FogOfWar/Public"});
		PrivateIncludePaths.AddRange(new string[] {"FogOfWar/Private"});
	}
}