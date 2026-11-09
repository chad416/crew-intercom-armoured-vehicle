# Requirement-to-test traceability

Every CB requirement in [../SRS.md](../SRS.md) maps to unit tests (`tests/unit/file.c:function`), Renode
integration tests (`tests/robot/icom.robot`, IT01–IT08) or other evidence. `tools/trace.py` fails if a row
is missing or names a test that does not exist. CI runs it.

| ID | Parent | Unit tests | Integration tests | Other evidence |
|---|---|---|---|---|
| CB-01 | SYS-3.2.1 | test_box.c:test_box | IT05 | model/studies/s1_chain.py (codec bandwidth) |
| CB-02 | SYS-3.2.4 | test_frame.c:test_roundtrip | IT01 | model/studies/s4_budget.py |
| CB-03 | SYS-3.2.4 | test_mix.c:test_mix | | |
| CB-04 | SYS-3.2.7 | test_box.c:test_box | IT02 | |
| CB-05 | SYS-3.2.4 | | IT05 | |
| CB-10 | SYS-3.2.7 | test_frame.c:test_crc, test_frame.c:test_roundtrip | IT01 | |
| CB-11 | SYS-3.2.7 | test_frame.c:test_roundtrip, test_frame.c:test_encode_limits | | |
| CB-12 | SYS-3.5.1 | test_frame.c:test_bitflips, test_frame.c:test_parser_errors | IT04 | |
| CB-13 | SYS-3.2.7 | test_box.c:test_box | | |
| CB-14 | SYS-3.2.7 | test_link.c:test_link | IT04 | |
| CB-15 | SYS-3.5.1 | test_link.c:test_link | IT04 | |
| CB-16 | SYS-3.2.7 | test_link.c:test_link | | |
| CB-20 | SYS-3.2.7 | test_modes.c:test_modes | IT01 | |
| CB-21 | SYS-3.2.7 | test_modes.c:test_modes | | |
| CB-22 | SYS-3.2.7 | test_modes.c:test_modes, test_box.c:test_box | IT02 | |
| CB-23 | SYS-3.2.7 | test_modes.c:test_modes, test_box.c:test_box | IT03 | |
| CB-24 | SYS-3.2.7 | test_modes.c:test_modes, test_box.c:test_box | IT07 | |
| CB-25 | SYS-3.2.1 | test_modes.c:test_modes, test_box.c:test_box | IT06 | |
| CB-27 | SYS-3.2.7 | test_modes.c:test_modes | IT01, IT02, IT08 | |
| CB-28 | SYS-3.2.7 | test_modes.c:test_modes, test_box.c:test_box | IT06 | |
| CB-30 | SYS-3.3.1 | test_mix.c:test_mix | | |
| CB-31 | SYS-3.2.1 | test_mix.c:test_mix | | |
| CB-32 | SYS-3.2.1 | test_mix.c:test_mix | | |
| CB-33 | SYS-3.3.2 | test_mix.c:test_mix | | |
| CB-34 | SYS-3.2.3 | test_mix.c:test_mix | | |
| CB-35 | SYS-3.2.3 | test_mix.c:test_mix | | |
| CB-40 | SYS-3.2.2 | test_anr.c:test_anr | IT05 | model/studies/s2_anr.py |
| CB-41 | SYS-3.2.2 | test_anr.c:test_anr, test_box.c:test_box | IT05 | |
| CB-42 | SYS-3.2.2 | test_anr.c:test_anr | | model/studies/s2_anr.py (stability sweep) |
| CB-43 | SYS-3.2.3 | test_anr.c:test_anr | IT06 | |
| CB-44 | SYS-3.2.2 | test_anr.c:test_anr | | |
| CB-45 | SYS-3.2.2 | test_anr.c:test_anr | | |
| CB-46 | SYS-3.2.3 | test_anr.c:test_anr, test_box.c:test_box | IT06 | |
| CB-47 | SYS-3.2.2 | test_anr.c:test_anr, test_box.c:test_box | IT05 | |
