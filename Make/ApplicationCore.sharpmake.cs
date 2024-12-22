using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("MultiverseUtils.sharpmake.cs")]
[module: Sharpmake.Include("Utils.sharpmake.cs")]
[module: Sharpmake.Include("ImGui.sharpmake.cs")]
[module: Sharpmake.Include("ProjectCommon.sharpmake.cs")]

[Generate]
public class ApplicationCore: ProjectCommon
{
    public ApplicationCore()
    {
        Name = "ApplicationCore";
        SourceRootPath = @"[project.SharpmakeCsPath]/../Source/ApplicationCore";

        AddTargets(new Target(
            Platform.win64, 
            DevEnv.vs2022, 
            Optimization.Debug | Optimization.Release
        ));
    }

    [Configure]
    public void ConfigureAll(Project.Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.IncludePaths.Add(MultiverseUtils.GetVulkanSDKPath() + "/Include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/glm");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui");

        conf.LibraryPaths.Add(MultiverseUtils.GetVulkanSDKPath() + "/Lib");

        conf.LibraryFiles.Add("vulkan-1.lib");

        // Define preprocessor macros
        conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");

        conf.Output = Configuration.OutputType.Lib;

        // Add Dependencies
        conf.AddPublicDependency<Utils>(target);
        conf.AddPublicDependency<ImGui>(target);
    }
}