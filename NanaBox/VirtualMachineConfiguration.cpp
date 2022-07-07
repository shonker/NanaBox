﻿/*
 * PROJECT:   NanaBox
 * FILE:      VirtualMachineConfiguration.cpp
 * PURPOSE:   Implementation for the Virtual Machine Configuration
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#include "VirtualMachineConfiguration.h"

#include <json.hpp>

NLOHMANN_JSON_SERIALIZE_ENUM(NanaBox::GuestType, {
    { NanaBox::GuestType::Unknown, nullptr },
    { NanaBox::GuestType::Windows, "Windows" },
    { NanaBox::GuestType::Linux, "Linux" }
})

NLOHMANN_JSON_SERIALIZE_ENUM(NanaBox::GpuAssignmentMode, {
    { NanaBox::GpuAssignmentMode::Disabled, nullptr },
    { NanaBox::GpuAssignmentMode::Default, "Default" },
    { NanaBox::GpuAssignmentMode::List, "List" },
    { NanaBox::GpuAssignmentMode::Mirror, "Mirror" }
})

NLOHMANN_JSON_SERIALIZE_ENUM(NanaBox::ScsiDeviceType, {
    { NanaBox::ScsiDeviceType::VirtualDisk, "VirtualDisk" },
    { NanaBox::ScsiDeviceType::VirtualImage, "VirtualImage" },
    { NanaBox::ScsiDeviceType::PhysicalDevice, "PhysicalDevice" }
})

NanaBox::VirtualMachineConfiguration NanaBox::DeserializeConfiguration(
    std::string const& Configuration)
{
    nlohmann::json ParsedJson = nlohmann::json::parse(Configuration);

    nlohmann::json RootJson = ParsedJson["NanaBox"];

    if ("VirtualMachine" !=
        RootJson["Type"].get<std::string>())
    {
        throw std::exception(
            "Invalid Virtual Machine Configuration");
    }

    NanaBox::VirtualMachineConfiguration Result;

    try
    {
        Result.Version =
            RootJson["Version"].get<std::uint32_t>();
    }
    catch (...)
    {
        Result.Version = 1;
    }
    if (Result.Version < 1 || Result.Version > 1)
    {
        throw std::exception(
            "Invalid Version");
    }

    Result.GuestType =
        RootJson["GuestType"].get<NanaBox::GuestType>();

    Result.Name =
        RootJson["Name"].get<std::string>();

    try
    {
        Result.ProcessorCount =
            RootJson["ProcessorCount"].get<std::uint32_t>();
    }
    catch (...)
    {
        throw std::exception(
            "Invalid Processor Count");
    }

    try
    {
        Result.MemorySize =
            RootJson["MemorySize"].get<std::uint32_t>();
    }
    catch (...)
    {
        throw std::exception("Invalid Memory Size");
    }

    try
    {
        nlohmann::json ComPorts = RootJson["ComPorts"];

        for (nlohmann::json const& ComPort : ComPorts)
        {
            try
            {
                Result.ComPorts.push_back(
                    ComPort.get<std::string>());
            }
            catch (...)
            {

            }
        }
    }
    catch (...)
    {

    }

    try
    {
        nlohmann::json Gpu = RootJson["Gpu"];

        Result.Gpu.AssignmentMode =
            Gpu["AssignmentMode"].get<NanaBox::GpuAssignmentMode>();

        try
        {
            nlohmann::json SelectedDevices = Gpu["SelectedDevices"];

            for (nlohmann::json const& SelectedDevice : SelectedDevices)
            {
                try
                {
                    Result.Gpu.SelectedDevices.push_back(
                        SelectedDevice.get<std::string>());
                }
                catch (...)
                {

                }
            }
        }
        catch (...)
        {

        }

        if (Result.Gpu.SelectedDevices.empty() &&
            Result.Gpu.AssignmentMode == NanaBox::GpuAssignmentMode::List)
        {
            Result.Gpu.AssignmentMode = NanaBox::GpuAssignmentMode::Disabled;
        }

        if (Result.Gpu.AssignmentMode != NanaBox::GpuAssignmentMode::List)
        {
            Result.Gpu.SelectedDevices.clear();
        }
    }
    catch (...)
    {

    }

    try
    {
        nlohmann::json NetworkAdapters = RootJson["NetworkAdapters"];

        for (nlohmann::json const& NetworkAdapter : NetworkAdapters)
        {
            try
            {
                NanaBox::NetworkAdapterConfiguration Current;

                try
                {
                    Current.Enabled =
                        NetworkAdapter["Enabled"].get<bool>();
                }
                catch (...)
                {
                    Current.Enabled = false;
                }

                try
                {
                    Current.Connected =
                        NetworkAdapter["Connected"].get<bool>();
                }
                catch (...)
                {
                    Current.Connected = false;
                }

                Current.MacAddress =
                    NetworkAdapter["MacAddress"].get<std::string>();
                if (Current.MacAddress.empty())
                {
                    continue;
                }

                Result.NetworkAdapters.push_back(Current);
            }
            catch (...)
            {

            }
        }
    }
    catch (...)
    {

    }

    try
    {
        nlohmann::json ScsiDevices = RootJson["ScsiDevices"];

        for (nlohmann::json const& ScsiDevice : ScsiDevices)
        {
            NanaBox::ScsiDeviceConfiguration Current;

            try
            {
                Current.Enabled =
                    ScsiDevice["Enabled"].get<bool>();
            }
            catch (...)
            {
                Current.Enabled = false;
            }

            try
            {
                Current.Type =
                    ScsiDevice["Type"].get<NanaBox::ScsiDeviceType>();
            }
            catch (...)
            {
                continue;
            }

            Current.Path =
                ScsiDevice["Path"].get<std::string>();
            if (Current.Path.empty() &&
                Current.Type != NanaBox::ScsiDeviceType::VirtualImage)
            {
                continue;
            }

            Result.ScsiDevices.push_back(Current);
        }
    }
    catch (...)
    {

    }

    try
    {
        nlohmann::json SharedFolders = RootJson["SharedFolders"];

        for (nlohmann::json const& SharedFolder : SharedFolders)
        {
            try
            {
                NanaBox::SharedFolderConfiguration Current;

                try
                {
                    Current.Enabled =
                        SharedFolder["Enabled"].get<bool>();
                }
                catch (...)
                {
                    Current.Enabled = false;
                }

                try
                {
                    Current.ReadOnly =
                        SharedFolder["ReadOnly"].get<bool>();
                }
                catch (...)
                {
                    Current.ReadOnly = true;
                }

                Current.HostPath =
                    SharedFolder["HostPath"].get<std::string>();
                if (Current.HostPath.empty())
                {
                    continue;
                }

                Current.GuestName =
                    SharedFolder["GuestName"].get<std::string>();
                if (Current.GuestName.empty())
                {
                    continue;
                }

                Result.SharedFolders.push_back(Current);
            }
            catch (...)
            {

            }
        }
    }
    catch (...)
    {

    }

    try
    {
        Result.SecureBoot =
            RootJson["SecureBoot"].get<bool>();
    }
    catch (...)
    {
        Result.SecureBoot = false;
    }

    try
    {
        Result.Tpm =
            RootJson["Tpm"].get<bool>();
    }
    catch (...)
    {
        Result.Tpm = false;
    }

    try
    {
        Result.GuestStateFile =
            RootJson["GuestStateFile"].get<std::string>();
    }
    catch (...)
    {

    }

    try
    {
        Result.RuntimeStateFile =
            RootJson["RuntimeStateFile"].get<std::string>();
    }
    catch (...)
    {

    }

    return Result;
}

std::string NanaBox::SerializeConfiguration(
    NanaBox::VirtualMachineConfiguration const& Configuration)
{
    nlohmann::json RootJson;
    RootJson["Type"] = "VirtualMachine";
    RootJson["Version"] = Configuration.Version;
    RootJson["GuestType"] = Configuration.GuestType;
    RootJson["Name"] = Configuration.Name;
    RootJson["ProcessorCount"] = Configuration.ProcessorCount;
    RootJson["MemorySize"] = Configuration.MemorySize;
    {
        nlohmann::json ComPorts;
        for (std::string const& ComPort
            : Configuration.ComPorts)
        {
            ComPorts.push_back(ComPort);
        }
        RootJson["ComPorts"] = ComPorts;
    }
    {
        nlohmann::json Gpu;
        Gpu["AssignmentMode"] = Configuration.Gpu.AssignmentMode;
        {
            nlohmann::json SelectedDevices;
            for (std::string const& SelectedDevice
                : Configuration.Gpu.SelectedDevices)
            {
                SelectedDevices.push_back(SelectedDevice);
            }
            Gpu["SelectedDevices"] = SelectedDevices;
        }
        RootJson["Gpu"] = Gpu;
    }
    {
        nlohmann::json NetworkAdapters;
        for (NanaBox::NetworkAdapterConfiguration const& NetworkAdapter
            : Configuration.NetworkAdapters)
        {
            nlohmann::json Current;
            Current["Enabled"] = NetworkAdapter.Enabled;
            Current["Connected"] = NetworkAdapter.Connected;
            Current["MacAddress"] = NetworkAdapter.MacAddress;
            NetworkAdapters.push_back(Current);
        }
        RootJson["NetworkAdapters"] = NetworkAdapters;
    }
    {
        nlohmann::json ScsiDevices;
        for (NanaBox::ScsiDeviceConfiguration const& ScsiDevice
            : Configuration.ScsiDevices)
        {
            nlohmann::json Current;
            Current["Enabled"] = ScsiDevice.Enabled;
            Current["Type"] = ScsiDevice.Type;
            Current["Path"] = ScsiDevice.Path;
            ScsiDevices.push_back(Current);
        }
        RootJson["ScsiDevices"] = ScsiDevices;
    }
    {
        nlohmann::json SharedFolders;
        for (NanaBox::SharedFolderConfiguration const& SharedFolder
            : Configuration.SharedFolders)
        {
            nlohmann::json Current;
            Current["Enabled"] = SharedFolder.Enabled;
            Current["ReadOnly"] = SharedFolder.ReadOnly;
            Current["HostPath"] = SharedFolder.HostPath;
            Current["GuestName"] = SharedFolder.GuestName;
            SharedFolders.push_back(Current);
        }
        RootJson["SharedFolders"] = SharedFolders;
    }
    RootJson["SecureBoot"] = Configuration.SecureBoot;
    RootJson["Tpm"] = Configuration.Tpm;
    RootJson["GuestStateFile"] = Configuration.GuestStateFile;
    RootJson["RuntimeStateFile"] = Configuration.RuntimeStateFile;

    nlohmann::json Result;
    Result["NanaBox"] = RootJson;
    return Result.dump(2);
}