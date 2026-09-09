using System;
using System.Diagnostics;
using System.IO;
using Microsoft.Win32;
namespace NRStudio {
 public static class Prerequisites {
  public static string SupportSummary(string info) {
   bool ada=System.Text.RegularExpressions.Regex.IsMatch(info??"",@"\bRTX\s+40\d{2}\b",System.Text.RegularExpressions.RegexOptions.IgnoreCase);
   return ada ? "RTX 40-series: Ada runtime available; hardware validation pending. Performance versus RTX 3090 depends on GPU model, VRAM, resolution and game."
    : "Runtime profiles: RTX 3090 and RTX 40-series (Ada). RTX 3090 tested with driver 616.64; Ada hardware validation pending.";
  }
  public static string GpuInfo() {
   string smi=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System),"nvidia-smi.exe");
   if(!File.Exists(smi))smi=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),@"NVIDIA Corporation\NVSMI\nvidia-smi.exe");
   if(!File.Exists(smi))return "NVIDIA driver not detected";
   try {using(var p=Process.Start(new ProcessStartInfo(smi,"--query-gpu=name,driver_version,memory.total --format=csv,noheader"){UseShellExecute=false,CreateNoWindow=true,RedirectStandardOutput=true,RedirectStandardError=true})) {
    var read=p.StandardOutput.ReadToEndAsync();if(!p.WaitForExit(5000)){p.Kill();return "NVIDIA driver check timed out";}return p.ExitCode==0?read.Result.Trim():"NVIDIA driver could not be queried";
   }}catch(Exception e){return "NVIDIA driver check: "+e.Message;}
  }
  public static bool NeedsDriver(string info) {
   foreach(string row in info.Split('\n')) {var columns=row.Split(',');Version version;if(columns.Length>1 && Version.TryParse(columns[1].Trim(),out version) && version>=new Version(616,64))return false;}
   return true;
  }
  public static bool NeedsVC() {
   using(var root=RegistryKey.OpenBaseKey(RegistryHive.LocalMachine,RegistryView.Registry32))using(var key=root.OpenSubKey(@"SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64")) {
    Version version;return key==null || Convert.ToInt32(key.GetValue("Installed",0))!=1 || !Version.TryParse(Convert.ToString(key.GetValue("Version","")).TrimStart('v'),out version) || version<new Version(14,51,36231);
   }
  }
  public static int InstallVC(string app) {using(var p=Process.Start(new ProcessStartInfo(Path.Combine(app,@"prerequisites\vc_redist.x64.exe"),"/install /passive /norestart"){UseShellExecute=true,Verb="runas"})) {p.WaitForExit();return p.ExitCode;}}
  public static void InstallDriver(string app) {Process.Start(new ProcessStartInfo(Path.Combine(app,@"prerequisites\NVIDIA-616.64.exe")){UseShellExecute=true,Verb="runas"});}
 }
}
