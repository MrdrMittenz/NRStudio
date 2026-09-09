# Preprocessing register allocation experiment

Candidate stays private; installed 1.3.4 unchanged.

Four synthetic seeds matched all candidates to current 224-register output with passing allocation guards. The 255 limit compiles to 240 registers without spills and improved isolated kernel timing, but full-pipeline tests did not establish a repeatable improvement.

Twelve alternating actual-model frames matched the earlier validated baseline exactly. Three 200-frame alternating pipeline tests, discarding first 32 frames:

- alternate-timing01: 0.443% lower model time.
- alternate-timing02: -0.119% lower model time.
- alternate-timing03: 0.034% lower model time.

Control uses resource 109 containing current validated 224-register preprocessing; candidate resource 108 contains the 255-limit variant. Prepared post and SWIN8 stay enabled in both modes. No new gameplay FPS gain claimed and no deployment warranted.
