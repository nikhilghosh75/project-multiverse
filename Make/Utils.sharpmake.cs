using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("ProjectCommon.sharpmake.cs")]

[Generate]
public class Utils: ProjectCommon
{
    public Utils()
    {
        Name = "Utils";
        SourceRootPath = @"[project.SharpmakeCsPath]/../Source/Utils";

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

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/glm");

        // Define preprocessor macros
        conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");

        conf.Output = Configuration.OutputType.Lib;
    }
}