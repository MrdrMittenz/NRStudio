using System;
using System.Drawing;
using System.Windows.Forms;

namespace NRStudio {
 public sealed class HelpForm:Form {
  readonly Color ink=Color.FromArgb(225,233,240), panel=Color.FromArgb(26,33,44), accent=Color.FromArgb(109,221,190);
  readonly TabControl tabs=new TabControl{Dock=DockStyle.Fill};

  public HelpForm() {
   SuspendLayout();AutoScaleMode=AutoScaleMode.None;
   Text="NR Studio Help";ClientSize=new Size(860,640);MinimumSize=new Size(640,460);
   StartPosition=FormStartPosition.Manual;ShowInTaskbar=false;
   Font=new Font("Segoe UI",10.5f);BackColor=panel;ForeColor=ink;Padding=new Padding(16);
   var layout=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=1,RowCount=3};
   layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));
   layout.RowStyles.Add(new RowStyle(SizeType.Absolute,48));
   layout.RowStyles.Add(new RowStyle(SizeType.Percent,100));
   layout.RowStyles.Add(new RowStyle(SizeType.Absolute,48));Controls.Add(layout);
   layout.Controls.Add(new Label{Text="Choose a Help tab. Scroll to read each control's description.",Dock=DockStyle.Fill,TextAlign=ContentAlignment.MiddleLeft,ForeColor=accent},0,0);
   layout.Controls.Add(tabs,0,1);
   var close=new Button{Text="Close",DialogResult=DialogResult.Cancel,Anchor=AnchorStyles.Right,Size=new Size(100,34),FlatStyle=FlatStyle.Flat,ForeColor=ink,BackColor=panel};
   close.Click+=(s,e)=>Close();layout.Controls.Add(close,0,2);CancelButton=close;

   Page("Start here",
    "What NR Studio does", "NR Studio installs and manages an experimental neural-rendering pass for compatible games. The model changes image detail, colour and lighting appearance. DLSS upscaling enlarges the game image; NR adds its own image processing. A stronger visible effect does not always mean a more realistic result.",
    "First setup", "Add the game's actual 64-bit executable, select it in Your games, close the game, then click Install / update NR. Launch the game and select DirectX 12 and native DLSS where supported. Press Insert to open the in-game NR Studio menu. Adding a game to the library does not prove compatibility.",
    "Desktop settings: save, then launch", "All controls in the desktop app edit settings for the selected game. Close the game before clicking Save settings, then launch it to apply them. Editing a value or changing the selected game does not save it automatically. Each game has its own settings.",
    "In-game settings: preview, then save", "The Insert menu lets you preview appearance changes while playing. Detail strength, Colour strength, Paper white and Highlight guard apply as you drag. Model sliders apply when released and can briefly pause while the model rebuilds. Click Save profile to keep these values for future launches.",
    "Before / after upscaling needs a restart", "The desktop NR processing order and the in-game NR before upscaling checkbox select the same setting. A running game keeps its current order until restarted. The in-game checkbox saves this choice immediately; other in-game values use Save profile.",
    "What auto means", "auto uses the runtime's default for that setting. It does not automatically tune the image or target an FPS. Numeric boxes also accept typed values within the supported range; use a decimal point, such as 0.75. Automatic skin mask accepts auto, true or false.",
    "Waiting for native DLSS frames", "The menu can open even when NR is not evaluating. Select native DLSS in the game and check the live panel for Native NR running. In STALKER, a separate super-resolution profile can retain TSR even when another settings file lists DLSS; reselect and apply DLSS in-game. DirectX 12 alone is insufficient.",
    "Comparing changes", "Change one setting at a time. Use Comparison > Wipe to examine the original and NR result in the same scene. Check faces, fine textures, bright lights and moving objects. Turn Comparison and any Debug view off for normal play. Before upscaling and lower Model resolution can alter detail and motion quality.",
    "Game support", "Runtime profiles cover RTX 3090 and RTX 40-series (Ada). RTX 3090 has been tested; Ada hardware validation is pending. A newer series does not guarantee higher FPS than a 3090: GPU model, VRAM, game and settings matter. Games need a compatible input path; DirectX 12 alone is insufficient. The 7 Days to Die profile does not support EAC-required servers.");

   Page("Appearance",
    "Composition (0 current / 1 residual)", "0 retains the current composition. 1 uses the experimental matched residual: subtract the prepared model input from the matching model output, store the signed edit, and enlarge only that edit onto the original frame. Requires the 1.3.0 runtime or later. Reduced resolution can change the result. One STALKER scene test reached 32.23 presentation FPS at 0.67 scale; that is not a universal FPS guarantee or proof of equal visual quality.",
    "Residual HDR resizing", "When reducing an HDR-derived model input, NR Studio averages its represented light before encoding it again. This avoids making thin lights artificially dark just because they cover only part of a smaller pixel. It preserves light in the prepared proxy, not clipped scene highlights or missing model detail. Full-resolution processing and Current composition retain their existing paths. No additional slider is needed.",
    "Residual tone and Residual detail (0 to 2)", "These live controls apply only in residual composition. Tone controls broad spatial changes using a 3 by 3 blur of the residual; detail controls what remains. Both at 1 apply the whole edit. Tone at 0 suppresses broad changes. These are spatial frequencies, not semantic lighting and material labels, so inspect the result. Both at 0 return the original but the model still runs. They do not rebuild the model.",
    "Residual highlight guard", "In residual mode the guard caps luminance brightening for colour edits as well. Signed HDR edits are scaled by Paper white and added to the retained original. At zero edit the original pixels and alpha pass through exactly. This does not prove that a lower-resolution model produces the same visual result as the full-resolution model.",
    "Enable neural rendering", "Turns the NR pass on or off for this game. Off shows the image without NR's edits. It does not uninstall the runtime or change the game's DLSS setting. Use this switch for an NR on/off performance comparison; setting a strength slider to zero can still leave the model running.",
    "Detail strength (0 to 2)", "Controls how much of the model's processed image is blended into the original. 0 removes this visible blend; 1 applies the model's result before the colour and highlight controls. Values above 1 exaggerate the change and can create an overprocessed look. This is a blend control, not the model's Intensity setting.",
    "Colour strength (0 to 1)", "Controls how much of the model's colour is used. 0 keeps the game's colour while allowing NR brightness changes; 1 allows the model's colour as well. Intermediate values blend between them. Lower this if the game's original colour palette is important to you.",
    "Paper white (0.25 to 4)", "Sets the brightness reference used to prepare a linear game image for the model. A higher value sends a darker image to the model; a lower value sends a brighter one. This can change how the model treats bright areas. It is not a monitor brightness or HDR-nits setting. Already tone-mapped inputs bypass this preparation, so the effect depends on the game.",
    "Highlight guard (1 to 8)", "Limits the brightness multiplier in the luminance correction used when blending the result. Lower values restrain that brightening; higher values allow more. It does not limit darkening. Colour strength also affects the final blend, so this is not a universal cap on every output pixel.",
    "Intensity (0 to 2)", "Requests the model's internal processing strength. The model and chosen profile determine the response; it need not increase evenly as the value rises. Unlike Detail strength, this changes model processing and rebuilds the model when changed in-game.",
    "Local structure (0 to 2)", "Requests how strongly the model treats local shapes, edges and texture structure. Use small changes and inspect fine detail and motion. The exact response is model-dependent; a higher number is not a guaranteed detail improvement.",
    "Local tone (0 to 2)", "Requests the model's treatment of local brightness and tonal contrast. It can affect the appearance of light and shade within surfaces. This is an internal model setting, not the game's exposure or gamma control; the visible response depends on the profile and scene.",
    "Skin structure (-1 to 2)", "Requests a separate structure strength for skin. -1 follows Local structure; it does not mean zero strength. Values of 0 or higher request an independent skin setting. Inspect faces closely when adjusting it, especially with Automatic skin mask enabled.",
    "Automatic skin mask (auto / true / false)", "true asks the model to identify skin automatically for its skin-specific processing. false disables that automatic mask; auto uses the runtime default. Detection and the visible effect depend on the model and scene. It is not a manual face-selection tool.");

   Page("Model & speed",
    "NR processing order: After upscaling", "The default. NR processes the image after DLSS has enlarged it. This keeps the existing processing order used by the app. With Model resolution at 1, NR works at the full image size at this stage; lower Model resolution values still reduce the model's own working size.",
    "NR processing order: Before upscaling", "Experimental. NR processes the smaller render-resolution image before DLSS enlarges it. This can reduce its cost, but fine detail and motion may differ. NR still runs on each evaluated frame. Unsupported input layouts fall back to the after-upscaling route. Close the game, save, and relaunch to apply the choice.",
    "Model resolution (1 = 100%)", "Sets the model's working width and height relative to the image at the chosen processing stage. 1 uses both dimensions in full; 0.5 uses about half the width and height, or one quarter of the pixels. Lower values can improve performance at a cost to NR detail. This does not change the game's output resolution or its DLSS Quality/Balanced/Performance selection. The supported range is 0.25 to 1.",
    "Style (0 standard / 1 natural / 2 cinematic)", "Selects one of the model's processing profiles. Standard, Natural and Cinematic are community labels for profiles 0, 1 and 2, not official descriptions or a quality ranking. Their appearance varies by scene. Compare them in-game; changing Style rebuilds the model.",
    "Model preset (0 to 3)", "Selects an internal NR model preset. 0 is Default; 1, 2 and 3 request alternatives. These are not the game's DLSS upscaling presets, and larger numbers do not guarantee better quality or speed. Exact differences are undocumented. Desktop changes apply next launch; the Insert menu can rebuild the model during play.",
    "Which changes can cause a pause?", "In-game Model resolution, Intensity, Local structure, Local tone and Skin structure apply when you release the slider. Style, Model preset and Automatic skin mask also rebuild model resources. Wait for the rebuild before judging the result. Processing order requires a full game restart.",
    "Understanding performance", "NR adds GPU work. Its cost depends on the scene, image size, model settings and game integration. Before upscaling and Model resolution are the main size-related options here; neither guarantees a particular FPS or identical visuals. The game's own DLSS and graphics settings also affect overall performance.");

   Page("GPU monitor",
    "GPU / VRAM monitor", "Opens a separate window with NVIDIA's GPU counters: VRAM used, free and total; GPU load; temperature; power draw and limit; graphics and memory clocks. Choose an adapter at the top if your PC has several GPUs. The selected adapter is not automatically identified as the game's GPU.",
    "Reading VRAM", "These counters cover the whole selected GPU, including Windows and other apps. Free is the driver's reported free physical VRAM; it is not the game's Windows memory budget or a safe cache-allocation target. Used plus free can be below total because of reserved memory. Unavailable counters are shown as Unavailable, never as zero.",
    "Refresh every 3 seconds", "Reads new counters while this window is open and not minimized. Uncheck it to pause. Refresh now takes one sample, including while paused. The timestamp shows when that sample was read. A failed query clears the readings so an old number is not mistaken for a live value. Close this window to stop monitoring.",
    "VRAM history", "The line shows used VRAM as a share of total across the most recent 40 samples. This is a memory graph, not an FPS or frame-time graph. Changing adapters clears the history. Pause or close monitoring during controlled FPS tests; background monitoring itself adds some work.",
    "Selected game evidence", "Shows saved settings for the game selected in the main window, followed by recent or historical native evaluation evidence. Saved settings apply at the next launch and do not prove the live processing order. Use the Insert menu for the active path, fallbacks and NR GPU cost. This window does not measure game FPS.",
    "Copy snapshot", "Copies the displayed counters, sample time, monitoring status and selected game's evidence to the clipboard. A paused snapshot retains its original timestamp. It is a diagnostic note, not a benchmark. Nothing is uploaded automatically.",
    "What spare VRAM can do", "Spare VRAM can hold reusable model data or intermediate results when the runtime has a validated use for them. Allocating more memory by itself does not speed up the model. Full-resolution rendering remains the quality reference; lowering model resolution or moving NR before upscaling can change visuals.");

   Page("In-game controls",
    "Insert", "Opens or closes the NR Studio overlay in a compatible running game. The desktop app does not need to stay open. Changes saved from the desktop apply at the next launch; use this overlay to preview changes during play.",
    "Save profile", "Writes the current in-game settings for future launches. Look for the saved confirmation. A save error usually means the game folder could not be written. The before-upscaling checkbox saves its own restart choice separately.",
    "Comparison: Off / Side by side / Wipe", "Off shows the normal NR result. Side by side displays the original and processed images together for inspection. Wipe reveals parts of each image on opposite sides of a movable divider. Original means the image without NR's edits, not a separate native-resolution rendering mode. With NR before upscaling, the comparison is made before DLSS's final processing.",
    "Wipe position / Split", "Moves the divider across the image. 0 places it at the left edge, 0.5 in the centre and 1 at the right edge. Comparison stays active when you close the overlay; set it to Off when finished.",
    "Label original and NR / Label the sides", "Draws labels identifying the two comparison images. In the advanced NR panel, Swap sides exchanges the images and Label size changes the text size. Labels can appear in screenshots.",
    "Zoom (advanced NR panel)", "Changes how much of each comparison image is shown in Side by side mode. Increasing it shows a closer crop; it does not increase the model's resolution or improve rendering quality.",
    "NR before upscaling (restart required)", "The in-game equivalent of the desktop processing-order choice. Checked requests Before; unchecked requests After. Restart the game to apply it. The current-order text can still show the old choice while a restart is pending.",
    "Native NR running / GPU pass", "Running indicates that the NR pass is active. GPU pass time, when available, measures the NR work in milliseconds, including its supporting copies and blending. Lower is faster. It is not the entire frame time or a displayed FPS counter; timing pending means no measurement is available yet.",
    "Waiting / NR unavailable / Retry NR", "Waiting means the runtime has not received suitable native DLSS frames; check the game's upscaling setting and supported scene. Unavailable includes the recorded failure reason. Retry NR asks the runtime to try again after a failure; it does not install missing files or guarantee compatibility.",
    "Advanced OptiScaler / Back to NR Studio", "Opens the underlying integration menu, which includes more upscaling and diagnostic settings. Back to NR Studio returns to the main NR controls. The same NR values are shared between the two menus. The wider OptiScaler controls have their own contextual help; leave unfamiliar integration settings at their working values.",
    "Capture 8 frames (advanced NR panel)", "Saves eight pairs of raw images from before and after NR processing into dlssnr-capture in the game folder. This is a diagnostic capture, not a video recording. It can pause the game and use substantial disk space; a new capture overwrites the previous capture in that folder.",
    "Debug view (advanced NR panel)", "Off restores the normal image. Proxy shows the image supplied to the model; Model output shows its raw result; Difference amplifies the change around grey. These views help inspect processing and are not intended as normal image-quality modes.");

   Page("Library & tools",
    "Your games", "Selects which game's settings and installation the app displays and manages. The executable path below the game name identifies the selected game folder. Save edits before selecting another game if you want to keep them.",
    "+ Add game", "Adds the game's actual executable to your local library. For Unreal games this is often the Shipping executable inside Binaries\\Win64, rather than a launcher. Known profiles can supply launch options. Adding does not install NR; use Install / update NR afterwards.",
    "Remove from library", "Removes the selected library entry. A game managed by NR Studio must have its original files restored first. This button does not delete the game or uninstall it from Steam.",
    "Install / update NR", "With the selected game closed, installs this package's runtime into its folder or updates an installation managed by NR Studio. Original files are backed up. Updates keep the game's saved settings and original backups. This does not update every game in the library at once or download a new model.",
    "Save settings", "Saves the desktop values and processing order for the selected game's next launch, with a settings backup. Close the game first. It does not change a running game's live sliders. Older runtimes need Install / update NR before using the new Before upscaling option.",
    "Restore original files", "Returns the files managed by NR Studio to their state before its first installation into that game. If another NR setup was present then, that earlier setup is restored. Current settings are archived. Close the game first and keep its backup folder. Unexpected DLL changes can stop restoration to protect other modifications.",
    "Launch game", "Starts the selected game using its saved launch profile. STALKER 2 uses Steam; other profiles can include required launch arguments. Unsaved desktop edits are not applied automatically.",
    "Check runtime", "Checks the selected game's runtime files against this package, reports the model's signature status, looks for native evaluation evidence in logs, and shows the saved order for next launch. Read the report in the activity area. File matches alone do not prove NR is currently running; historical log evidence describes an earlier run.",
    "Open game folder", "Opens File Explorer at the folder containing the selected executable. This is where the runtime, configuration and installation backups are stored.",
    "GPU / driver setup", "Shows the detected GPU, graphics driver and VRAM. If the driver is older than the tested version or cannot be identified, it offers the bundled NVIDIA installer. Viewing the information does not change the driver; an installation requires choosing that action in its prompts.",
    "Package contents", "Opens the installed README with setup instructions, compatibility notes, source and licence information, and the model-audit explanation. It describes what ships with this installer.",
    "Help", "Opens this guide. Reading it does not alter settings. Use the tabs, scroll wheel or keyboard to browse; Close or Escape returns to the app.",
    "Installation status", "Not managed means NR Studio has no installation record for this folder; another setup may still exist. Installed means a managed installation is recorded, not that NR has been verified in the current scene. Installing, Updating or Restoring describe operations in progress; if left after an interruption, use Restore original files for recovery.",
    "NR model ready / Model missing", "Shows whether the bundled model file is present for installation. Ready is a file-availability check, not proof of game compatibility or an authenticated official NVIDIA release. Check runtime provides the detailed file and signature report.",
    "Activity area", "The box at the bottom of the app shows timestamped progress, completion messages, errors and runtime-check results. Scroll it to read earlier messages. Wait for an operation to finish before starting another.",
    "Uninstalling NR Studio", "Use Windows Installed apps to remove the desktop app. This does not restore the runtime files in your games. Use Restore original files for each managed game first if you want those installations undone. Your game library and external backups are retained.");
   using(var graphics=Graphics.FromHwnd(IntPtr.Zero))Studio.ScaleLayout(this,graphics.DpiX/96F);
   ResumeLayout(true);
   tabs.SelectedIndexChanged+=(s,e)=>BeginInvoke((Action)ScrollToStart);
   Shown+=(s,e)=>{
    var area=Screen.FromControl(Owner??this).WorkingArea;
    if(Width>area.Width || Height>area.Height){MinimumSize=Size.Empty;Size=new Size(Math.Min(Width,area.Width),Math.Min(Height,area.Height));}
    var anchor=Owner==null?area:Owner.Bounds;
    Location=new Point(Math.Max(area.Left,Math.Min(anchor.Left+(anchor.Width-Width)/2,area.Right-Width)),Math.Max(area.Top,Math.Min(anchor.Top+(anchor.Height-Height)/2,area.Bottom-Height)));
    BeginInvoke((Action)(()=>{tabs.Focus();ScrollToStart();}));
   };
  }

  void ScrollToStart() { if(IsDisposed || tabs.SelectedTab==null)return;var text=(RichTextBox)tabs.SelectedTab.Controls[0];text.Select(0,0);text.ScrollToCaret(); }

  void Page(string title,params string[] content) {
   var page=new TabPage(title){BackColor=panel,Padding=new Padding(16)};
   var text=new RichTextBox{Dock=DockStyle.Fill,ReadOnly=true,BorderStyle=BorderStyle.None,BackColor=panel,ForeColor=ink,Font=Font,WordWrap=true,ScrollBars=RichTextBoxScrollBars.Vertical,DetectUrls=false,AccessibleName=title+" help"};
   using(var heading=new Font(Font,FontStyle.Bold)) {
    for(int i=0;i<content.Length;i+=2) {
     text.SelectionFont=heading;text.SelectionColor=accent;text.AppendText(content[i]+Environment.NewLine);
     text.SelectionFont=Font;text.SelectionColor=ink;text.AppendText(content[i+1]+Environment.NewLine+Environment.NewLine);
    }
   }
   text.Select(0,0);text.ScrollToCaret();page.Controls.Add(text);tabs.TabPages.Add(page);
  }
 }
}
