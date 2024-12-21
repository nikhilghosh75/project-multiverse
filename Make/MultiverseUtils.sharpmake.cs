using System;
using System.IO;

public static class MultiverseUtils
{
    public static string GetVulkanSDKPath()
    {
        return Environment.GetEnvironmentVariable("VULKAN_SDK");
    }
}