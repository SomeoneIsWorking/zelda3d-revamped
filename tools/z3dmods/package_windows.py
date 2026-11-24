"""The z3d package for Windows (x86-64), cross-built here with llvm-mingw from the shared
target-platform checkout, and the zip of it a release carries.

    z3d.exe             the launcher (app/launcher): python\\pythonw.exe -I -m z3dmods.packed play
    bin/z3d.exe         the app, with the core, SDL3, SDL3_ttf, SDL3_image, FreeType and the C++
                        runtime linked in, so it imports only DLLs Windows ships
    python/             uv's relocatable CPython for Windows, with the locked wheels for Windows

Vulkan is the system's vulkan-1.dll, which every Vulkan driver installs. The tools' import check
runs the package's own interpreter under Wine.
"""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

from z3dmods import app, package, sources
from z3dmods.config import Config

ZIP = "z3d-windows-x86_64.zip"
# The target-platform libraries the app links (shared/target-platform's libraries.json).
LIBRARIES = ("SDL3", "SDL3_image", "freetype", "SDL3_ttf")
# What a cross find_package searches: the core's prefix joins the libraries'.
JOINED = ("CMAKE_PREFIX_PATH", "CMAKE_FIND_ROOT_PATH")


def _platform(cfg: Config, *words: str) -> str:
    """target-platform's output for `words`, with its dependency root under ours."""
    checkout = sources.folder(cfg, "target-platform")
    return subprocess.run(["uv", "run", "--directory", str(checkout), "--frozen", "python", "-m", "target_platform",
                           *words, "--root", str(cfg.dependencies / "target-platform")],
                          check=True, capture_output=True, text=True).stdout


def _flags(cfg: Config) -> list[str]:
    """The toolchain and the libraries' prefix, with the core's prefix in both search lists."""
    library_words = [word for name in LIBRARIES for word in ("--library", name)]
    definitions = _platform(cfg, "provision", "windows", *library_words).split()
    core = app.core_prefix(cfg, BUILDER)
    joined = []
    for definition in definitions:
        key, _, value = definition.removeprefix("-D").partition("=")
        joined.append(f"-D{key}={core};{value}" if key in JOINED else definition)
    return [*joined, *sources.package_flags(cfg)]


def _run_here(_cfg: Config, command: list[str]) -> None:
    subprocess.run(command, check=True)


BUILDER = app.Builder("llvm-mingw", lambda cfg: cfg.repo / "build" / "windows-app", _run_here, _flags,
                      compilers=(), binary="z3d.exe")


def _licenses(cfg: Config, licenses: Path) -> None:
    """The licences the libraries linked into the app installed into their prefix."""
    prefix = Path(next(d for d in _flags(cfg) if d.startswith("-DCMAKE_FIND_ROOT_PATH=")).split(";")[-1])
    for folder in sorted((prefix / "share" / "licenses").iterdir()):
        shutil.copytree(folder, licenses / folder.name)


def folder(cfg: Config) -> Path:
    return cfg.repo / "build" / "package" / "windows" / "z3d"


def build(cfg: Config, out: Path) -> Path:
    """Builds the package folder `out` from this checkout, replacing an earlier one, and the zip beside it."""
    if out.exists():
        shutil.rmtree(out)
    (out / "bin").mkdir(parents=True)
    built = app.current_binary(cfg, BUILDER)
    shutil.copy2(built, out / "bin" / built.name)
    shutil.copy2(built.with_name("z3d_launcher.exe"), out / "z3d.exe")
    print(_platform(cfg, "audit-windows", str(out / "z3d.exe"), str(out / "bin" / built.name)), end="")
    interpreter = package.contents(cfg, out, package.WINDOWS_PYTHON)
    _licenses(cfg, out / "licenses")
    package.finish(cfg, out, interpreter, package.WINDOWS_PYTHON)
    archive = out.with_name(ZIP)
    archive.unlink(missing_ok=True)
    shutil.make_archive(str(archive.with_suffix("")), "zip", out.parent, out.name)
    return archive
