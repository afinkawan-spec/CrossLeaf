# Building CrossLeaf 1.0.0

## Tested build

- Target: Xteink X4 (`slim` PlatformIO environment)
- Source base: CrossPoint Reader 1.6.5, with the CrossLeaf 1.0.0 changes
- Build tool: PlatformIO Core and the versions declared by `platformio.ini`
- Firmware artifact: `CrossLeaf-1-0.0-fix.bin` (published in the GitHub Release)

The source snapshot includes the FreeInk SDK used for this build under
`freeink-sdk/`. It is vendored intentionally so the build does not depend on a
moving SDK branch or missing submodule metadata.

## Windows

Use a short checkout path to avoid Windows command-line length failures in the
ESP-IDF/SCons toolchain. For example, clone or extract this repository to
`C:\\CrossLeaf` and run from that directory:

```powershell
pio run -e slim -j 1
```

The output is normally written to:

```text
.pio/build/slim/firmware.bin
```

For the reproducible release binary, use the `.bin` attached to the matching
GitHub Release. A locally rebuilt image can differ because the PlatformIO
platform and package URLs in `platformio.ini` may point to upstream-hosted
artifacts.

## Notes

- Do not commit `.pio/`, generated SDK configuration files, local PlatformIO
  overrides, or firmware `.bin` files. These are covered by `.gitignore`.
- The release binary is intentionally kept out of all source commits and tags;
  it is attached to the GitHub Release instead.
- The embedded product version remains `1.0.0`; Git commit metadata may be
  appended by development builds.
