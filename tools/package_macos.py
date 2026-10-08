#!/usr/bin/env python3
"""Build an Apple Silicon .app with all non-system runtime libraries included."""
from pathlib import Path
import os
import plistlib
import shutil
import subprocess
import zipfile
import struct

root = Path(__file__).resolve().parents[1]
build = root / "build/macos"
app = root / "dist/OpenGW.app"

def run(*args):
    return subprocess.check_output(args, text=True).strip()

run("cmake", "-S", str(root), "-B", str(build), "-DCMAKE_BUILD_TYPE=Release",
    "-DCMAKE_OSX_ARCHITECTURES=arm64")
run("cmake", "--build", str(build), "-j4")
if app.exists():
    shutil.rmtree(app)
contents = app / "Contents"
macos, resources, frameworks = (contents / n for n in ("MacOS", "Resources", "Frameworks"))
for folder in (macos, resources, frameworks):
    folder.mkdir(parents=True)
binary = macos / "opengw"
shutil.copy2(build / "opengw", binary)
for folder in ("images", "sounds"):
    shutil.copytree(root / folder, resources / folder)
# Bundle the SDL2 compatibility library and the SDL3 library it loads at runtime.
dependencies = run("otool", "-L", str(binary)).splitlines()[1:]
sdl = next(Path(line.strip().split(" (")[0]) for line in dependencies if "libSDL2" in line)
shutil.copy2(sdl, frameworks / sdl.name)
brew_prefix = sdl.parents[3]
sdl3 = brew_prefix / "opt/sdl3/lib/libSDL3.dylib"
shutil.copy2(sdl3, frameworks / "libSDL3.dylib")
run("install_name_tool", "-change", str(sdl), "@executable_path/../Frameworks/" + sdl.name, str(binary))
for library in frameworks.iterdir():
    run("install_name_tool", "-id", "@rpath/" + library.name, str(library))
    # Ensure nothing depends on the development machine's Homebrew installation.
    linked = run("otool", "-L", str(library)).splitlines()[2:]
    assert not any("/opt/homebrew" in line or "/usr/local" in line for line in linked), linked
licenses = resources / "Licenses"
licenses.mkdir()
for prefix, name in ((sdl.parents[1], "SDL2-compat"), (sdl3.parents[1], "SDL3")):
    shutil.copy2(prefix / "LICENSE.txt", licenses / (name + "-LICENSE.txt"))
shutil.copy2(root / "README-macOS.md", resources / "README-macOS.md")
for document in ("COPYING", "LICENSE.md"):
    shutil.copy2(root / document, resources / document)

# Reuse the game's existing icon for Finder and the Dock.
iconset = build / "OpenGW.iconset"
iconset.mkdir(exist_ok=True)
for size in (16, 32, 128, 256, 512):
    for scale in (1, 2):
        output = iconset / (f"icon_{size}x{size}" + ("@2x" if scale == 2 else "") + ".png")
        run("sips", "-z", str(size * scale), str(size * scale), str(root / "images/icon.png"), "--out", str(output))
# ICNS supports PNG chunks directly; avoid iconutil's conversion backend.
chunks = []
for tag, filename in ((b"icp4", "icon_16x16.png"), (b"icp5", "icon_32x32.png"),
    (b"ic07", "icon_128x128.png"), (b"ic08", "icon_256x256.png"),
    (b"ic09", "icon_512x512.png"), (b"ic10", "icon_512x512@2x.png")):
    data = (iconset / filename).read_bytes()
    chunks.append(tag + struct.pack(">I", len(data) + 8) + data)
payload = b"".join(chunks)
(resources / "OpenGW.icns").write_bytes(b"icns" + struct.pack(">I", len(payload) + 8) + payload)
with (contents / "Info.plist").open("wb") as f:
    plistlib.dump({
        "CFBundleName": "OpenGW", "CFBundleDisplayName": "OpenGW",
        "CFBundleIdentifier": "net.opengw.apple-silicon", "CFBundleExecutable": "opengw",
        "CFBundlePackageType": "APPL", "CFBundleIconFile": "OpenGW.icns",
        "CFBundleShortVersionString": "1.0", "CFBundleVersion": "42.5",
        "LSMinimumSystemVersion": "26.0", "NSHighResolutionCapable": True,
        "SDL_FILESYSTEM_BASE_DIR": "resource",
    }, f)
for library in frameworks.iterdir():
    run("codesign", "--force", "--sign", "-", str(library))
run("codesign", "--force", "--sign", "-", str(binary))
run("codesign", "--force", "--sign", "-", str(app))
run("codesign", "--verify", "--deep", "--strict", str(app))
assert run("lipo", "-archs", str(binary)) == "arm64"
assert "/opt/homebrew" not in run("otool", "-L", str(binary))
archive = root / "dist/OpenGW-macOS-AppleSilicon.zip"
if archive.exists():
    archive.unlink()
run("ditto", "-c", "-k", "--sequesterRsrc", "--keepParent", str(app), str(archive))
with zipfile.ZipFile(archive) as z:
    assert z.testzip() is None
print(f"Verified Apple Silicon app: {app}")
print(f"Verified ZIP: {archive} ({archive.stat().st_size / 1024 / 1024:.1f} MiB)")
