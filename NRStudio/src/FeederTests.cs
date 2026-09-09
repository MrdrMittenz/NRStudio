using System;
using System.IO;
using NRStudio;
class FeederTests {
 static void Main(string[] args) {
  string folder=Path.Combine(Path.GetTempPath(),"NRStudio-feeder-"+Guid.NewGuid().ToString("N"));Directory.CreateDirectory(folder);
  var g=new Game{Exe=Path.Combine(folder,"Game.exe")};File.Copy(args[1],g.Exe);Core.Payload=args[0];Core.Home=Path.Combine(folder,"user");
  File.WriteAllText(Path.Combine(folder,"dlss5-feed.addon64"),"feeder");File.WriteAllText(Path.Combine(folder,"renodx-dlss5.addon64"),"other NR");
  File.WriteAllText(Path.Combine(folder,"ReShade.ini"),"[ADDON]\nDisabledAddons=Unrelated\n");
  string cfg=Path.Combine(folder,"dlss5-feed.cfg"),original="enabled=1\ncreate_delay=60\nwarmup_rebuild=180\nwork_resolution=75\n";File.WriteAllText(cfg,original);
  Core.Install(g);Core.UpdateRuntime(g);string changed=File.ReadAllText(cfg);
  if(!changed.Contains("create_delay=0") || !changed.Contains("warmup_rebuild=0") || !changed.Contains("work_resolution=75"))throw new Exception("Feeder profile not applied or unrelated value changed");
  Core.Restore(g);if(File.ReadAllText(cfg)!=original)throw new Exception("Feeder configuration not restored exactly");
  File.WriteAllText(cfg,"enabled=1\n");Core.Install(g);changed=File.ReadAllText(cfg);if(!changed.Contains("create_delay=0") || !changed.Contains("warmup_rebuild=0"))throw new Exception("Missing feeder keys not added");Core.Restore(g);
  Console.WriteLine("PASS feeder delay settings, update idempotence, unrelated settings, exact restore and absent keys");
 }
}
