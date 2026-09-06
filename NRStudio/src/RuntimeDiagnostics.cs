using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;

namespace NRStudio {
 public static class RuntimeDiagnostics {
  [DllImport("kernel32.dll")] static extern ulong GetTickCount64();
  public static string Signature(string path) {
   if(!File.Exists(path))return "Model signature: file missing.";
   try {
    // Pass the filename as data, never as PowerShell command text.
    string script="$ErrorActionPreference='Stop'; $s=Get-AuthenticodeSignature -LiteralPath $env:NRSTUDIO_SIGNATURE_PATH; [Console]::WriteLine('Model Authenticode: '+$s.Status); if($s.SignerCertificate){[Console]::WriteLine('Embedded signer: '+$s.SignerCertificate.Subject)}";
    string powershell=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System),@"WindowsPowerShell\v1.0\powershell.exe");
    var start=new ProcessStartInfo(powershell,"-NoProfile -NonInteractive -EncodedCommand "+Convert.ToBase64String(Encoding.Unicode.GetBytes(script))){UseShellExecute=false,CreateNoWindow=true,RedirectStandardOutput=true,RedirectStandardError=true};
    start.EnvironmentVariables["NRSTUDIO_SIGNATURE_PATH"]=Path.GetFullPath(path);
    using(var p=Process.Start(start)) {
     var output=p.StandardOutput.ReadToEndAsync();var error=p.StandardError.ReadToEndAsync();
     if(!p.WaitForExit(10000)){p.Kill();return "Model signature: check timed out; authenticity not established.";}
     if(p.ExitCode!=0)return "Model signature: could not check; authenticity not established. "+error.Result.Trim();
     return output.Result.Trim()+"\nA matching package hash is not proof of a valid NVIDIA signature or official DLSS 5 support.";
    }
   }catch(Exception e){return "Model signature: check unavailable. "+e.Message;}
  }
  public static string DescribeEvaluation(string tail, int currentPid, ulong uptime) {
   var entries=Regex.Matches(tail,@"(?m)^pid=(\d+) tick=(\d+) evaluate frame=(\d+) result=([0-9a-fA-F]{8}) size=(\d+x\d+) guides=(\d+x\d+) reset=([01])\r?$");
   if(entries.Count==0)return "No native evaluation evidence in the log tail.";
   var entry=entries[entries.Count-1];
   string summary="PID "+entry.Groups[1].Value+", frame "+entry.Groups[3].Value+", result "+entry.Groups[4].Value+", model "+entry.Groups[5].Value+", guides "+entry.Groups[6].Value;
   ulong tick;int pid;
   bool recent=int.TryParse(entry.Groups[1].Value,out pid) && pid==currentPid && currentPid>0 && ulong.TryParse(entry.Groups[2].Value,out tick) && tick<=uptime && uptime-tick<=10000;
   bool success=string.Equals(entry.Groups[4].Value,"00000001",StringComparison.OrdinalIgnoreCase);
   return (recent?"Recent native evaluation: ":"Historical native evaluation (not live confirmation): ")+(success?"success. ":"FAILED. ")+summary+". This does not measure FPS or official feature parity.";
  }
  public static string Evaluation(string exe) {
   string path=Path.Combine(Path.GetDirectoryName(exe),"dlssnr-native.log");
   if(!File.Exists(path))return "No native evaluation log yet.";
   try {
    string tail;using(var file=new FileStream(path,FileMode.Open,FileAccess.Read,FileShare.ReadWrite|FileShare.Delete)) {
     bool partial=file.Length>65536;file.Seek(Math.Max(0,file.Length-65536),SeekOrigin.Begin);
     using(var reader=new StreamReader(file)){if(partial)reader.ReadLine();tail=reader.ReadToEnd();}
    }
    int currentPid=0;DateTime written=File.GetLastWriteTimeUtc(path);
    foreach(var p in Process.GetProcessesByName(Path.GetFileNameWithoutExtension(exe)))using(p) {
     try {if(string.Equals(p.MainModule.FileName,Path.GetFullPath(exe),StringComparison.OrdinalIgnoreCase) && written>=p.StartTime.ToUniversalTime() && DateTime.UtcNow-written<=TimeSpan.FromSeconds(10))currentPid=p.Id;}catch(System.ComponentModel.Win32Exception){}catch(InvalidOperationException){}
    }
    return DescribeEvaluation(tail,currentPid,GetTickCount64());
   }catch(Exception e){return "Native evidence could not be read: "+e.Message;}
  }
 }
}
