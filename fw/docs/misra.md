# MISRA C:2012 compliance (crew-box library)

**Scope:** `fw/lib/icom/src` and `fw/lib/icom/include`, the code shared by the host tests and the firmware.

**Checker:** cppcheck 2.13 with the MISRA addon, run by `fw/scripts/misra.sh`.
- The script fails on **any** output, because cppcheck's exit code does not cover every addon finding.
- The Zephyr glue in `fw/app/src/main.c` is out of scope.

**Note:** this is a developer-level check, not a qualified MISRA compliance tool. Without the licensed
rule texts, cppcheck reports rule numbers only.

## Result

There are no findings except deviation D1. These were fixed while setting up the check (9 Nov 2026):

| Rule | Where | Fix |
|---|---|---|
| 10.8 (composite expression cast to a different essential type) | `(float)(h + 1U)` in `anr.c` | written as `(float)h + 1.0f` |
| 8.9 (object used in one function should be defined there) | priority table in `mix.c` | moved into `icom_mix_frame` |
| 2.5 (unused macro) | `TELE_CRC` in `box.c` | removed (the last telemetry item is the `else` branch) |
| cppcheck variableScope | `frame.c` encoders | declarations moved into their blocks |
| cppcheck constParameter | `pcm` in `icom_mix_frame` | made `const`, through the `icom_chans_t` wrapper type (a `const` 2-D array parameter is not portable before C23) |

## Deviations

| ID | Rule | Category | Where | Reason |
|---|---|---|---|---|
| D1 | 2.5: a project should not contain unused macro definitions | Advisory | `icom.h` | `icom.h` is the public interface and the bus-protocol definition. Its channel indices and CCU mode codes are used by the application, the tests and the CCU emulator, all outside the analysed file set. |

## Design rules followed (not only checked)

- No dynamic memory.
- No recursion.
- No library calls apart from `<math.h>` (`sinf`, `cosf`, `log10f`, `fabsf`).
- Every state is in caller-owned structs.
- Single-precision float with explicit `f` suffixes (`-Wdouble-promotion` is on).
- Explicit casts at every narrowing.
- `if`/`else` chains end with `else`.
