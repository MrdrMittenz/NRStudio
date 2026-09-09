# SWIN1 instruction and occupancy follow-up

No candidate accepted or deployed. Existing installed optimizations preserved.

Inspected original machine instructions and tested 1,280 exact decode rewrites on the compact SWIN1 prototype. The five-operation unpack becomes four operations using LOP3 0xea. This is the same unsigned bit identity previously validated for SWIN8; FP16 multiplication is unchanged. Parsed static instruction sites: original 13,727, compact prototype 10,040, decode variant 10,047. The smaller source expression did not produce a smaller compiled kernel. Static counts are not dynamic execution counts or speed predictions.

Also tested compact prototype register limits 168 and 176 to examine the trade-off between occupancy and spills, following original-kernel profiling. The previously tested cap 192 and higher variants remain rejected.

Each candidate passed 12 alternating full-model frame hashes against the validated baseline. Each then ran three 200-frame cumulative-pipeline ABBA measurements, excluding the first 32 frames. Preprocessing, prepared post, and SWIN8 remain active in both modes. No graphics settings changed.

| Candidate/run | Baseline model ms | Candidate model ms | Time increase |
| --- | ---: | ---: | ---: |
| decode alternate-timing01 | 35.2466 | 35.5287 | +0.800% |
| decode alternate-timing02 | 34.7259 | 34.9952 | +0.776% |
| decode alternate-timing03 | 33.4290 | 33.1663 | -0.786% |
| occupancy168 alternate-timing01 | 35.6137 | 37.1272 | +4.250% |
| occupancy168 alternate-timing02 | 34.4300 | 35.7688 | +3.889% |
| occupancy168 alternate-timing03 | 34.9338 | 36.2204 | +3.683% |
| occupancy176 alternate-timing01 | 33.5237 | 34.5969 | +3.201% |
| occupancy176 alternate-timing02 | 34.9153 | 35.0991 | +0.526% |
| occupancy176 alternate-timing03 | 32.6175 | 33.0450 | +1.311% |

The decode variant did not improve consistently; both lower-register variants regressed in all three runs. Absolute baseline timing varied substantially during this session; no new clock-stability study was collected, so small differences are not sufficient evidence. Rejecting these candidates preserves earlier gains. No new gameplay FPS claim or broad image-quality claim follows from these tests. Full-model hash comparisons cover the tested synthetic-guide frames only.

This closes the current conversion/register/cache sweep. Further SWIN1 work should require a materially different transformation with evidence from source-correlated profiling, rather than more blind register-limit trials. Game remains closed for offline work. Resolution/HDR settings are unchanged.
