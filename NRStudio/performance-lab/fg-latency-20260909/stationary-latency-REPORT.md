# Stationary FG latency comparison

2026-09-09, RTX 3090, STALKER 2 PID 8048. User operated FG menu. Agent made no setting changes. Both captures lasted 15 seconds, passed idle input/timestamp checks, foreground and display checks, and had no PresentMon warnings. Screenshots show matching static scene landmarks, with natural foliage, lighting and hand animation differences. Single off/on pair, separated by approximately three minutes, not repeated ABBA.

| Mode | Observed display FPS | Instrumented latency mean | p95 latency | Valid latency samples |
|---|---:|---:|---:|---:|
| FG off | 21.1431 | 57.0669 ms | 58.7845 ms | 312 |
| FG on | 39.7583 | 59.1446 ms | 60.5909 ms | 294 |

Observed display rate increased approximately 88.05%; mean instrumented latency increased 2.0777 ms in this pair. This is instrumented frame-start-to-display, not hardware click-to-photon. Frame types remain labeled Application, missing latency values are excluded, and no independent validation of generated-frame marker association exists. This pair does not establish a universal 2.1 ms FG penalty or verify Reflex sleep effectiveness. It shows substantial delay already present with FG off and does not support blaming the full observed latency on FG.

FG remains on per user selection. No optimization or latency fix deployed; all previous NR improvements remain installed. Raw CSV, metadata, input logs, GPU samples, pipeline timings and screenshots use reflex-off-stationary1 and reflex-on-stationary1 prefixes. Summary: stationary-latency-comparison.json.
