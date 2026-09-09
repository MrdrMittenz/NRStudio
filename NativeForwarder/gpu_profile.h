#pragma once
#include <windows.h>
#include <nvapi.h>

namespace NRGpu {
enum class Profile { Unsupported, Ampere3090, Ada };
using Query = void*(__cdecl*)(unsigned);
inline Profile Select(unsigned vendor, unsigned device, unsigned architecture) {
    if (vendor != 0x10de) return Profile::Unsupported;
    if (architecture == NV_GPU_ARCHITECTURE_AD100) return Profile::Ada;
    if (device == 0x2204) return Profile::Ampere3090;
    return Profile::Unsupported;
}
// Match the game's adapter, not the first NVIDIA GPU installed in the PC.
inline unsigned Architecture(LUID target, Query query) {
    if (!query) return 0;
    auto init=reinterpret_cast<decltype(&NvAPI_Initialize)>(query(0x0150e828));
    auto enumerate=reinterpret_cast<decltype(&NvAPI_EnumLogicalGPUs)>(query(0x48b3ea59));
    auto logicalInfo=reinterpret_cast<decltype(&NvAPI_GPU_GetLogicalGpuInfo)>(query(0x842b066e));
    auto archInfo=reinterpret_cast<decltype(&NvAPI_GPU_GetArchInfo)>(query(0xd8265d24));
    if (!init || !enumerate || !logicalInfo || !archInfo || init()!=NVAPI_OK) return 0;
    NvLogicalGpuHandle handles[NVAPI_MAX_LOGICAL_GPUS]{}; NvU32 count=0;
    if (enumerate(handles,&count)!=NVAPI_OK || count>NVAPI_MAX_LOGICAL_GPUS) return 0;
    for (NvU32 i=0;i<count;i++) {
        LUID luid{}; NV_LOGICAL_GPU_DATA info{};
        info.version=NV_LOGICAL_GPU_DATA_VER;info.pOSAdapterId=&luid;
        if (logicalInfo(handles[i],&info)!=NVAPI_OK || luid.LowPart!=target.LowPart || luid.HighPart!=target.HighPart) continue;
        if (info.physicalGpuCount!=1) return 0;
        NV_GPU_ARCH_INFO arch{};arch.version=NV_GPU_ARCH_INFO_VER;
        return archInfo(info.physicalGpuHandles[0],&arch)==NVAPI_OK ? arch.architecture : 0;
    }
    return 0;
}
}
