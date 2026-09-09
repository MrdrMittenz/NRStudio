using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NRStudio;
class Tests {
 static int count;
 static void Assert(bool b,string name){if(!b)throw new Exception(name);Console.WriteLine("PASS "+name);count++;}
 static void Refuses(Action a,string name){bool failed=false;try{a();}catch(IOException){failed=true;}Assert(failed,name);}
  static void Main(string[] args) {
  if(args.Length==1 && args[0]=="--wait-fixture") { System.Threading.Thread.Sleep(30000); return; }
  string temp=Path.Combine(Path.GetTempPath(),"NRStudio-tests-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(temp);
  Core.Home=Path.Combine(temp,"user");Core.Payload=args[0];
  Assert(File.Exists(Core.Model) && Core.Hash(Core.Model)==Core.ModelHash && !Directory.Exists(Core.Home),"complete package works without an imported model or existing user state");
  string folder=Path.Combine(temp,"game");Directory.CreateDirectory(folder);var g=new Game{Name="Fixture",Exe=Path.Combine(folder,"Game.exe")};File.Copy(args[2],g.Exe);
  string config="[Other]\r\nKeep=42\r\n[DlssNr]\r\nEnabled=true\r\nTransferStrength=1.1\r\n";File.WriteAllText(Path.Combine(folder,"OptiScaler.ini"),config);
  string liveExe=Path.Combine(folder,"RunningFixture.exe");File.Copy(System.Reflection.Assembly.GetExecutingAssembly().Location,liveExe);
  using(var live=System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(liveExe,"--wait-fixture"){UseShellExecute=false,CreateNoWindow=true})) {
   try { bool refused=false;try {Core.CheckGame(g);}catch(InvalidOperationException) {refused=true;}Assert(refused,"live game protection remains effective with process-exit race handling"); }
   finally { if(!live.HasExited)live.Kill();live.WaitForExit(3000); }
  } File.Delete(liveExe);
  Core.Install(g);Assert(Core.Status(g)=="Installed","install commits");Assert(File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==config,"existing settings preserved");
  Assert(!Core.BeforeUpscaling(g) && Core.SupportsPlacement(g),"fresh install defaults to after upscaling with both orders supported");
  Assert(File.Exists(Path.Combine(folder,Core.OptimizedMarker)) && File.Exists(Path.Combine(folder,Core.PreparedMarker)),"install enables the optimized 3090 runtime");
  Assert(Core.Hash(Path.Combine(folder,"nvngx_dlssnr.dll"))==Core.ModelHash,"native model distinct and verified");
  Refuses(()=>Core.Install(g),"duplicate installation refused");
  Core.SaveSettings(g,new Dictionary<string,string>{{"TransferStrength","0.75"}});string changed=File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"));Assert(Core.GetIni(changed,"Other","Keep","")=="42","unrelated settings preserved");Assert(Core.GetIni(changed,"DlssNr","TransferStrength","")=="0.75","settings written");
  Core.SaveSettings(g,new Dictionary<string,string>(),true);Assert(Core.BeforeUpscaling(g),"before upscaling persists in the runtime marker");
  Core.SaveSettings(g,new Dictionary<string,string>{{"Intensity","1.25"}});Assert(Core.BeforeUpscaling(g),"saving unrelated settings preserves placement");
  Core.SaveSettings(g,new Dictionary<string,string>(),false);Assert(!Core.BeforeUpscaling(g),"after upscaling removes the marker");
  Core.SaveSettings(g,new Dictionary<string,string>(),true);
  File.WriteAllText(Path.Combine(folder,"dxgi.dll"),"external modification");Refuses(()=>Core.Restore(g),"external DLL changes protected");Assert(Core.Status(g)=="Installed","conflict makes no changes");
  File.Copy(Path.Combine(Core.Payload,"dxgi.dll"),Path.Combine(folder,"dxgi.dll"),true);Core.Restore(g);Assert(!File.Exists(Path.Combine(folder,"dxgi.dll")),"restore removes newly installed DLL");Assert(File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==config,"restore recovers original configuration");Assert(Directory.GetFiles(folder,"settings-at-restore-*.ini",SearchOption.AllDirectories).Length==1,"edited settings archived");
  Assert(!Core.BeforeUpscaling(g) && !File.Exists(Path.Combine(folder,Core.OptimizedMarker)) && !File.Exists(Path.Combine(folder,Core.PreparedMarker)),"restore removes newly introduced option markers");
  File.WriteAllText(Path.Combine(folder,Core.BeforeMarker),"original preference");Core.Install(g);
  Core.SaveSettings(g,new Dictionary<string,string>(),false);Core.Restore(g);
  Assert(File.ReadAllText(Path.Combine(folder,Core.BeforeMarker))=="original preference","restore recovers a pre-existing placement marker byte-for-byte");File.Delete(Path.Combine(folder,Core.BeforeMarker));
  File.Copy(Path.Combine(Core.Payload,"dxgi.dll"),Path.Combine(folder,"dxgi.dll"));Core.Install(g);Core.Restore(g);Assert(Core.Hash(Path.Combine(folder,"dxgi.dll"))==Core.ProxyHash,"existing proxy restored byte-for-byte");
  File.Delete(Path.Combine(folder,"dxgi.dll"));Core.Install(g);var j=Core.Read<Journal>(Core.Record(g));j.State="Installing";Core.Write(Core.Record(g),j);File.Delete(Path.Combine(folder,"nvngx_dlssnr.dll"));Core.Restore(g);Assert(!File.Exists(Core.Record(g)),"interrupted deployment recoverable");
  Core.Install(g);j=Core.Read<Journal>(Core.Record(g));j.Options=null;Core.Write(Core.Record(g),j);
  Core.SaveSettings(g,new Dictionary<string,string>(),true);Assert(Core.BeforeUpscaling(g),"legacy journal gains a placement backup when first changed");
  string originalIni=File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"));
  File.WriteAllText(Path.Combine(folder,"dxgi.dll"),"managed previous proxy");File.WriteAllText(Path.Combine(folder,"nvngx.dll_dlssnr.dll"),"managed previous forwarder");
  j=Core.Read<Journal>(Core.Record(g));foreach(var e in j.Files.Where(e=>e.Name!="OptiScaler.ini"))e.Installed=Core.Hash(Path.Combine(folder,e.Name));Core.Write(Core.Record(g),j);
  Refuses(()=>Core.SaveSettings(g,new Dictionary<string,string>{{"Intensity","0.5"}},true),"unsupported runtime cannot silently accept before upscaling");
  Assert(File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==originalIni,"unsupported selection leaves settings unchanged");
  Core.UpdateRuntime(g);Assert(Core.SupportsPlacement(g) && Core.BeforeUpscaling(g),"runtime upgrade preserves selected placement");
  Assert(File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==originalIni,"runtime upgrade preserves all existing sliders");
  File.WriteAllText(Path.Combine(folder,"dxgi.dll"),"external new mod");Refuses(()=>Core.UpdateRuntime(g),"update protects unexpected DLL changes");
  File.Copy(Path.Combine(Core.Payload,"dxgi.dll"),Path.Combine(folder,"dxgi.dll"),true);Core.Restore(g);
  Assert(!File.Exists(Path.Combine(folder,"dxgi.dll")) && File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==config,"restore after upgrade retains the original pre-install backups");
  // A process interruption may leave either previous or replacement DLL bytes.
  Core.Install(g);j=Core.Read<Journal>(Core.Record(g));var previous=j.Files.Single(e=>e.Name=="dxgi.dll");previous.PreviousInstalled=previous.Installed;previous.Installed=new string('a',64);j.State="Updating";Core.Write(Core.Record(g),j);Core.Restore(g);
  Assert(!File.Exists(Core.Record(g)),"interrupted runtime update can restore originals");
  Core.Install(g);j=Core.Read<Journal>(Core.Record(g));j.Files[0].Name="../outside.dll";Core.Write(Core.Record(g),j);Refuses(()=>Core.Restore(g),"tampered journal path rejected");j.Files[0].Name="dxgi.dll";Core.Write(Core.Record(g),j);Core.Restore(g);
  File.WriteAllText(Path.Combine(folder,"dxgi.dll"),"another mod");Refuses(()=>Core.Install(g),"unknown proxy protected");
  string ini=Core.SetIni("[DlssNr]\nEnabled=false\nEnabled=auto\n[X]\nEnabled=false","DlssNr","Enabled","true");Assert(ini.Split('\n').Count(x=>x.Trim()=="Enabled=true")==1 && Core.GetIni(ini,"X","Enabled","")=="false","duplicate keys collapsed without touching other sections");
  Assert(Core.GetIni(Core.Defaults(""),"Menu","ShortcutKey","")=="0x2D","Insert menu default");
  string freshFolder=Path.Combine(temp,"fresh-game");Directory.CreateDirectory(freshFolder);
  var fresh=new Game{Name="Fresh quality defaults",Exe=Path.Combine(freshFolder,"Game.exe")};File.Copy(args[2],fresh.Exe);
  Core.Install(fresh);string freshIni=File.ReadAllText(Path.Combine(freshFolder,"OptiScaler.ini"));
  Assert(Core.GetIni(freshIni,"DlssNr","ComposeMode","")=="0" && Core.GetIni(freshIni,"DlssNr","WorkingScale","")=="1.0" && !Core.BeforeUpscaling(fresh),"fresh game keeps Current full-resolution quality default");
  Core.Restore(fresh);
  if(args.Length>3) {
   string rs=Path.Combine(freshFolder,"ReShade64.dll"),dx=Path.Combine(freshFolder,"dxgi.dll"),dlss=Path.Combine(freshFolder,"nvngx_dlss.dll");
   File.Copy(args[3],dx);string rh=Core.Hash(dx);File.WriteAllText(dlss,"swapped DLSS remains untouched");
   File.WriteAllText(rs,"different existing mod");Refuses(()=>Core.Install(fresh),"conflicting ReShade destination preserved");
   Assert(!Directory.Exists(Core.Backup(fresh)) && Core.Hash(dx)==rh,"ReShade conflict leaves game and backups unchanged");File.Delete(rs);
   Core.Install(fresh);Assert(Core.Hash(rs)==rh && Core.Hash(dx)==Core.ProxyHash,"ReShade is chained byte-for-byte alongside NR");
   Assert(Core.GetIni(File.ReadAllText(Path.Combine(freshFolder,"OptiScaler.ini")),"Plugins","LoadReshade","")=="true","ReShade chain enabled");
   Core.UpdateRuntime(fresh);Assert(Core.Hash(rs)==rh,"runtime update preserves chained ReShade");
   File.WriteAllText(rs,"external edit");Refuses(()=>Core.Restore(fresh),"restore protects externally changed chained ReShade");File.Copy(args[3],rs,true);
   Core.Restore(fresh);Assert(Core.Hash(dx)==rh && !File.Exists(rs) && File.ReadAllText(dlss)=="swapped DLSS remains untouched","restore returns ReShade layout and preserves swapped DLSS");
   File.Copy(args[3],rs);Core.Install(fresh);Core.Restore(fresh);Assert(Core.Hash(rs)==rh && Core.Hash(dx)==rh,"pre-existing matching ReShade chain restored");
   string reshadeIni=Path.Combine(freshFolder,"ReShade.ini"),feeder=Path.Combine(freshFolder,"dlss5-feed.addon64"),otherNr=Path.Combine(freshFolder,"renodx-dlss5.addon64");
   string originalReshade="[ADDON]\r\nDisabledAddons=Unrelated\r\n[GENERAL]\r\nKeep=yes\r\n";
   File.WriteAllText(reshadeIni,originalReshade);File.WriteAllText(feeder,"feeder");File.WriteAllText(otherNr,"other NR");
   Core.Install(fresh);Core.UpdateRuntime(fresh);
   string disabled=Core.GetIni(File.ReadAllText(reshadeIni),"ADDON","DisabledAddons","");
   Assert(disabled=="Unrelated,@renodx-dlss5.addon64" && File.ReadAllText(feeder)=="feeder" && File.ReadAllText(otherNr)=="other NR","feeder uses one NR hook without removing add-ons; update is idempotent");
   Core.Restore(fresh);Assert(File.ReadAllText(reshadeIni)==originalReshade,"restore recovers original ReShade add-on selection");
  }
  Console.WriteLine(count+" tests passed. Fixture retained at "+temp);
 }
}
