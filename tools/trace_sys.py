#!/usr/bin/env python3
"""System-level traceability check: every SYS requirement of DOSSIER.md §5 has
a row in docs/sys_traceability.md with a method, evidence that exists, and a
status. Files must exist; Renode tests (ITnn) and unit-test files named in the
evidence must exist.
"""

from __future__ import annotations

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main():
    dossier = open(os.path.join(ROOT, "DOSSIER.md"), encoding="utf-8").read()
    ids = sorted(set(re.findall(r"^\| (SYS-\d\.\d\.\d) \|", dossier, re.M)))
    rows = {}
    for line in open(os.path.join(ROOT, "docs", "sys_traceability.md"), encoding="utf-8"):
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if len(cells) == 6 and re.fullmatch(r"SYS-\d\.\d\.\d", cells[0]):
            rows[cells[0]] = cells
    robot = open(os.path.join(ROOT, "fw", "tests", "robot", "icom.robot"), encoding="utf-8").read()
    its = set(re.findall(r"^(IT\d\d)\b", robot, re.M))
    errors = []
    for rid in ids:
        if rid not in rows:
            errors.append("%s: in DOSSIER.md §5 but not in docs/sys_traceability.md" % rid)
            continue
        _, _, method, evidence, result, status = rows[rid]
        if not re.fullmatch(r"[ATID](, [ATID])*", method):
            errors.append("%s: method '%s' is not A/T/I/D" % (rid, method))
        if not (evidence and result and status):
            errors.append("%s: evidence, result or status missing" % rid)
        for path in re.findall(r"[\w./-]+\.(?:json|md|py|c|robot)", evidence):
            if not os.path.exists(os.path.join(ROOT, path)):
                errors.append("%s: %s does not exist" % (rid, path))
        for it in re.findall(r"\b(IT\d\d)\b", result):
            if it not in its:
                errors.append("%s: %s is not a Renode test" % (rid, it))
    for rid in rows:
        if rid not in ids:
            errors.append("%s: in docs/sys_traceability.md but not in DOSSIER.md" % rid)
    for e in errors:
        print("TRACE ERROR " + e)
    print("%d system requirements, %d with evidence, %d errors" % (len(ids), len(rows), len(errors)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
