# Cooling-test preparation and CPU overhead measurement

2026-09-06. No fan or hardware settings were changed. The game remains closed.

The initial fan state read through the installed MSI Afterburner Control SDK is
0% at idle, automatic flag 1, supported range 30-100%. MSI Afterburner's custom
software fan curve is disabled. This does not establish that the hardware fan
curve is defective; the prior gameplay observation separately confirmed software
thermal slowdown at 85 C. Cooling-test noise preference remains pending.

fanctl.cpp builds against the locally installed SDK Include/MACMSharedMemory.h.
It follows the SDK's named mutex and FLUSH acknowledgement protocol, limits
writes to fan speed/mode and the command field, validates v2.3 layout, one GPU,
and the RTX 3090 PCI prefix. It rejects busy/unsupported layouts. `fanctl inspect`
is read-only. `fanctl set 80` requests manual 80% and `fanctl auto` restores the
observed original automatic control. Write commands have NOT been executed or
validated in this session. No MSI SDK header/library is copied into the review.
The current GPU is a single 3090; this helper is not a general fan-control app.

An isolated CPU timing forwarder measured the duration of the native model
EvaluateFeature call using QueryPerformanceCounter. Ten 1440p evaluations
completed; the warm median was 0.2851 ms. The final output was byte-identical to
the prior ten-frame original-model reference. This measures model CPU command
recording only, excluding the forwarder's parameter setup and logging. It does
not measure GPU execution, all host overhead, or game FPS.

Source inspection of OptiScaler's normal NR pass found no explicit per-frame
CPU fence wait. Full-frame captures are conditional on an active capture request.
Its normal timing readback reads timestamp values. These observations and the
CPU measurement provide no evidence of a large avoidable host-side stall that
would explain the approximately 40 ms measured GPU model work.

The CPU diagnostic DLL is isolated here and must not be installed into the game
or app. All deployed files remain unchanged. No additional speedup is claimed.
Build CPU diagnostic with build_cpu.cmd; use the existing native_probe with the
installed core/model and this directory's diagnostic forwarder for reproduction.

## Fan-control smoke test, 2026-09-06

The user authorized temporarily louder fans. Afterburner acknowledged manual
80%, and its shared-memory readback showed fan_percent=80, fan_flags=0. After
four seconds nvidia-smi reported 64% during the ramp (this sensor reports intended
speed, not proof of physical RPM). Automatic mode was then restored and read
back as fan_flags=1. Power limit remained 350 W. The GPU was at 46 C during this
brief idle test. It does not establish performance or cooling under game load.

STALKER was relaunched. game-benchmark/run_cooling.py prepares an automatic-fan
30-second sample, then manual 80%, a 60-second cooling period and another
30-second sample. It restores automatic fan mode in finally and verifies the
mode readback. Fan/temperature/clock/thermal-slowdown telemetry is recorded.
The loaded gameplay comparison is pending the user's ready signal.
The earlier statement that write commands were untested describes the state
before this smoke test; the commands have now completed with acknowledged
manual and restored automatic mode.

## Loaded-game comparison completed

See game-benchmark/REPORT.md. Automatic fans already requested 83-86%, so
manual 80% was less cooling, not more. Mean temperature rose from 82.20 to
84.19 C; FPS was effectively unchanged (19.118 versus 19.147). Neither interval
recorded thermal slowdown. Automatic fan control was restored and verified.
No fan, quality or runtime change is recommended from this experiment.
