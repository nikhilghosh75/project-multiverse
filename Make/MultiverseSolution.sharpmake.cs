using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("ApplicationCore.sharpmake.cs")]
[module: Sharpmake.Include("Utils.sharpmake.cs")]
[module: Sharpmake.Include("Renderer.sharpmake.cs")]
[module: Sharpmake.Include("Game.sharpmake.cs")]
[module: Sharpmake.Include("ImGui.sharpmake.cs")]

[Generate]
public class MultiverseSolution : Solution
{
    public MultiverseSolution()
    {
        Name = "Project Multiverse";

        // Define solution platforms and configurations
        AddTargets(new Target(
            Platform.win64,  // Target Windows x64
            DevEnv.vs2022,   // Visual Studio 2022
            Optimization.Debug | Optimization.Release // Configurations
        ));
    }

    [Configure]
    public void ConfigureAll(Solution.Configuration conf, Target target)
    {
        conf.SolutionPath = @"[solution.SharpmakeCsPath]/../Generated/";
        
        conf.AddProject<Utils>(target);
        conf.AddProject<ApplicationCore>(target);
        conf.AddProject<Renderer>(target);
        conf.AddProject<Game>(target);

        conf.AddProject<ImGui>(target, true, "Third Party");
    }
}