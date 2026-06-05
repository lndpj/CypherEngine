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

    platform_files = {
        str((src_dir / "CypherEngine/CypherSystem/CypherSystem_PlatformWin32.cpp").resolve()),
        str((src_dir / "CypherEngine/CypherSystem/CypherSystem_PlatformLinux.cpp").resolve()),
    }

    mingw_cxx = (
        shutil.which("x86_64-w64-mingw32-g++")
        or "/opt/homebrew/bin/x86_64-w64-mingw32-g++"
    )
    llvm_cxx = (
        "/opt/homebrew/opt/llvm/bin/clang++"
        if Path("/opt/homebrew/opt/llvm/bin/clang++").exists()
        else (shutil.which("clang++") or "clang++")
    )

    entries = []
    for entry in native_entries:
        if str(Path(entry["file"]).resolve()) in platform_files:
            continue

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

    project_include = f"-I{src_dir}"

    win32_file = str((src_dir / "CypherEngine/CypherSystem/CypherSystem_PlatformWin32.cpp").resolve())
    linux_file = str((src_dir / "CypherEngine/CypherSystem/CypherSystem_PlatformLinux.cpp").resolve())

    entries.append({
        "directory": str(root_dir),
        "file": win32_file,
        "output": "clangd/sys_platform_win32.cpp.o",
        "arguments": [
            mingw_cxx,
            "-std=c++20",
            project_include,
            "-D_WIN32=1",
            "-DWIN32=1",
            "-DWIN64=1",
            "-D_WINDOWS=1",
            "-DUNICODE=1",
            "-D_UNICODE=1",
            "-DWIN32_LEAN_AND_MEAN=1",
            "-c",
            win32_file,
        ],
    })

    entries.append({
        "directory": str(root_dir),
        "file": linux_file,
        "output": "clangd/sys_platform_linux.cpp.o",
        "arguments": [
            llvm_cxx,
            "-xc++",
            "-std=c++20",
            project_include,
            "-D__linux__=1",
            "-D__unix__=1",
            "-Dlinux=1",
            "-Dunix=1",
            "-c",
            linux_file,
        ],
    })

    clangd_db_dir.mkdir(parents=True, exist_ok=True)
    clangd_db_path.write_text(json.dumps(entries, indent=2) + "\n")

    print(f"wrote {clangd_db_path}")
    print(f"entries: {len(entries)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
