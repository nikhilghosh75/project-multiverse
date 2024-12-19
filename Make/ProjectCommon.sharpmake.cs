using Sharpmake;
using System.Collections.Generic;

[Generate]
public class ProjectCommon : Project
{
    public ProjectCommon()
    {
        Name = "Project Common";

        AddTargets(new Target(
            Platform.win64, 
            DevEnv.vs2022, 
            Optimization.Debug | Optimization.Release
        ));
    }

    [Configure]
    public void ConfigureAll(Project.Configuration conf, Target target)
    {
        conf.ProjectPath = @"[project.SharpmakeCsPath]/../Generated";
        
        // Add Utils to all project include paths
        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Source/Utils");

        // Define preprocessor macros
        conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");

        // Set output directory
        conf.TargetPath = @"[project.SharpmakeCsPath]/../Bin/[target.Platform]/[target.Optimization]";

        // Enable C++20
        conf.Options.Add(Options.Vc.Compiler.CppLanguageStandard.CPP20);

        // Set working directory (for the debugger)
        conf.VcxprojUserFile = new Project.Configuration.VcxprojUserFileSettings
        {
            LocalDebuggerWorkingDirectory = @"[project.SharpmakeCsPath]/../"
        };
    }
}