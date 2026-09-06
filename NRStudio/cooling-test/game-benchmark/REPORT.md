# Fan comparison result, 2026-09-06

STALKER 2 PID 19932, RTX 3090, driver 616.64. NR remained enabled at
2560x1440, with 1972x1108 guides and the optimized post kernel.

| Setting | Average FPS | 1% low FPS | Mean GPU temperature | Reported fan speed |
| --- | ---: | ---: | ---: | ---: |
| Automatic | 19.118 | 17.942 | 82.20 C | 83-86% |
| Manual 80% | 19.147 | 18.082 | 84.19 C | 80% |

The chosen 80% setting was LOWER than the automatic request under this load.
It increased mean temperature by about 2 C after the 60-second settling period.
It was not an effective stronger-cooling intervention. The +0.029 FPS observed
difference is negligible and does not establish a speedup. Automatic control is
restored and was verified by Afterburner readback fan_flags=1.

Neither measured interval reported active software thermal slowdown, and its
cumulative counter did not increase in either interval. This does not invalidate
the earlier moment where slowdown was observed; it shows that it was not an
ongoing limiter during these samples. These results do not support changing the
fan setting to improve current FPS. They do not measure a stronger-cooling option.

Both captures remained foreground. The automatic sample recorded input activity;
the manual sample was input-idle. There is one trial per condition, in fixed order,
with no randomized repeat. Fan percentages from nvidia-smi represent intended
speed, not an independent measurement of physical fan RPM. Clocks averaged
1769.5 and 1767.1 MHz respectively. All 11 logged NR evaluation samples returned
success. Presentation intervals were not classified for frame generation.

Quality settings and deployed runtime files are unchanged. The game remains
running with optimized NR and automatic fan control. The results narrow the
current limitation back to the rendering workload; no additional FPS gain is
claimed. Source/measurement text is in the private review; screenshots stay local.
