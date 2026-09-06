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
  string temp=Path.Combine(Path.GetTempPath(),"NRStudio-tests-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(temp);
  Core.Home=Path.Combine(temp,"user");Core.Payload=args[0];
  Assert(File.Exists(Core.Model) && Core.Hash(Core.Model)==Core.ModelHash && !Directory.Exists(Core.Home),"complete package works without an imported model or existing user state");
  string folder=Path.Combine(temp,"game");Directory.CreateDirectory(folder);var g=new Game{Name="Fixture",Exe=Path.Combine(folder,"Game.exe")};File.Copy(args[2],g.Exe);
  string config="[Other]\r\nKeep=42\r\n[DlssNr]\r\nEnabled=true\r\nTransferStrength=1.1\r\n";File.WriteAllText(Path.Combine(folder,"OptiScaler.ini"),config);
  Core.Install(g);Assert(Core.Status(g)=="Installed","install commits");Assert(File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==config,"existing settings preserved");
  Assert(Core.Hash(Path.Combine(folder,"nvngx_dlssnr.dll"))==Core.ModelHash,"native model distinct and verified");
  Refuses(()=>Core.Install(g),"duplicate installation refused");
  Core.SaveSettings(g,new Dictionary<string,string>{{"TransferStrength","0.75"}});string changed=File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"));Assert(Core.GetIni(changed,"Other","Keep","")=="42","unrelated settings preserved");Assert(Core.GetIni(changed,"DlssNr","TransferStrength","")=="0.75","settings written");
  File.WriteAllText(Path.Combine(folder,"dxgi.dll"),"external modification");Refuses(()=>Core.Restore(g),"external DLL changes protected");Assert(Core.Status(g)=="Installed","conflict makes no changes");
  File.Copy(Path.Combine(Core.Payload,"dxgi.dll"),Path.Combine(folder,"dxgi.dll"),true);Core.Restore(g);Assert(!File.Exists(Path.Combine(folder,"dxgi.dll")),"restore removes newly installed DLL");Assert(File.ReadAllText(Path.Combine(folder,"OptiScaler.ini"))==config,"restore recovers original configuration");Assert(Directory.GetFiles(folder,"settings-at-restore-*.ini",SearchOption.AllDirectories).Length==1,"edited settings archived");
  File.Copy(Path.Combine(Core.Payload,"dxgi.dll"),Path.Combine(folder,"dxgi.dll"));Core.Install(g);Core.Restore(g);Assert(Core.Hash(Path.Combine(folder,"dxgi.dll"))==Core.ProxyHash,"existing proxy restored byte-for-byte");
  File.Delete(Path.Combine(folder,"dxgi.dll"));Core.Install(g);var j=Core.Read<Journal>(Core.Record(g));j.State="Installing";Core.Write(Core.Record(g),j);File.Delete(Path.Combine(folder,"nvngx_dlssnr.dll"));Core.Restore(g);Assert(!File.Exists(Core.Record(g)),"interrupted deployment recoverable");
  Core.Install(g);j=Core.Read<Journal>(Core.Record(g));j.Files[0].Name="../outside.dll";Core.Write(Core.Record(g),j);Refuses(()=>Core.Restore(g),"tampered journal path rejected");j.Files[0].Name="dxgi.dll";Core.Write(Core.Record(g),j);Core.Restore(g);
  File.WriteAllText(Path.Combine(folder,"dxgi.dll"),"another mod");Refuses(()=>Core.Install(g),"unknown proxy protected");
  string ini=Core.SetIni("[DlssNr]\nEnabled=false\nEnabled=auto\n[X]\nEnabled=false","DlssNr","Enabled","true");Assert(ini.Split('\n').Count(x=>x.Trim()=="Enabled=true")==1 && Core.GetIni(ini,"X","Enabled","")=="false","duplicate keys collapsed without touching other sections");
  Assert(Core.GetIni(Core.Defaults(""),"Menu","ShortcutKey","")=="0x2D","Insert menu default");
  Console.WriteLine(count+" tests passed. Fixture retained at "+temp);
 }
}
