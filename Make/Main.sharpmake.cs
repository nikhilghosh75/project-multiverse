using Sharpmake;
using System.IO;

[module: Sharpmake.Include("MultiverseSolution.sharpmake.cs")]
[module: Sharpmake.Include("Utils.sharpmake.cs")]
[module: Sharpmake.Include("ApplicationCore.sharpmake.cs")]

public static class MultiverseMain
{
    [Sharpmake.Main]
    public static void SharpmakeMain(Sharpmake.Arguments arguments)
    {
        arguments.Generate<MultiverseSolution>();
    }
}