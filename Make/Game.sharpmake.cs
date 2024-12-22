using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("ApplicationCore.sharpmake.cs")]
[module: Sharpmake.Include("MultiverseUtils.sharpmake.cs")]
[module: Sharpmake.Include("Renderer.sharpmake.cs")]
[module: Sharpmake.Include("Utils.sharpmake.cs")]
[module: Sharpmake.Include("ProjectCommon.sharpmake.cs")]

[Generate]
public class Game : ProjectCommon
{
    public Game()
    {
        Name = "Game";
        SourceRootPath = @"[project.SharpmakeCsPath]/../Source/Game";

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
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/glm");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/stb");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/freetype/include");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Source/ApplicationCore");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Source/Renderer");
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Source/Game");

        // Define preprocessor macros
        conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");

        conf.Output = Configuration.OutputType.Exe;

        // Add Dependencies
        conf.AddPublicDependency<Utils>(target);
        conf.AddPublicDependency<ApplicationCore>(target);
        conf.AddPublicDependency<Renderer>(target);
    }
}