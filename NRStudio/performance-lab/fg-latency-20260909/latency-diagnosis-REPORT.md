# Latency diagnosis

Read-only inspection, 2026-09-09. No game menu, driver, power, clock or NR settings changed.

Clean fg-user-on1 display intervals: median 25.8962 ms, p95 27.8161 ms, p99 28.9157 ms, rate 38.3883/s. Clean fg-user-off3: median 49.1730 ms, p95 51.1759 ms, p99 53.0156 ms, rate 20.3443/s. Viewpoints differ slightly; these are indicative, not a strict matched-scene gain.

FG-on presents alternate approximately 0.36 ms and 51 ms apart while actual display intervals are near 26 ms. Thus the long application present intervals alone are not evidence of visible stutter. All frames are labeled Application, so no direct rendered/generated classification is available. Paired cadence suggests approximately 19 rendered frames/s with FG; this is an inference.

NR pipeline means: FG on 33.6983 ms (model 33.4731); off 33.9946 ms (model 33.7667). Samples have approximate capture boundaries and no frame-ID join. This points to substantial NR/model GPU cost. Subtracting pipeline time from total frame time is not a rigorous critical-path decomposition.

PresentMon DisplayLatency means: on 84.6450 ms, off 93.7476 ms. These are not click-to-photon latency and FG changes the presentation mapping. They cannot establish an input latency improvement or regression.

GPU capture readings show near-full utilization and graphics clocks mainly around 1770 MHz, temperatures 85-86 C. A later nvidia-smi snapshot shows P0, software power cap active and thermal slowdown inactive at that instant. Historical thermal counters are not evidence of thermal throttling during the captures. No low-clock state explanation is apparent; no power-limit increase is warranted by these observations.

OptiScaler log confirms Streamline selected Reflex 2.14.0, registered callbacks and logs frame-limit operations. This proves plugin loading, not correct per-frame low-latency sleep/marker operation through the SM86 mod. ForceReflex and latency replacement options remain auto. The displayed On+Boost selection is not sufficient to establish actual latency behavior.

Conclusion: useful display-rate improvement with reasonably even pacing; input response remains limited by slow underlying rendered cadence. Next technical targets are verify Reflex per-frame timing integration and profile remaining NR model kernels (SWIN4 backlog) while retaining every accepted optimization. No new performance improvement or latency fix was deployed by this inspection. Motion/HUD quality passed the user's subjective check, not an instrumented quality test.
