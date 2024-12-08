#include "Registry.h"

#include <Windows.h>

std::string Registry::QueryValue(const std::string& subkey, const std::string& valueName)
{
    HKEY hKey;
    // Open the registry key

    LONG result = RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey.c_str(), 0, KEY_READ, &hKey);

    char value[512]; // Buffer for the value
    DWORD valueSize = sizeof(value);
    DWORD valueType;

    // Query the registry value
    result = RegQueryValueExA(hKey, valueName.c_str(), nullptr, &valueType, reinterpret_cast<LPBYTE>(value), &valueSize);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS)
    {
        return std::string();
    }

    return std::string(value, valueSize - 1);
}
