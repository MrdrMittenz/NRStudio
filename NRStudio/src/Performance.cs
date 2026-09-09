using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace NRStudio {
 public sealed class PerformanceForm:Form {
  readonly Color ink=Color.FromArgb(225,233,240), panel=Color.FromArgb(26,33,44), accent=Color.FromArgb(109,221,190);
  readonly ComboBox adapters=new ComboBox{DropDownStyle=ComboBoxStyle.DropDownList,Dock=DockStyle.Fill,AccessibleName="Monitored GPU"};
  readonly TextBox readings=new TextBox(), evidence=new TextBox();
  readonly Label status=new Label{Dock=DockStyle.Fill,AutoEllipsis=true};
  readonly CheckBox live=new CheckBox{Text="Refresh every 3 seconds",Checked=true,AutoSize=true};
  readonly Timer timer=new Timer{Interval=3000};
  readonly Func<Game> selectedGame;
  readonly MemoryHistory history=new MemoryHistory();
  readonly Button refresh=new Button{Text="Refresh now",Size=new Size(150,32)},copy=new Button{Text="Copy snapshot",Size=new Size(150,32)};
  bool reading,changingAdapters;
  string gameReport="",lastSnapshot="";
  int generation;
  List<GpuSample> samples=new List<GpuSample>();

  public PerformanceForm(Func<Game> selection) {
   selectedGame=selection;SuspendLayout();AutoScaleMode=AutoScaleMode.None;
   Text="NR Studio - GPU / VRAM monitor";ClientSize=new Size(850,680);MinimumSize=new Size(650,650);StartPosition=FormStartPosition.Manual;ShowInTaskbar=false;
   Font=new Font("Segoe UI",10);BackColor=panel;ForeColor=ink;Padding=new Padding(18);
   var layout=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=7};
   layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));
   foreach(int height in new[]{38,152,100,64})layout.RowStyles.Add(new RowStyle(SizeType.Absolute,height));
   layout.RowStyles.Add(new RowStyle(SizeType.Percent,100));layout.RowStyles.Add(new RowStyle(SizeType.Absolute,34));layout.RowStyles.Add(new RowStyle(SizeType.Absolute,44));Controls.Add(layout);
   adapters.BackColor=panel;adapters.ForeColor=ink;layout.Controls.Add(adapters);
   foreach(var text in new[]{readings,evidence}){text.Multiline=true;text.ReadOnly=true;text.Dock=DockStyle.Fill;text.BorderStyle=BorderStyle.None;text.BackColor=panel;text.ForeColor=ink;text.ScrollBars=ScrollBars.Vertical;}
   readings.AccessibleName="GPU readings";readings.Text="Reading GPU counters...";layout.Controls.Add(readings);
   history.Dock=DockStyle.Fill;history.ForeColor=accent;layout.Controls.Add(history);
   layout.Controls.Add(new Label{Dock=DockStyle.Fill,Text="VRAM and load cover the whole selected GPU, including other apps. Free VRAM is not the game's Windows memory budget. Extra VRAM alone does not increase FPS.",Padding=new Padding(0,6,0,0)});
   evidence.AccessibleName="Selected game evidence";layout.Controls.Add(evidence);
   status.ForeColor=accent;layout.Controls.Add(status);
   var actions=new FlowLayoutPanel{Dock=DockStyle.Fill,Margin=Padding.Empty};
   foreach(var b in new[]{refresh,copy}){b.FlatStyle=FlatStyle.Flat;b.BackColor=panel;b.ForeColor=ink;b.Margin=new Padding(0,4,8,4);actions.Controls.Add(b);}copy.Enabled=false;
   live.Margin=new Padding(16,7,0,0);actions.Controls.Add(live);layout.Controls.Add(actions);
   refresh.Click+=(s,e)=>RefreshReadings();
   copy.Click+=(s,e)=>{if(lastSnapshot.Length>0)try{Clipboard.SetText("NR Studio 1.3.0 monitor snapshot\r\nStatus at copy: "+status.Text+"\r\n"+lastSnapshot);}catch(System.Runtime.InteropServices.ExternalException){status.Text="Clipboard is busy. Try Copy snapshot again.";}};
   live.CheckedChanged+=(s,e)=>{generation++;if(live.Checked){timer.Start();RefreshReadings();}else{timer.Stop();status.Text="Paused - displayed readings are a saved sample.";}};
   adapters.SelectedIndexChanged+=(s,e)=>{if(!changingAdapters){history.Clear();ShowSample(false);}};
   timer.Tick+=(s,e)=>RefreshReadings();
   Resize+=(s,e)=>{if(WindowState==FormWindowState.Minimized){generation++;status.Text="Paused while minimized - displayed readings are a saved sample.";}else if(live.Checked)RefreshReadings();};
   Shown+=(s,e)=>{
    var area=Screen.FromControl(Owner??this).WorkingArea;if(Width>area.Width||Height>area.Height){MinimumSize=Size.Empty;Size=new Size(Math.Min(Width,area.Width),Math.Min(Height,area.Height));}
    var anchor=Owner==null?area:Owner.Bounds;
    Location=new Point(Math.Max(area.Left,Math.Min(anchor.Left+(anchor.Width-Width)/2,area.Right-Width)),Math.Max(area.Top,Math.Min(anchor.Top+(anchor.Height-Height)/2,area.Bottom-Height)));
    timer.Start();RefreshReadings();
   };
   FormClosed+=(s,e)=>{generation++;timer.Stop();timer.Dispose();};
   using(var graphics=Graphics.FromHwnd(IntPtr.Zero))Studio.ScaleLayout(this,graphics.DpiX/96F);
   ResumeLayout(true);
  }
  static string GameEvidence(Game game) {
   if(game==null)return "Select a game in the main window to show its saved settings and native evaluation evidence.\r\nGame FPS and NR GPU time: use the in-game Insert menu or your frame counter.";
   try {
    string file=Path.Combine(Core.Folder(game),"OptiScaler.ini"),ini=File.Exists(file)?File.ReadAllText(file):"";
    return game.Name+"\r\nSaved for next launch: NR "+Core.GetIni(ini,"DlssNr","Enabled","not configured")+", "+(Core.BeforeUpscaling(game)?"before upscaling (experimental)":"after upscaling")+", model resolution "+Core.GetIni(ini,"DlssNr","WorkingScale","auto")+".\r\n"+
     RuntimeDiagnostics.Evaluation(game.Exe)+"\r\nLive processing order, NR GPU time and fallbacks: open Insert in-game. Game FPS is not measured by this monitor.";
   }catch(Exception e){return "Game evidence unavailable: "+e.Message;}
  }
  async void RefreshReadings() {
   if(reading||IsDisposed||WindowState==FormWindowState.Minimized)return;
   reading=true;refresh.Enabled=false;int request=++generation;var game=selectedGame();
   try {
    var sampleTask=Task.Run(()=>GpuMonitor.Read());var gameTask=Task.Run(()=>GameEvidence(game));
    await Task.WhenAll(sampleTask,gameTask);
    if(IsDisposed||request!=generation)return;
    gameReport=gameTask.Result;samples=sampleTask.Result;
    string uuid=(adapters.SelectedItem as GpuSample)?.Uuid;
    changingAdapters=true;adapters.Items.Clear();foreach(var sample in samples)adapters.Items.Add(sample);
    int index=samples.FindIndex(x=>x.Uuid==uuid);adapters.SelectedIndex=index>=0?index:0;changingAdapters=false;
    if(index<0)history.Clear();
    ShowSample(true);
   }catch(Exception e){if(!IsDisposed&&request==generation){status.Text="Unavailable - "+e.Message;readings.Text="No current GPU reading. Refresh to try again.";samples.Clear();adapters.Items.Clear();history.Clear();lastSnapshot="";copy.Enabled=false;evidence.Text=GameEvidence(game);}}
   finally {reading=false;if(!IsDisposed)refresh.Enabled=true;}
  }
  void ShowSample(bool addHistory) {
   var sample=adapters.SelectedItem as GpuSample;if(sample==null)return;
   readings.Text=sample.Describe();evidence.Text=Prerequisites.SupportSummary(sample.Name)+"\r\n"+gameReport;
   string timestamp=sample.CapturedUtc.ToLocalTime().ToString("HH:mm:ss");
   status.Text=(live.Checked?"Last sampled ":"Paused - sampled ")+timestamp+". Pause or close for clean FPS benchmarks.";
   lastSnapshot="Sampled UTC: "+sample.CapturedUtc.ToString("o")+"\r\n"+readings.Text+"\r\nWhole GPU readings; not the game's memory budget or an FPS measurement.\r\n"+gameReport;copy.Enabled=true;
   if(addHistory)history.Add(sample);
  }
 }
 internal sealed class MemoryHistory:Control {
  readonly List<GpuSample> points=new List<GpuSample>();
  public MemoryHistory(){DoubleBuffered=true;AccessibleName="VRAM usage history";}
  public void Clear(){points.Clear();Invalidate();}
  public void Add(GpuSample sample){points.Add(sample);while(points.Count>40)points.RemoveAt(0);Invalidate();}
  protected override void OnPaint(PaintEventArgs e) {
   base.OnPaint(e);float scale=e.Graphics.DpiX/96F;int left=(int)(8*scale),top=Font.Height+(int)(6*scale),width=Math.Max(1,Width-left*2),height=Math.Max(1,Height-top-left);
   using(var brush=new SolidBrush(ForeColor))e.Graphics.DrawString("VRAM used / total - last 40 samples",Font,brush,left,2*scale);
   using(var grid=new Pen(Color.FromArgb(65,85,99)))e.Graphics.DrawRectangle(grid,left,top,width,height);
   if(points.Count<2)return;var latest=points[points.Count-1];DateTime end=latest.CapturedUtc,start=points[0].CapturedUtc;double span=Math.Max(1,(end-start).TotalSeconds);
   PointF? previous=null;
   using(var pen=new Pen(ForeColor,2))foreach(var p in points) {
    if(!p.UsedMiB.HasValue||!p.TotalMiB.HasValue){previous=null;continue;}
    var xy=new PointF(left+(float)((p.CapturedUtc-start).TotalSeconds/span)*width,top+height-(float)(p.UsedMiB.Value/p.TotalMiB.Value)*height);
    if(previous.HasValue)e.Graphics.DrawLine(pen,previous.Value,xy);previous=xy;
   }
  }
 }
}
