using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using NRStudio;
class AddSevenDays {
 static int Main(string[] args) {
  try {
   Core.Payload=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),@"Programs\NRStudio\runtime");
   string exe=@"C:\Program Files (x86)\Steam\steamapps\common\7 Days To Die\7DaysToDie.exe";
   var games=Core.Games();var g=games.FirstOrDefault(x=>string.Equals(x.Exe,exe,StringComparison.OrdinalIgnoreCase));
   if(g==null){g=new Game{Name="7 Days to Die",Exe=exe};games.Add(g);}
   Core.CheckGame(g);g.LaunchArguments="-force-d3d12";g.Compatibility="NR untested / DirectX 12 / EAC off";
   File.Copy(Path.Combine(Core.Home,"games.json"),Path.Combine(Core.Home,"games-before-7days-"+DateTime.UtcNow.ToString("yyyyMMddHHmmss")+".json"));
   Core.SaveGames(games);
   if(!File.Exists(Core.Record(g)))Core.Install(g);
   Core.SaveSettings(g,new Dictionary<string,string>{{"Enabled","true"},{"AutoCapture","false"}});
   Console.WriteLine("Added 7 Days to Die; runtime installed with originals backed up. DirectX 12 direct executable launch, EAC launcher not used. NR compatibility remains untested.");return 0;
  }catch(Exception e){Console.Error.WriteLine(e);return 1;}
 }
}
