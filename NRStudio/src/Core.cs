using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Web.Script.Serialization;

namespace NRStudio {
 public class Game { public string Name; public string Exe; public string LaunchArguments; public string Compatibility; public override string ToString() { return Name; } }
 public class Entry { public string Name; public string Original; public string Installed; }
 public class Journal { public string State; public List<Entry> Files = new List<Entry>(); }
 public static class Core {
  public static string Home = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), "NRStudio");
  public static string Payload = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "runtime");
  public const string ModelHash = "8270b350cd82de5ce89806872cdd6b6a9249b80836b91bbeb3573470744cc206";
  public const string ProxyHash = "8f085b570745b2a1577d39b8b9338bea76172fe3d4709899b1a3fb2b00063466";
  public const string ForwardHash = "575cd0b5087bff6a9b97d32656c33f9a9d24533cf8f2b5b1810b3baf67b2074a";
  public static string Model { get { string bundled=Path.Combine(Payload,"nvngx_dlssnr.dll");return File.Exists(bundled)?bundled:Path.Combine(Home,"model", "nvngx_dlssnr.dll"); } }
  public static string Hash(string p) { using(var s=File.OpenRead(p)) using(var h=SHA256.Create()) return BitConverter.ToString(h.ComputeHash(s)).Replace("-", "").ToLowerInvariant(); }
  public static T Read<T>(string p) { return new JavaScriptSerializer().Deserialize<T>(File.ReadAllText(p)); }
  public static void Write(string p, object o) { Atomic(p, Encoding.UTF8.GetBytes(new JavaScriptSerializer().Serialize(o))); }
  public static void Atomic(string p, byte[] bytes) {
   Directory.CreateDirectory(Path.GetDirectoryName(p)); string t=p+"."+Guid.NewGuid().ToString("N")+".tmp";
   try { using(var f=new FileStream(t,FileMode.CreateNew,FileAccess.Write,FileShare.None)) { f.Write(bytes,0,bytes.Length); f.Flush(true); }
    if(File.Exists(p)) File.Replace(t,p,null); else File.Move(t,p);
   } finally { if(File.Exists(t)) File.Delete(t); }
  }
  public static void SafePath(string p) { for(var d=new DirectoryInfo(Path.GetFullPath(p));d!=null;d=d.Parent) if(d.Exists && (d.Attributes&FileAttributes.ReparsePoint)!=0) throw new IOException("Linked folders are not supported: "+d.FullName); }
  public static string Folder(Game g) { return Path.GetDirectoryName(Path.GetFullPath(g.Exe)); }
  public static string Backup(Game g) { return Path.Combine(Folder(g),".nr-studio"); }
  public static string Record(Game g) { return Path.Combine(Backup(g),"installation.json"); }
  public static void CheckGame(Game g) {
   SafePath(Folder(g)); SafePath(Backup(g));
   if(!File.Exists(g.Exe)) throw new IOException("Game executable was not found.");
   using(var r=new BinaryReader(File.OpenRead(g.Exe))) { if(r.ReadUInt16()!=0x5a4d) throw new IOException("Choose a Windows game executable."); r.BaseStream.Position=0x3c; int offset=r.ReadInt32(); if(offset<64 || offset>r.BaseStream.Length-6) throw new IOException("Invalid executable."); r.BaseStream.Position=offset; if(r.ReadUInt32()!=0x4550 || r.ReadUInt16()!=0x8664) throw new IOException("This runtime requires a 64-bit Windows game."); }
   foreach(var p in Process.GetProcesses()) using(p) { try { string path=p.MainModule.FileName; if(path.StartsWith(Folder(g)+Path.DirectorySeparatorChar,StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("Close the game before changing its installation or settings."); } catch(System.ComponentModel.Win32Exception) {} catch(NotSupportedException) {} }
  }
  public static void ImportModel(string p) {
   if(Hash(p)!=ModelHash) throw new IOException("This model does not match the validated runtime. Expected SHA-256 "+ModelHash+".");
   SafePath(Home); string imported=Path.Combine(Home,"model","nvngx_dlssnr.dll");Directory.CreateDirectory(Path.GetDirectoryName(imported)); Atomic(imported,File.ReadAllBytes(p));
  }
  public static List<Game> Games() { var p=Path.Combine(Home,"games.json"); return File.Exists(p)?Read<List<Game>>(p):new List<Game>(); }
  public static void SaveGames(List<Game> games) { Write(Path.Combine(Home,"games.json"),games); }
  public static void ApplyKnownProfile(Game g) {
   if(string.Equals(Path.GetFileName(g.Exe),"7DaysToDie_EAC.exe",StringComparison.OrdinalIgnoreCase))throw new IOException("For the experimental NR profile, select 7DaysToDie.exe instead of the EAC launcher.");
   if(string.Equals(Path.GetFileName(g.Exe),"7DaysToDie.exe",StringComparison.OrdinalIgnoreCase)) {g.Name="7 Days to Die";g.LaunchArguments="-force-d3d12";g.Compatibility="NR untested / DirectX 12 / EAC off";}
  }
  public static string GetIni(string text,string section,string key,string fallback) {
   bool active=false; foreach(string line in text.Replace("\r","").Split('\n')) { var s=line.Trim(); if(s.StartsWith("[")) active=s.Equals("["+section+"]",StringComparison.OrdinalIgnoreCase); else if(active) { int i=s.IndexOf('='); if(i>0 && s.Substring(0,i).Trim().Equals(key,StringComparison.OrdinalIgnoreCase)) return s.Substring(i+1).Trim(); } } return fallback;
  }
  public static string SetIni(string text,string section,string key,string value) {
   var lines=text.Replace("\r","").Split('\n').ToList(); int start=lines.FindIndex(x=>x.Trim().Equals("["+section+"]",StringComparison.OrdinalIgnoreCase));
   if(start<0) { lines.Add("["+section+"]"); lines.Add(key+"="+value); }
   else { int end=start+1; while(end<lines.Count && !lines[end].TrimStart().StartsWith("[")) end++; bool found=false; for(int i=end-1;i>start;i--) { int eq=lines[i].IndexOf('='); if(eq>0 && lines[i].Substring(0,eq).Trim().Equals(key,StringComparison.OrdinalIgnoreCase)) { if(found) lines.RemoveAt(i); else { lines[i]=key+"="+value; found=true; } } } if(!found) lines.Insert(start+1,key+"="+value); }
   return string.Join("\r\n",lines);
  }
  public static string Defaults(string ini) {
   foreach(var kv in new Dictionary<string,string>{{"Enabled","true"},{"TransferStrength","0.75"},{"ColourStrength","0.50"},{"WorkingScale","1.0"},{"AutoCapture","false"}}) ini=SetIni(ini,"DlssNr",kv.Key,kv.Value);
   ini=SetIni(ini,"Menu","OverlayMenu","true"); return SetIni(ini,"Menu","ShortcutKey","0x2D");
  }
  static void ValidateJournal(Journal j) { string[] allowed={"dxgi.dll","nvngx.dll_dlssnr.dll","nvngx_dlssnr.dll","OptiScaler.ini"}; if(j==null || j.Files==null || j.Files.Count!=4 || j.Files.Select(e=>e.Name).Distinct().Count()!=4 || j.Files.Any(e=>!allowed.Contains(e.Name))) throw new IOException("Invalid installation record. Backups have been preserved."); }
  public static string Status(Game g) { if(!File.Exists(Record(g))) return "Not managed"; return Read<Journal>(Record(g)).State; }
  public static void Install(Game g) {
   CheckGame(g);
   if(File.Exists(Record(g))) throw new IOException("This game already has a managed installation. Restore it before reinstalling.");
   if(Directory.Exists(Backup(g))) throw new IOException("An existing backup folder needs recovery. It has been preserved: "+Backup(g));
   if(!File.Exists(Model) || Hash(Model)!=ModelHash) throw new IOException("Import the validated NR model first.");
   string proxy=Path.Combine(Payload,"dxgi.dll"), forward=Path.Combine(Payload,"nvngx.dll_dlssnr.dll");
   if(Hash(proxy)!=ProxyHash || Hash(forward)!=ForwardHash) throw new IOException("Runtime integrity check failed. Reinstall NR Studio.");
   var sources=new Dictionary<string,string>{{"dxgi.dll",proxy},{"nvngx.dll_dlssnr.dll",forward},{"nvngx_dlssnr.dll",Model},{"OptiScaler.ini",Path.Combine(Payload,"OptiScaler.ini")}};
   foreach(var name in sources.Keys) { string p=Path.Combine(Folder(g),name); if(File.Exists(p) && (File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0) throw new IOException("Linked runtime files are not supported."); }
   string existing=Path.Combine(Folder(g),"dxgi.dll");
   if(File.Exists(existing) && Hash(existing)!=ProxyHash && FileVersionInfo.GetVersionInfo(existing).OriginalFilename!="OptiScaler.dll") throw new IOException("An existing dxgi.dll belongs to another or unidentified mod. Resolve that proxy conflict before installing NR.");
   Directory.CreateDirectory(Backup(g)); var j=new Journal{State="Installing"};
   // Save every original and every desired file before changing anything in the game folder.
   foreach(var kv in sources) {
    string dest=Path.Combine(Folder(g),kv.Key); var e=new Entry{Name=kv.Key,Original=File.Exists(dest)?Hash(dest):null};
    if(e.Original!=null) File.Copy(dest,Path.Combine(Backup(g),kv.Key+".original"));
    byte[] data=kv.Key=="OptiScaler.ini"?Encoding.UTF8.GetBytes(File.Exists(dest)?File.ReadAllText(dest):Defaults(File.ReadAllText(kv.Value))):File.ReadAllBytes(kv.Value);
    string stage=Path.Combine(Backup(g),kv.Key+".new"); Atomic(stage,data); e.Installed=Hash(stage); j.Files.Add(e);
   }
   Write(Record(g),j);
   foreach(var e in j.Files) Atomic(Path.Combine(Folder(g),e.Name),File.ReadAllBytes(Path.Combine(Backup(g),e.Name+".new")));
   j.State="Installed"; Write(Record(g),j);
  }
  public static void Restore(Game g) {
   CheckGame(g); var j=Read<Journal>(Record(g)); ValidateJournal(j);
   foreach(var e in j.Files) {
    string p=Path.Combine(Folder(g),e.Name); if(File.Exists(p) && (File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0) throw new IOException("Linked runtime files are not supported.");
    if(e.Original!=null && Hash(Path.Combine(Backup(g),e.Name+".original"))!=e.Original) throw new IOException("Backup integrity failed: "+e.Name);
    if(File.Exists(p) && e.Name!="OptiScaler.ini") { string h=Hash(p); if(h!=e.Installed && h!=e.Original) throw new IOException(e.Name+" changed outside NR Studio. Restore was stopped to preserve that change."); }
   }
   j.State="Restoring"; Write(Record(g),j);
   foreach(var e in j.Files) {
    string p=Path.Combine(Folder(g),e.Name);
    if(e.Name=="OptiScaler.ini" && File.Exists(p)) File.Copy(p,Path.Combine(Backup(g),"settings-at-restore-"+Guid.NewGuid().ToString("N")+".ini"));
    if(e.Original!=null) Atomic(p,File.ReadAllBytes(Path.Combine(Backup(g),e.Name+".original"))); else if(File.Exists(p)) File.Delete(p);
   }
   j.State="Restored"; Write(Record(g),j);
   Directory.Move(Backup(g),Backup(g)+"-restored-"+DateTime.UtcNow.ToString("yyyyMMddHHmmssfff"));
  }
  public static void SaveSettings(Game g,Dictionary<string,string> values) {
   CheckGame(g); string p=Path.Combine(Folder(g),"OptiScaler.ini"); if(!File.Exists(p)) throw new IOException("Install NR first.");
   if(File.Exists(Record(g)) && Status(g)!="Installed") throw new IOException("Restore the incomplete installation first.");
   string text=File.ReadAllText(p); foreach(var kv in values) text=SetIni(text,"DlssNr",kv.Key,kv.Value);
   text=SetIni(text,"Menu","OverlayMenu","true"); text=SetIni(text,"Menu","ShortcutKey","0x2D");
   string backups=Path.Combine(Home,"settings-backups"); Directory.CreateDirectory(backups);
   File.Copy(p,Path.Combine(backups,Guid.NewGuid().ToString("N")+".ini")); Atomic(p,Encoding.UTF8.GetBytes(text));
  }
 }
}
