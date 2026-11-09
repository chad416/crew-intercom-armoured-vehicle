#!/usr/bin/env python3
"""Regenerate every study figure (docs/img) and result file (results/).

    python model/run_all.py [s1 s2 ...]

S1 speech chain, S2 ANR, S3 STI validation (slow: about 5 min), S4 latency,
S5 AoA, S6 RMA, S7 28 V input stage.
"""

import importlib
import os
import sys
import time

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "studies"))
sys.path.insert(0, HERE)

STUDIES = ["s1_chain", "s2_anr", "s3_stipa", "s4_budget", "s5_aoa", "s6_rma", "s7_power"]

if __name__ == "__main__":
    pick = sys.argv[1:]
    for name in STUDIES:
        if pick and name.split("_")[0] not in pick:
            continue
        t0 = time.time()
        print("== %s" % name, flush=True)
        importlib.import_module(name).main()
        print("   %.0f s" % (time.time() - t0), flush=True)
