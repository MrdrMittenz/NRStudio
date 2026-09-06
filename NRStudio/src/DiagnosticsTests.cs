using System;
using NRStudio;
class DiagnosticsTests {
 static int count;
 static void Check(bool value,string name){if(!value)throw new Exception(name);Console.WriteLine("PASS "+name);count++;}
 static void Main(string[] args){
  string good="pid=123 tick=20000 evaluate frame=120 result=00000001 size=2560x1440 guides=1972x1108 reset=0\n";
  Check(RuntimeDiagnostics.DescribeEvaluation(good,123,21000).StartsWith("Recent native evaluation: success"),"matching recent process evidence");
  Check(RuntimeDiagnostics.DescribeEvaluation(good,123,40000).StartsWith("Historical"),"stale evidence is historical");
  Check(RuntimeDiagnostics.DescribeEvaluation(good,456,21000).StartsWith("Historical"),"other process is historical");
  Check(RuntimeDiagnostics.DescribeEvaluation(good,0,21000).StartsWith("Historical"),"closed game is historical");
  Check(RuntimeDiagnostics.DescribeEvaluation(good,123,1000).StartsWith("Historical"),"future tick after reboot is historical");
  string bad="pid=123 tick=20500 evaluate frame=121 result=bad00002 size=2560x1440 guides=1972x1108 reset=0\n";
  Check(RuntimeDiagnostics.DescribeEvaluation(good+bad,123,21000).StartsWith("Recent native evaluation: FAILED"),"latest failure supersedes earlier success");
  Check(RuntimeDiagnostics.DescribeEvaluation("pid=123 create result=00000001",123,21000).StartsWith("No native evaluation"),"creation alone is not evaluation");
  if(args.Length>0){Console.WriteLine(RuntimeDiagnostics.Signature(args[0]));if(args.Length>1)Console.WriteLine(RuntimeDiagnostics.Evaluation(args[1]));}
  Console.WriteLine(count+" diagnostic tests passed.");
 }
}
