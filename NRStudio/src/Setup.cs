using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Windows.Forms;
using Microsoft.Win32;

class Setup {
 static string Target=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"Programs","NRStudio");
 static string Shortcut=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.Programs),"NR Studio.lnk");
 const string RegPath=@"Software\Microsoft\Windows\CurrentVersion\Uninstall\NRStudio";
 [STAThread] static int Main(string[] args) {
  Application.EnableVisualStyles(); bool uninstall=args.Contains("--uninstall"),silent=args.Contains("--quiet");
  try {
   if(uninstall) {
    if(!silent && MessageBox.Show("Uninstall NR Studio? Your model, library, game installations and backups will be kept.","NR Studio",MessageBoxButtons.OKCancel)!=DialogResult.OK)return 0;
    if(!args.Contains("--worker")) {
     EnsureClosed();string worker=Path.Combine(Path.GetTempPath(),"NRStudio-uninstall-"+Guid.NewGuid().ToString("N")+".exe");File.Copy(Assembly.GetExecutingAssembly().Location,worker);
     Process.Start(new ProcessStartInfo(worker,"--uninstall --worker --quiet --wait "+Process.GetCurrentProcess().Id){WindowStyle=ProcessWindowStyle.Hidden});return 0;
    }
    int wi=Array.IndexOf(args,"--wait");if(wi>=0){try{using(var parent=Process.GetProcessById(int.Parse(args[wi+1])))parent.WaitForExit(30000);}catch(ArgumentException){}}
    EnsureClosed();SafeTarget();
    // Remove only files recorded by this installer. Never recurse over an arbitrary directory.
    string manifest=Path.Combine(Target,"installed-files.txt");
    if(File.Exists(manifest))foreach(string relative in File.ReadAllLines(manifest)) { string p=Contained(relative);if(File.Exists(p) && !string.Equals(p,Assembly.GetExecutingAssembly().Location,StringComparison.OrdinalIgnoreCase))File.Delete(p); }
    if(File.Exists(Shortcut))File.Delete(Shortcut);Registry.CurrentUser.DeleteSubKeyTree(RegPath,false);
    if(!silent)MessageBox.Show("NR Studio removed. Game installations and user data are preserved.","NR Studio");return 0;
   }
   if(!silent && MessageBox.Show("Install NR Studio for your Windows account?\n\n"+Target+"\n\nIncludes the NR model, live Insert overlay, runtime, Visual C++ installer and tested NVIDIA driver installer. No Swapper or separate model download is needed.","NR Studio setup",MessageBoxButtons.OKCancel)!=DialogResult.OK)return 0;
   EnsureClosed();SafeTarget();Directory.CreateDirectory(Target);
   using(var stream=Assembly.GetExecutingAssembly().GetManifestResourceStream("app.zip"))using(var archive=new ZipArchive(stream,ZipArchiveMode.Read)) {
    var names=archive.Entries.Where(e=>!string.IsNullOrEmpty(e.Name)).Select(e=>e.FullName).ToList();
    foreach(var name in names)Contained(name);
    foreach(var entry in archive.Entries) { if(string.IsNullOrEmpty(entry.Name))continue;string dest=Contained(entry.FullName);Directory.CreateDirectory(Path.GetDirectoryName(dest));using(var s=entry.Open())using(var m=new MemoryStream()){s.CopyTo(m);NRStudio.Core.Atomic(dest,m.ToArray());} }
    names.Add("installed-files.txt");File.WriteAllLines(Path.Combine(Target,"installed-files.txt"),names);
   }
   dynamic shell=Activator.CreateInstance(Type.GetTypeFromProgID("WScript.Shell"));dynamic link=shell.CreateShortcut(Shortcut);link.TargetPath=Path.Combine(Target,"NRStudio.exe");link.WorkingDirectory=Target;link.Description="NR Studio — Neural rendering manager";link.Save();
   using(var key=Registry.CurrentUser.CreateSubKey(RegPath)) {key.SetValue("DisplayName","NR Studio");key.SetValue("DisplayVersion","1.1.0");key.SetValue("Publisher","NR Studio (independent project)");key.SetValue("InstallLocation",Target);key.SetValue("DisplayIcon",Path.Combine(Target,"NRStudio.exe"));key.SetValue("UninstallString","\""+Path.Combine(Target,"Uninstall.exe")+"\" --uninstall");key.SetValue("NoModify",1);key.SetValue("NoRepair",1);}
   if(!silent) {
    if(NRStudio.Prerequisites.NeedsVC()) {int code=NRStudio.Prerequisites.InstallVC(Target);if(code!=0 && code!=3010 && code!=1638)throw new IOException("Visual C++ setup returned "+code+". Run prerequisites\\vc_redist.x64.exe before using NR.");}
    string gpu=NRStudio.Prerequisites.GpuInfo();
    if(NRStudio.Prerequisites.NeedsDriver(gpu) && MessageBox.Show(gpu+"\n\nNR was tested with driver 616.56. Install that bundled driver now? Close games first. NVIDIA setup may require a restart.","NR Studio — graphics driver",MessageBoxButtons.YesNo,MessageBoxIcon.Information)==DialogResult.Yes)NRStudio.Prerequisites.InstallDriver(Target);
    Process.Start(Path.Combine(Target,"NRStudio.exe"));
   }return 0;
  }catch(Exception e){if(silent)File.WriteAllText(Path.Combine(Path.GetTempPath(),"NRStudio-setup-error.txt"),e.ToString());else MessageBox.Show(e.Message,"NR Studio setup",MessageBoxButtons.OK,MessageBoxIcon.Error);return 1;}
 }
 static void SafeTarget(){NRStudio.Core.SafePath(Target);}
 static string Contained(string relative) {string p=Path.GetFullPath(Path.Combine(Target,relative));if(!p.StartsWith(Target+Path.DirectorySeparatorChar,StringComparison.OrdinalIgnoreCase))throw new IOException("Invalid installer path.");NRStudio.Core.SafePath(Path.GetDirectoryName(p));if(File.Exists(p)&&(File.GetAttributes(p)&FileAttributes.ReparsePoint)!=0)throw new IOException("Linked installation files are not supported.");return p;}
 static void EnsureClosed(){foreach(var p in Process.GetProcessesByName("NRStudio"))using(p)throw new IOException("Close NR Studio before installing or uninstalling.");}
}
