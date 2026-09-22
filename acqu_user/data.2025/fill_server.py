#!/usr/bin/env python3
"""Add file entries from Full-NoErrors-Files.csv to Server-CBTagg.dat."""

import argparse
import re
from pathlib import Path


HERE = Path(__file__).resolve().parent
NAME = re.compile(r"CBTagg_\d+\.dat(?:\.xz)?")
ENTRY = re.compile(r"^File-Name:\s+(CBTagg_\d+\.dat(?:\.xz)?)\s+\d+\s+\d+\s*$")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", nargs="?", type=Path, default=HERE / "empty-noerrors-files.csv")
    parser.add_argument("server", nargs="?", type=Path, default=HERE / "Server-CBTagg_EmptyTarget.dat")
    args = parser.parse_args()

    names = []
    for line_number, line in enumerate(args.csv.read_text().splitlines(), 1):
        name = line.strip()
        if not name:
            continue
        if not NAME.fullmatch(name):
            parser.error(f"{args.csv}:{line_number}: invalid file name: {name!r}")
        names.append(name.removesuffix(".xz") + ".xz")

    content = args.server.read_text()
    lines = content.splitlines(keepends=True)
    existing = set()
    for line in lines:
        match = ENTRY.fullmatch(line.rstrip("\r\n"))
        if match:
            existing.add(match.group(1))

    additions = []
    for name in names:
        if name not in existing:
            additions.append(f"File-Name:      {name:<27}0       0\n")
            existing.add(name)

    if additions:
        end = next((i for i, line in enumerate(lines) if line.strip() == "## END"), None)
        if end is None:
            parser.error(f"{args.server}: missing ## END marker")
        lines[end:end] = additions + ["\n"]
        args.server.write_text("".join(lines))
    print(f"Added {len(additions)} entries to {args.server}")


if __name__ == "__main__":
    main()
