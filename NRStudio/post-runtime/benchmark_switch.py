"""Select a kernel in a running benchmark-enabled forwarder; NR stays enabled."""
import argparse
import ctypes
from ctypes import wintypes

api=ctypes.WinDLL('kernel32',use_last_error=True)
api.OpenEventW.argtypes=[wintypes.DWORD,wintypes.BOOL,wintypes.LPCWSTR]
api.OpenEventW.restype=wintypes.HANDLE
api.SetEvent.argtypes=[wintypes.HANDLE];api.SetEvent.restype=wintypes.BOOL
api.ResetEvent.argtypes=[wintypes.HANDLE];api.ResetEvent.restype=wintypes.BOOL
api.CloseHandle.argtypes=[wintypes.HANDLE]
def select(pid,mode):
    if mode not in ('original','optimized') or pid<=0:raise ValueError('Invalid PID or mode')
    event=api.OpenEventW(2,False,fr'Local\NRStudio.PostOriginal.{pid}')
    if not event:raise ctypes.WinError(ctypes.get_last_error())
    try:
        if not (api.SetEvent(event) if mode=='original' else api.ResetEvent(event)):
            raise ctypes.WinError(ctypes.get_last_error())
    finally:api.CloseHandle(event)
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pid',type=int);parser.add_argument('mode',choices=['original','optimized'])
    args=parser.parse_args();select(args.pid,args.mode)
    print(f'Requested {args.mode} kernel in PID {args.pid}; confirm mode in dlssnr-native.log')
