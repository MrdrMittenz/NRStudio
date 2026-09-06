using System;
using NRStudio;
class RuntimeTool {
 static int Main(string[] args) {
  try {Core.Payload=args[0];var g=new Game{Name="Validation target",Exe=args[1]};if(args[2]=="install")Core.Install(g);else if(args[2]=="restore")Core.Restore(g);else throw new ArgumentException("Expected install or restore");Console.WriteLine(args[2]+" complete: "+Core.Status(g));return 0;}catch(Exception e){Console.Error.WriteLine(e);return 1;}
 }
}
