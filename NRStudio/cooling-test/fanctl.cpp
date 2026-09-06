// Fan-only access through the installed MSI Afterburner Control SDK.
#include <windows.h>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstddef>
#include <MACMSharedMemory.h>
int main(int argc,char**argv){
 if(argc!=2&&argc!=3){puts("usage: fanctl inspect | set PERCENT | auto");return 2;}
 bool inspect=!strcmp(argv[1],"inspect"),automatic=!strcmp(argv[1],"auto");
 bool setting=!strcmp(argv[1],"set")&&argc==3;
 if(!inspect&&!automatic&&!setting)return 2;
 char*end=nullptr;long speed=setting?strtol(argv[2],&end,10):0;
 if(setting&&(!end||*end||speed<50||speed>100))return 2;
 HANDLE mapping=OpenFileMappingW(inspect?FILE_MAP_READ:FILE_MAP_ALL_ACCESS,FALSE,L"MACMSharedMemory");
 if(!mapping){printf("OpenFileMapping error=%lu\n",GetLastError());return 3;}
 auto h=(MACM_SHARED_MEMORY_HEADER*)MapViewOfFile(mapping,inspect?FILE_MAP_READ:FILE_MAP_ALL_ACCESS,0,0,0);
 if(!h){CloseHandle(mapping);return 4;}
 HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Global\\Access_MACMSharedMemory");
 if(!mutex){UnmapViewOfFile(h);CloseHandle(mapping);return 5;}
 DWORD wait=WaitForSingleObject(mutex,2000);
 if(wait!=WAIT_OBJECT_0&&wait!=WAIT_ABANDONED){CloseHandle(mutex);UnmapViewOfFile(h);CloseHandle(mapping);return 6;}
 printf("signature=%08lx version=%08lx header=%lu entries=%lu entry_bytes=%lu command=%08lx\n",h->dwSignature,h->dwVersion,h->dwHeaderSize,h->dwNumGpuEntries,h->dwGpuEntrySize,h->dwCommand);
 int result=0;
 MEMORY_BASIC_INFORMATION region{};VirtualQuery(h,&region,sizeof(region));
 size_t total=size_t(h->dwHeaderSize)+size_t(h->dwNumGpuEntries)*h->dwGpuEntrySize;
 if(h->dwSignature!='MACM'||h->dwVersion!=0x20003||h->dwHeaderSize<sizeof(*h)||h->dwNumGpuEntries!=1||h->dwGpuEntrySize<sizeof(MACM_SHARED_MEMORY_GPU_ENTRY)||total>region.RegionSize||h->dwCommand){puts("Unsupported/busy control layout");result=7;}
 else{
  auto gpu=(MACM_SHARED_MEMORY_GPU_ENTRY*)((BYTE*)h+h->dwHeaderSize);
  printf("fan_percent=%lu fan_flags=%lu min=%lu max=%lu supported=%d\n",gpu->dwFanSpeedCur,gpu->dwFanFlagsCur,gpu->dwFanSpeedMin,gpu->dwFanSpeedMax,(gpu->dwFlags&MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_FAN_SPEED)!=0);
  if(!inspect){
   if(strncmp(gpu->szGpuId,"VEN_10DE&DEV_2204&",strlen("VEN_10DE&DEV_2204&"))){puts("Unexpected GPU; refusing fan command");ReleaseMutex(mutex);CloseHandle(mutex);UnmapViewOfFile(h);CloseHandle(mapping);return 10;}
   if(!(gpu->dwFlags&MACM_SHARED_MEMORY_GPU_ENTRY_FLAG_FAN_SPEED)||(!automatic&&(speed<gpu->dwFanSpeedMin||speed>gpu->dwFanSpeedMax))){puts("Fan setting unsupported");result=8;}
   else{
    if(automatic)gpu->dwFanFlagsCur|=MACM_SHARED_MEMORY_GPU_ENTRY_FAN_FLAG_AUTO;
    else{gpu->dwFanSpeedCur=(DWORD)speed;gpu->dwFanFlagsCur&=~MACM_SHARED_MEMORY_GPU_ENTRY_FAN_FLAG_AUTO;}
    h->dwCommand=MACM_SHARED_MEMORY_COMMAND_FLUSH;
   }
  }
 }
 ReleaseMutex(mutex);
 if(!inspect&&!result){
  bool completed=false;
  for(unsigned i=0;i<120;i++){
   Sleep(50);DWORD acquired=WaitForSingleObject(mutex,1000);
   if(acquired!=WAIT_OBJECT_0&&acquired!=WAIT_ABANDONED)break;
   completed=h->dwCommand==0;ReleaseMutex(mutex);if(completed)break;
  }
  if(!completed){puts("Command acknowledgement timed out; inspect actual state");result=9;}
  else puts("Fan command acknowledged; verify fan telemetry");
 }
 CloseHandle(mutex);UnmapViewOfFile(h);CloseHandle(mapping);return result;
}
