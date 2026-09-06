using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NRStudio {
 static class Program {
  [STAThread] static void Main(string[] args) {
   Application.EnableVisualStyles(); Application.SetCompatibleTextRenderingDefault(false);
   bool created; using(var mutex=new System.Threading.Mutex(true,"Local\\NRStudio.Manager",out created)) {
    if(!created) { MessageBox.Show("NR Studio is already open."); return; }
    Application.Run(new Studio());
   }
  }
 }
 public class Studio:Form {
  List<Game> games; ListBox library=new ListBox(); Label headline=new Label(), location=new Label(), state=new Label(), model=new Label();
  TextBox output=new TextBox(); FlowLayoutPanel actions=new FlowLayoutPanel(); TableLayoutPanel fields=new TableLayoutPanel();
  Dictionary<string,ComboBox> editors=new Dictionary<string,ComboBox>(); CheckBox enabled=new CheckBox(); bool busy;
  Color ink=Color.FromArgb(225,233,240), panel=Color.FromArgb(26,33,44), accent=Color.FromArgb(109,221,190);
  public Studio() {
   SuspendLayout(); AutoScaleMode=AutoScaleMode.None;
   Text="NR Studio"; Size=new Size(1100,850); MinimumSize=new Size(950,720); StartPosition=FormStartPosition.CenterScreen;
   Font=new Font("Segoe UI",10); BackColor=Color.FromArgb(15,21,30); ForeColor=ink;
   var root=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=2,RowCount=2,Padding=new Padding(22)};
   root.ColumnStyles.Add(new ColumnStyle(SizeType.Absolute,245)); root.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));
   root.RowStyles.Add(new RowStyle(SizeType.Absolute,74)); root.RowStyles.Add(new RowStyle(SizeType.Percent,100)); Controls.Add(root);
   var brand=new Label{Text="NR STUDIO",Font=new Font("Segoe UI",24,FontStyle.Bold),ForeColor=accent,Dock=DockStyle.Fill}; root.Controls.Add(brand,0,0);
   root.Controls.Add(new Label{Text="Experimental native neural rendering\nLocal profiles  /  Insert for the in-game menu",Dock=DockStyle.Fill,Padding=new Padding(18,5,0,0)},1,0);
   var left=new TableLayoutPanel{Dock=DockStyle.Fill,RowCount=5,ColumnCount=1,Padding=new Padding(0,0,16,0)};
   left.RowStyles.Add(new RowStyle(SizeType.Absolute,32)); left.RowStyles.Add(new RowStyle(SizeType.Percent,100)); left.RowStyles.Add(new RowStyle(SizeType.Absolute,48));left.RowStyles.Add(new RowStyle(SizeType.Absolute,48));left.RowStyles.Add(new RowStyle(SizeType.Absolute,92));root.Controls.Add(left,0,1);
   left.Controls.Add(new Label{Text="YOUR GAMES",ForeColor=accent,Dock=DockStyle.Fill});
   library.Dock=DockStyle.Fill; library.BackColor=panel;library.ForeColor=ink; library.BorderStyle=BorderStyle.None;library.ItemHeight=32; library.IntegralHeight=false;left.Controls.Add(library);
   left.Controls.Add(Button("+  Add game",AddGame)); left.Controls.Add(Button("Remove from library",RemoveGame));
   left.Controls.Add(new Label{Text="Start with a 64-bit DirectX 12 game using native DLSS. New games are untested until validated.",Dock=DockStyle.Fill,ForeColor=Color.Silver,Padding=new Padding(0,10,0,0)});
   var right=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=7,Padding=new Padding(18,0,0,0)};root.Controls.Add(right,1,1);
   foreach(int height in new[]{38,42,42,58}) right.RowStyles.Add(new RowStyle(SizeType.Absolute,height));
   right.RowStyles.Add(new RowStyle(SizeType.Percent,100)); right.RowStyles.Add(new RowStyle(SizeType.Absolute,112));right.RowStyles.Add(new RowStyle(SizeType.Absolute,96));
   headline.Font=new Font("Segoe UI",17,FontStyle.Bold); headline.Dock=DockStyle.Fill;right.Controls.Add(headline);
   location.Dock=DockStyle.Fill;location.AutoEllipsis=true;location.ForeColor=Color.Silver;right.Controls.Add(location);
   state.Dock=DockStyle.Fill;state.ForeColor=accent;right.Controls.Add(state);
   var modelBar=new FlowLayoutPanel{Dock=DockStyle.Fill};model.AutoSize=true;model.Margin=new Padding(0,12,12,0);modelBar.Controls.Add(model);modelBar.Controls.Add(Button("Package contents",()=>Process.Start(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"README.txt"))));right.Controls.Add(modelBar);
   var scroll=new Panel{Dock=DockStyle.Fill,AutoScroll=true,BackColor=panel,Padding=new Padding(12)};right.Controls.Add(scroll);
   fields.AutoSize=true;fields.Dock=DockStyle.Top;fields.ColumnCount=2;fields.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,54));fields.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,46));scroll.Controls.Add(fields);
   enabled.Text="Enable neural rendering";enabled.AutoSize=true;enabled.Margin=new Padding(4,8,4,12);fields.Controls.Add(enabled);fields.SetColumnSpan(enabled,2);
   Field("Detail strength","TransferStrength",new[]{"auto","0","0.5","0.75","1","1.5","2"});
   Field("Colour strength","ColourStrength",new[]{"auto","0","0.25","0.5","0.75","1"});
   Field("Model resolution (1 = 100%)","WorkingScale",new[]{"auto","0.25","0.5","0.75","1"});
   Field("Style (0 standard / 1 natural / 2 cinematic)","Style",new[]{"auto","0","1","2"});
   Field("Model preset (requires restart)","Preset",new[]{"auto","0","1","2","3"});
   Field("Intensity","Intensity",new[]{"auto","0","0.5","1","1.5","2"});
   Field("Local structure","LocalStructure",new[]{"auto","0","0.5","1","1.5","2"});
   Field("Local tone","LocalTone",new[]{"auto","0","0.5","1","1.5","2"});
   Field("Skin structure (-1 follows local structure)","SkinStructure",new[]{"auto","-1","0","0.5","1","2"});
   Field("Automatic skin mask","AutoMask",new[]{"auto","true","false"});
   Field("Paper white","WhitePointScale",new[]{"auto","0.25","0.5","1","2","4"});
   Field("Highlight guard","MaxRatio",new[]{"auto","1","2","4","8"});
   actions.Dock=DockStyle.Fill;actions.AutoScroll=true;right.Controls.Add(actions);
   actions.Controls.Add(Button("Install NR",()=>{var g=Selected();Run("Installing runtime",()=>Core.Install(g));}));
   actions.Controls.Add(Button("Save settings",Save));
   actions.Controls.Add(Button("Restore original files",Restore));
   actions.Controls.Add(Button("Launch game",Launch));
   actions.Controls.Add(Button("Check runtime",Diagnose));
   actions.Controls.Add(Button("Open game folder",()=>{if(Selected()!=null) Process.Start("explorer.exe",Core.Folder(Selected()));}));
   actions.Controls.Add(Button("GPU / driver setup",Hardware));
   actions.Controls.Add(Button("Help",()=>Process.Start(Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"README.txt"))));
   output.Multiline=true;output.ReadOnly=true;output.Dock=DockStyle.Fill;output.ScrollBars=ScrollBars.Vertical;output.BackColor=panel;output.ForeColor=ink;output.BorderStyle=BorderStyle.None;right.Controls.Add(output);
   library.SelectedIndexChanged+=(s,e)=>RefreshGame();
   try { games=Core.Games(); } catch(Exception e) { games=new List<Game>();Log("Could not load library: "+e.Message); }
   if(games.Count==0 && !File.Exists(Path.Combine(Core.Home,"games.json"))) {
    string known=@"C:\Program Files (x86)\Steam\steamapps\common\S.T.A.L.K.E.R. 2 Heart of Chornobyl\Stalker2\Binaries\Win64";
    if(Directory.Exists(known)) { string exe=Directory.GetFiles(known,"*Shipping.exe").FirstOrDefault(); if(exe!=null) { games.Add(new Game{Name="S.T.A.L.K.E.R. 2",Exe=exe});Core.SaveGames(games); } }
   }
   Reload();Log("Experimental NR integration. Check runtime reports model signature and native evaluation evidence. Press Insert in-game for live sliders. Desktop settings apply next launch.");
   Shown+=(s,e)=> { if(!File.Exists(Core.Model)) { string candidate=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),@"Programs\DLSS 5 Swapper\resources\payload\streamline\nvngx_dlssnr.dll");if(File.Exists(candidate)) Run("Importing your installed model",()=>Core.ImportModel(candidate)); } };
   FormClosing+=(s,e)=>{if(busy){e.Cancel=true;Log("Wait for the current operation to finish before closing.");}};
   using(var graphics=Graphics.FromHwnd(IntPtr.Zero)) ScaleLayout(this,graphics.DpiX/96F);
   ResumeLayout(true);
  }
  static void ScaleLayout(Control c,float factor) {
   c.SuspendLayout();
   foreach(Control child in c.Controls) ScaleLayout(child,factor);
   c.Size=new Size((int)(c.Width*factor),(int)(c.Height*factor));
   c.MinimumSize=new Size((int)(c.MinimumSize.Width*factor),(int)(c.MinimumSize.Height*factor));
   c.Padding=Scaled(c.Padding,factor);c.Margin=Scaled(c.Margin,factor);
   var table=c as TableLayoutPanel;
   if(table!=null){foreach(RowStyle r in table.RowStyles)if(r.SizeType==SizeType.Absolute)r.Height*=factor;foreach(ColumnStyle col in table.ColumnStyles)if(col.SizeType==SizeType.Absolute)col.Width*=factor;}
   c.ResumeLayout(true);
  }
  static Padding Scaled(Padding p,float f){return new Padding((int)(p.Left*f),(int)(p.Top*f),(int)(p.Right*f),(int)(p.Bottom*f));}
  Button Button(string text,Action action) { var b=new Button{Text=text,AutoSize=false,Size=new Size(164,36),FlatStyle=FlatStyle.Flat,BackColor=panel,ForeColor=ink,Margin=new Padding(0,4,8,4),Padding=new Padding(6,0,6,0)};b.FlatAppearance.BorderColor=Color.FromArgb(65,85,99);b.Click+=(s,e)=>{if(busy)return;try{action();}catch(Exception ex){Log(ex.Message);MessageBox.Show(this,ex.Message,"NR Studio",MessageBoxButtons.OK,MessageBoxIcon.Information);}};return b; }
  void Field(string label,string key,string[] choices) { int row=(fields.Controls.Count+1)/2;if(fields.RowStyles.Count==0)fields.RowStyles.Add(new RowStyle(SizeType.Absolute,38));fields.RowStyles.Add(new RowStyle(SizeType.Absolute,38));var l=new Label{Text=label,AutoSize=false,Dock=DockStyle.Fill,TextAlign=ContentAlignment.MiddleLeft,Margin=new Padding(4,0,8,0)};var c=new ComboBox{Width=150,Anchor=AnchorStyles.Left|AnchorStyles.Right,BackColor=panel,ForeColor=ink,FlatStyle=FlatStyle.Flat,Margin=new Padding(4,4,4,4)};c.Items.AddRange(choices);c.Text="auto";fields.Controls.Add(l,0,row);fields.Controls.Add(c,1,row);editors[key]=c; }
  Game Selected() { var g=library.SelectedItem as Game;if(g==null)throw new IOException("Add or select a game first.");return g; }
  void Reload() { int index=library.SelectedIndex;library.Items.Clear();foreach(var g in games)library.Items.Add(g);if(games.Count>0)library.SelectedIndex=Math.Max(0,Math.Min(index,games.Count-1));RefreshGame(); }
  void RefreshGame() {
   model.Text=File.Exists(Core.Model)?"NR model ready":"Model missing";
   var g=library.SelectedItem as Game;fields.Enabled=g!=null;headline.Text=g==null?"Add your first game":g.Name;location.Text=g==null?"Choose the game's actual 64-bit rendering executable.":g.Exe;
   if(g==null){state.Text="Ready to add games";return;}
   try { bool known=g.Exe.IndexOf("Stalker2",StringComparison.OrdinalIgnoreCase)>=0;
    state.Text=Core.Status(g)+"  /  "+(!string.IsNullOrWhiteSpace(g.Compatibility)?g.Compatibility:known?"Runtime tested in S.T.A.L.K.E.R. 2":"Compatibility untested");
    string ini=Path.Combine(Core.Folder(g),"OptiScaler.ini"),text=File.Exists(ini)?File.ReadAllText(ini):Core.Defaults("");
    enabled.Checked=Core.GetIni(text,"DlssNr","Enabled","false")=="true";
    foreach(var kv in editors) kv.Value.Text=Core.GetIni(text,"DlssNr",kv.Key,"auto");
   }catch(Exception e){Log(e.Message);}
  }
  void Log(string text) { output.AppendText(DateTime.Now.ToString("HH:mm:ss")+"  "+text+Environment.NewLine); }
  async void Run(string title,Action action) {
   if(busy)return;busy=true;library.Enabled=false;actions.Enabled=false;fields.Enabled=false;Log(title+"...");
   try { await Task.Run(action);Log(title+": complete."); } catch(Exception e){Log("Stopped: "+e.Message);MessageBox.Show(this,e.Message,"NR Studio",MessageBoxButtons.OK,MessageBoxIcon.Information);}finally{busy=false;library.Enabled=true;actions.Enabled=true;RefreshGame();}
  }
  void AddGame() { using(var d=new OpenFileDialog{Title="Select the game's 64-bit executable (Unreal: Binaries\\Win64)",Filter="Game executable|*.exe"}) if(d.ShowDialog(this)==DialogResult.OK) {
   if(games.Any(g=>string.Equals(Core.Folder(g),Path.GetDirectoryName(d.FileName),StringComparison.OrdinalIgnoreCase)))throw new IOException("This game folder is already in your library.");
   var g=new Game{Name=Path.GetFileNameWithoutExtension(d.FileName).Replace("-Win64-Shipping",""),Exe=d.FileName};Core.ApplyKnownProfile(g);Core.CheckGame(g);games.Add(g);Core.SaveGames(games);Reload();library.SelectedItem=g;
  } }
  void RemoveGame() { var g=Selected();if(File.Exists(Core.Record(g)))throw new IOException("Restore the managed installation before removing this game.");games.Remove(g);Core.SaveGames(games);Reload(); }
  void Import() { using(var d=new OpenFileDialog{Filter="NR model|nvngx_dlssnr.dll",Title="Import your nvngx_dlssnr.dll model"})if(d.ShowDialog(this)==DialogResult.OK){string p=d.FileName;Run("Importing model",()=>Core.ImportModel(p));} }
  void Save() {
   var g=Selected();var values=new Dictionary<string,string>{{"Enabled",enabled.Checked?"true":"false"}};
   foreach(var kv in editors) { string v=kv.Value.Text.Trim().ToLowerInvariant();double n;
    if(v!="auto") { if(kv.Key=="AutoMask") {if(v!="true"&&v!="false")throw new IOException("Automatic skin mask must be auto, true or false.");}
     else { double min=0,max=2;if(kv.Key=="WorkingScale"){min=.25;max=1;}if(kv.Key=="ColourStrength")max=1;if(kv.Key=="Preset")max=3;if(kv.Key=="SkinStructure")min=-1;if(kv.Key=="WhitePointScale"){min=.25;max=4;}if(kv.Key=="MaxRatio"){min=1;max=8;}
      if(!double.TryParse(v,NumberStyles.Float,CultureInfo.InvariantCulture,out n)||double.IsNaN(n)||double.IsInfinity(n)||n<min||n>max||((kv.Key=="Style"||kv.Key=="Preset")&&n!=Math.Floor(n)))throw new IOException(kv.Key+" must be auto or between "+min+" and "+max+" (use a decimal point)."); }
    } values[kv.Key]=v;
   } Run("Saving settings for next launch",()=>Core.SaveSettings(g,values));
  }
  void Restore() { var g=Selected();if(!File.Exists(Core.Record(g)))throw new IOException("There is no NR Studio installation to restore. Existing installations are preserved until you install through this app.");Run("Restoring originals and archiving current settings",()=>Core.Restore(g)); }
  void Launch() { var g=Selected();if(g.Exe.IndexOf("Stalker2",StringComparison.OrdinalIgnoreCase)>=0)Process.Start("steam://rungameid/1643320");else Process.Start(new ProcessStartInfo(g.Exe,g.LaunchArguments??""){WorkingDirectory=Core.Folder(g),UseShellExecute=true}); }
  void Diagnose() {
   var g=Selected();string dir=Core.Folder(g);Run("Checking runtime files",()=>{
    var report=new List<string>();foreach(var kv in new Dictionary<string,string>{{"dxgi.dll",Core.ProxyHash},{"nvngx.dll_dlssnr.dll",Core.ForwardHash},{"nvngx_dlssnr.dll",Core.ModelHash}}){string p=Path.Combine(dir,kv.Key);report.Add(kv.Key+": "+(!File.Exists(p)?"missing":Core.Hash(p)==kv.Value?"matches package SHA-256":"different build"));}
    report.Add(RuntimeDiagnostics.Signature(Path.Combine(dir,"nvngx_dlssnr.dll")));
    report.Add(RuntimeDiagnostics.Evaluation(g.Exe));
    BeginInvoke((Action)(()=>Log(string.Join(Environment.NewLine,report))));
   });
  }
  void Hardware() {
   string info=Prerequisites.GpuInfo();Log("GPU, driver, VRAM: "+info);
   bool old=Prerequisites.NeedsDriver(info);
   string text=info+"\n\nValidated: RTX 3090, driver 616.56. Other configurations need testing.\n\n"+(old?"Install the bundled 616.56 driver now? The NVIDIA installer will guide you; close games first. A restart may be required.":"Driver is at least the tested version. Reinstall the bundled driver only if you need it; this could downgrade a newer driver.");
   if(MessageBox.Show(this,text,"NR Studio system setup",old?MessageBoxButtons.YesNo:MessageBoxButtons.OK,MessageBoxIcon.Information)==DialogResult.Yes)Prerequisites.InstallDriver(AppDomain.CurrentDomain.BaseDirectory);
  }
 }
}
