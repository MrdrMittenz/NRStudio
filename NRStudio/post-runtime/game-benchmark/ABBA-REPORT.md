# Same-session kernel comparison, 2026-09-06

STALKER 2 PID 17996, RTX 3090, driver 616.64. NR stayed enabled at 2560x1440
with 1972x1108 guides. The benchmark event selected original/optimized/optimized/
original, with a five-second settling period and 30-second capture per trial.
Runtime mode acknowledgements were verified before and after every capture.

| Kernel | Trial | Average FPS | 1% low FPS |
| --- | --- | ---: | ---: |
| Original | 1 | 18.888 | 17.835 |
| Optimized | 2 | 18.993 | 17.831 |
| Optimized | 3 | 19.009 | 17.739 |
| Original | 4 | 18.893 | 17.913 |

Arithmetic means of the two trials per mode: original 18.891 FPS, optimized
19.001 FPS. The observed difference is +0.110 FPS (+0.584%). The optimized
samples show no improvement in 1% lows. This is a small observed difference,
not an established reliable speedup or a 22% increase in game performance.

All captures remained in the foreground, but each recorded one input-timestamp
change. They are retained and marked uncontrolled. The input monitor cannot
identify which input device or software produced those events. No input was
sent by the benchmark controller. Two trials per mode do not establish a
confidence interval. PresentMon measures presentation intervals; generated
frames were not independently classified.

All 23 sampled native evaluations returned success. Model and guide sizes were
unchanged. GPU temperature samples ranged from 84 to 86 C; mean reported GPU
clocks per trial were approximately 1739-1757 MHz. These readings alone do not
establish thermal throttling. Gameplay screenshots and raw telemetry remain
local. Text measurements and analysis are included in the private source review.

The controller restored optimized mode. A fresh runtime log acknowledgement
confirmed optimized mode and subsequent candidate launches. The game remains
running. No graphics or NR quality settings changed. This scene remains below
30 FPS; further gains require addressing more of the frame workload.
