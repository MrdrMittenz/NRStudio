using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;

namespace NRStudio {
 public sealed class GpuSample {
  public string Index, Uuid, Name, Driver;
  public DateTime CapturedUtc;
  public double? TotalMiB, UsedMiB, FreeMiB, Utilization, Temperature, Power, PowerLimit, GraphicsClock, MemoryClock;
  public override string ToString() { return "GPU "+Index+"  /  "+Name; }
  public static string Number(double? value,string unit,string format="0") { return value.HasValue?value.Value.ToString(format,CultureInfo.InvariantCulture)+unit:"Unavailable"; }
  public static string GiB(double? value) { return Number(value/1024," GiB","0.00"); }
  public string Describe() {
   return ToString()+"\r\nDriver "+Driver+"  /  "+Uuid+"\r\n"+
    "VRAM used: "+GiB(UsedMiB)+"   /   Free: "+GiB(FreeMiB)+"   /   Total: "+GiB(TotalMiB)+"\r\n"+
    "GPU load: "+Number(Utilization,"%")+"   /   Temperature: "+Number(Temperature," C")+"\r\n"+
    "Power: "+Number(Power," W","0.0")+"   /   Power limit: "+Number(PowerLimit," W","0.0")+"\r\n"+
    "Graphics clock: "+Number(GraphicsClock," MHz")+"   /   Memory clock: "+Number(MemoryClock," MHz");
  }
 }
 public static class GpuMonitor {
  // Only a read-only query. Use an installed NVIDIA binary, never a game-folder executable.
  const string Query="--query-gpu=index,uuid,name,driver_version,memory.total,memory.used,memory.free,utilization.gpu,temperature.gpu,power.draw,power.limit,clocks.current.graphics,clocks.current.memory --format=csv,noheader,nounits";
  public static List<GpuSample> Read() {
   string smi=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.System),"nvidia-smi.exe");
   if(!File.Exists(smi))smi=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles),@"NVIDIA Corporation\NVSMI\nvidia-smi.exe");
   if(!File.Exists(smi))throw new IOException("NVIDIA monitoring is unavailable. Install a supported NVIDIA driver to read these counters.");
   using(var p=Process.Start(new ProcessStartInfo(smi,Query){UseShellExecute=false,CreateNoWindow=true,RedirectStandardOutput=true,RedirectStandardError=true})) {
    var output=p.StandardOutput.ReadToEndAsync();var error=p.StandardError.ReadToEndAsync();
    if(!p.WaitForExit(5000)){try{p.Kill();}catch(InvalidOperationException){}throw new IOException("GPU query timed out. The last reading is no longer live.");}
    if(p.ExitCode!=0)throw new IOException("GPU query failed: "+error.Result.Trim());
    return Parse(output.Result,DateTime.UtcNow);
   }
  }
  static double? Metric(string value,double minimum=0,double maximum=double.MaxValue) {
   double n;
   return double.TryParse(value,NumberStyles.Float,CultureInfo.InvariantCulture,out n)&&!double.IsNaN(n)&&!double.IsInfinity(n)&&n>=minimum&&n<=maximum?(double?)n:null;
  }
  static string[] Columns(string line) {
   var columns=new List<string>();var value=new StringBuilder();bool quoted=false;
   for(int i=0;i<line.Length;i++) {
    char ch=line[i];
    if(ch=='"') {if(quoted && i+1<line.Length && line[i+1]=='"'){value.Append('"');i++;}else quoted=!quoted;}
    else if(ch==',' && !quoted){columns.Add(value.ToString().Trim());value.Clear();}
    else value.Append(ch);
   }
   if(quoted)throw new IOException("GPU query returned an incomplete CSV row.");
   columns.Add(value.ToString().Trim());return columns.ToArray();
  }
  public static List<GpuSample> Parse(string csv,DateTime capturedUtc) {
   var result=new List<GpuSample>();var ids=new HashSet<string>(StringComparer.Ordinal);
   foreach(string line in csv.Split(new[]{'\r','\n'},StringSplitOptions.RemoveEmptyEntries)) {
    if(string.IsNullOrWhiteSpace(line))continue;
    var c=Columns(line);int index;
    if(c.Length!=13 || !int.TryParse(c[0],NumberStyles.None,CultureInfo.InvariantCulture,out index) || index<0 || !c[1].StartsWith("GPU-",StringComparison.Ordinal) || !ids.Add(c[1]) || c[2].Length==0)
     throw new IOException("GPU query returned an unsupported format. No live readings are available.");
    var sample=new GpuSample {Index=c[0],Uuid=c[1],Name=c[2],Driver=c[3],CapturedUtc=capturedUtc,
     TotalMiB=Metric(c[4],1),UsedMiB=Metric(c[5]),FreeMiB=Metric(c[6]),Utilization=Metric(c[7],0,100),Temperature=Metric(c[8],-100,200),
     Power=Metric(c[9]),PowerLimit=Metric(c[10]),GraphicsClock=Metric(c[11]),MemoryClock=Metric(c[12])};
    if(sample.TotalMiB.HasValue) {
     if(sample.UsedMiB>sample.TotalMiB)sample.UsedMiB=null;
     if(sample.FreeMiB>sample.TotalMiB)sample.FreeMiB=null;
    }
    result.Add(sample);
   }
   if(result.Count==0)throw new IOException("NVIDIA reported no GPU readings.");
   return result;
  }
 }
}
