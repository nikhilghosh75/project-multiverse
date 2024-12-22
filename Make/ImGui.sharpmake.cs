using Sharpmake;
using System.Collections.Generic;

[module: Sharpmake.Include("ProjectCommon.sharpmake.cs")]
[module: Sharpmake.Include("MultiverseUtils.sharpmake.cs")]

[Generate]
public class ImGui : ProjectCommon
{
    public ImGui()
    {
        Name = "ImGui";
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/imgui.cpp");
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/imgui_demo.cpp");
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/imgui_draw.cpp");
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/imgui_tables.cpp");
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/imgui_widgets.cpp");
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/backends/imgui_impl_vulkan.cpp");
        SourceFiles.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui/backends/imgui_impl_win32.cpp");

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

        conf.IncludePaths.Add(@"[project.SharpmakeCsPath]/../Vendor/imgui");
        conf.IncludePaths.Add(MultiverseUtils.GetVulkanSDKPath() + "/Include");

        // Define preprocessor macros
        conf.Defines.Add("_CRT_SECURE_NO_WARNINGS");

        conf.Output = Configuration.OutputType.Lib;
    }
}