using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("ApplicationCore.sharpmake.cs")]
[module: Sharpmake.Include("MultiverseUtils.sharpmake.cs")]
[module: Sharpmake.Include("Utils.sharpmake.cs")]
[module: Sharpmake.Include("ProjectCommon.sharpmake.cs")]

[Generate]
public class Renderer : ProjectCommon
{
    public Renderer()
    {
        Name = "Renderer";
        SourceRootPath = @"[project.SharpmakeCsPath]/../Source/Renderer";

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
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/stb");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/freetype/include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Source/ApplicationCore");

        conf.LibraryPaths.Add(@"[project.SharpmakeCsPath]/../Vendor/freetype/objs/x64/Debug");

        conf.LibraryFiles.Add("freetype.lib");

        // Define preprocessor macros
        conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");

        conf.Output = Configuration.OutputType.Lib;

        // Add Dependencies
        conf.AddPublicDependency<Utils>(target);
        conf.AddPublicDependency<ApplicationCore>(target);
    }
}