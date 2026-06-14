#!/usr/bin/env python3

import json
import shutil
import shlex
from pathlib import Path


def main() -> int:
    root_dir = Path(__file__).resolve().parents[2]
    src_dir = root_dir / "src"
    native_db_path = root_dir / "build" / "compile_commands.json"
    clangd_db_dir = root_dir / "build-clangd"
    clangd_db_path = clangd_db_dir / "compile_commands.json"

    if not native_db_path.exists():
        raise SystemExit(f"missing native compile database: {native_db_path}")

    native_entries = json.loads(native_db_path.read_text())

    llvm_cxx = (
        "/opt/homebrew/opt/llvm/bin/clang++"
        if Path("/opt/homebrew/opt/llvm/bin/clang++").exists()
        else (shutil.which("clang++") or "clang++")
    )

    entries = []
    for entry in native_entries:
        fixed_entry = dict(entry)
        if "arguments" in fixed_entry:
            arguments = list(fixed_entry["arguments"])
        else:
            arguments = shlex.split(fixed_entry["command"])

        if arguments:
            arguments[0] = llvm_cxx

        fixed_entry.pop("command", None)
        fixed_entry["arguments"] = arguments
        entries.append(fixed_entry)

    clangd_db_dir.mkdir(parents=True, exist_ok=True)
    clangd_db_path.write_text(json.dumps(entries, indent=2) + "\n")

    print(f"wrote {clangd_db_path}")
    print(f"entries: {len(entries)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
