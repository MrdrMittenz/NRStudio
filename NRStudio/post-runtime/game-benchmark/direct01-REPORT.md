# STALKER 2 direct-activation kernel comparison

Previous validated optimized post kernel and original swin8; NR remains enabled in both modes.

| Trial | Kernels | Average FPS | 1% low FPS | Input timestamp changes |
| --- | --- | ---: | ---: | ---: |
| 1 | Previous | 19.434 | 16.024 | 1 |
| 2 | New | 19.917 | 18.829 | 2 |
| 3 | New | 19.936 | 18.278 | 0 |
| 4 | Previous | 19.441 | 18.166 | 0 |

Pooled previous: 19.437 FPS. Pooled new: 19.926 FPS. Observed difference: +0.489 FPS (+2.51%).

All trials controlled: False.

Presentation intervals; generated frames not independently classified. Two trials per mode. Input timestamp changes cannot be attributed to a device or person by this monitor.

Native evaluation samples: 24. Logged failures: 0.

New kernels restored and acknowledged, with subsequent candidate launches: True.
Raw gameplay images remain local. This comparison alone does not establish a gain across other scenes or games.

The final input-idle pair measured 19.441 FPS previous versus 19.936 FPS new (+2.55%). There is only one input-idle sample per mode.
