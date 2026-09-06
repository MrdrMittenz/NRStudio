# STALKER 2 optimized runtime gameplay samples

2026-09-06, RTX 3090, driver 616.64, PID 5640. The deployed forwarder
SHA-256 was verified against deployment.json and its opt-in marker exists.
Screenshots confirm loaded gameplay; the game stayed foreground throughout.

| Trial | Mean FPS | 1% low FPS | Mean frame ms | P99 frame ms |
| --- | ---: | ---: | ---: | ---: |
| optimized-gameplay-01 | 19.34 | 17.82 | 51.71 | 54.08 |
| optimized-gameplay-02 | 19.31 | 18.15 | 51.77 | 53.82 |

Both approximately 30-second PresentMon captures include successful native
NR evaluation samples at 2560x1440 with 1972x1108 guides. Candidate launch
messages occur during both captures. No failed evaluations appear in their
log excerpts. These facts confirm use of the optimized path in gameplay;
they do not verify every output pixel or guarantee long-session stability.

Both trials recorded input activity, so neither is a controlled stationary
comparison. The earlier 18.93 FPS measurement used a different session and
is not an equivalent baseline. No measured FPS speedup is claimed. Presentation
intervals are not independently classified for frame generation. This scene
runs around 19.3 FPS and does not meet 30 FPS. The kernel's isolated speed gain
must not be reported as a 22% improvement in game FPS.

The current candidate is left installed and running. Settings are unchanged.
Raw CSV, GPU telemetry, screenshots and metadata remain in this local folder.
Source review includes text measurements only. Further optimization needs a
matched baseline and candidate comparison in the same scene.
