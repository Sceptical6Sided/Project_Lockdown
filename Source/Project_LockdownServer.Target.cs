using UnrealBuildTool;
using System.Collections.Generic;

public class Project_LockdownServerTarget : TargetRules
{
    public Project_LockdownServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V2;
        ExtraModuleNames.Add("Project_Lockdown");
    }
}