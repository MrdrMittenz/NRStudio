using System;
using System.Globalization;
using System.IO;
using NRStudio;
class GpuMonitorTests {
 static int count;
 static void Check(bool ok,string name){if(!ok)throw new Exception(name);Console.WriteLine("PASS "+name);count++;}
 static void Reject(string text,string name){bool rejected=false;try{GpuMonitor.Parse(text,DateTime.UtcNow);}catch(IOException){rejected=true;}Check(rejected,name);}
 static void Main(string[] args) {
  var time=new DateTime(2026,9,7,12,0,0,DateTimeKind.Utc);
  const string good="0, GPU-one, NVIDIA GeForce RTX 3090, 616.64, 24576, 8300, 15800, 98, 67, 313.27, 350.00, 1815, 9751";
  var sample=GpuMonitor.Parse(good,time)[0];
  Check(sample.CapturedUtc==time && sample.Uuid=="GPU-one","sample carries timestamp and stable GPU identity");
  Check(sample.FreeMiB==15800 && sample.FreeMiB!=sample.TotalMiB-sample.UsedMiB,"free memory uses driver counter, preserving reserved-memory gap");
  Check(sample.Describe().Contains("15.43 GiB") && sample.Power==313.27,"MiB to GiB display and decimal power");
  var old=CultureInfo.CurrentCulture;
  try{CultureInfo.CurrentCulture=new CultureInfo("de-DE");Check(GpuMonitor.Parse(good,time)[0].Power==313.27,"counter parsing independent of Windows decimal locale");}finally{CultureInfo.CurrentCulture=old;}
  sample=GpuMonitor.Parse("0, GPU-one, Test, 616.64, N/A, [Not Supported], N/A, N/A, N/A, N/A, N/A, N/A, N/A",time)[0];
  Check(!sample.UsedMiB.HasValue && !sample.Power.HasValue && sample.Describe().Contains("Unavailable") && !sample.Describe().Contains("0.00 GiB"),"unsupported counters never become zero");
  sample=GpuMonitor.Parse("0, GPU-one, Test, 616.64, 1024, 2048, -1, 101, NaN, Infinity, -50, 0, 0",time)[0];
  Check(!sample.UsedMiB.HasValue && !sample.FreeMiB.HasValue && !sample.Utilization.HasValue && !sample.Temperature.HasValue && !sample.Power.HasValue && !sample.PowerLimit.HasValue,"invalid and impossible counters hidden");
  var multiple=GpuMonitor.Parse(good+"\r\n1, GPU-two, \"GPU, second\", 616.64, 8192, 100, 8000, 0, 35, 20, 200, 210, 405\r\n",time);
  Check(multiple.Count==2 && multiple[1].Uuid=="GPU-two" && multiple[1].Name=="GPU, second","multiple GPUs and quoted CSV names remain distinct");
  Reject(good+"\n"+good,"duplicate GPU identity rejected");Reject(good+", unexpected","driver format drift rejected");Reject("GPU lost","driver error text rejected");Reject("","empty response rejected");Reject("0, GPU-one, \"broken","incomplete CSV rejected");
  if(args.Length>0 && args[0]=="--live")foreach(var gpu in GpuMonitor.Read())Console.WriteLine(gpu.Describe());
  Console.WriteLine(count+" GPU monitor tests passed.");
 }
}
