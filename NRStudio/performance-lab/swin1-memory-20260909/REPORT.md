# SWIN1 cache and compiler screening ? 9 September 2026

Decision: reject all candidates; no installed changes.

Used swin1-prototype.ptx, the arithmetic-preserving compatibility reconstruction, rather than the rejected compact direct-conversion family. Compiled O3 and O2, plus O3 variants changing 65 ordinary global load sites to .cg or .cs. Stores, synchronization, arithmetic, parameter layout, resolution, and evaluation cadence unchanged. All four binaries have distinct hashes. Each compiled to 218 registers, zero spills.

Private current-1.3.4 forwarders add SWIN1 resource 109. Existing exact preprocessing, prepared post, and SWIN8 stay active in both modes. Only SWIN1 switches against the original kernel. Twelve alternating output hashes matched the validated baseline for each variant (48 frames total), with valid NR pipeline timestamps. Each then ran 200 alternating timing frames, first 32 excluded.

| Variant | Current model ms | Candidate model ms | Regression |
| --- | ---: | ---: | ---: |
| default-o3 | 32.1838 | 33.8857 | 5.29% |
| schedule-o2 | 32.1398 | 33.8575 | 5.34% |
| l2-loads | 32.1444 | 33.8586 | 5.33% |
| stream-loads | 32.1326 | 33.8452 | 5.33% |

The reconstructed kernel itself costs substantially more than the original. Cache hints and the compiler optimization level did not rescue it. Timing differences between variants are not paired tests of the cache hint effect, and must not be interpreted as small improvements. These results do not isolate the original kernel's memory bottleneck. Further work should first profile the original kernel rather than continue sweeping this slower reconstruction. No Nsight hardware-counter profile was collected in this experiment.

No broader quality testing or deployment warranted for the regressions. Output equality refers to the tested synthetic-guide model frames, not all possible inputs. All raw logs and artifacts retained. Installed gains and game settings preserved; game remains closed from the preceding offline test session.

Cache semantics reference: [NVIDIA PTX ISA](https://docs.nvidia.com/cuda/archive/11.8.0/parallel-thread-execution/index.html). Cache operators are performance hints; .cg bypasses L1 and .cs applies streaming eviction policy.
