# uvgPlyXporter

uvgPlyXporter is a C++17 library and command-line tool for exporting point cloud
frames to binary PLY files. It is part of the UVG V-PCC pipeline and is intended
to capture the geometry and color streams produced during real-time processing
and persist them to disk for later inspection, analysis, or playback.

The tool listens on two ZeroMQ PULL sockets, one for point positions and one for
per-point colors. Matching frames from both streams are paired, converted into a
tinyply point cloud, and written to disk as `binary-NNNN.ply`. Frames are
exported on a worker thread pool so that writing does not block message
reception.

## Features

- Receives position and color frames over ZeroMQ PULL sockets.
- Pairs frames from the two streams by arrival order.
- Exports binary PLY files using [tinyply](https://github.com/ddiakopoulos/tinyply).
- Stores positions as 32-bit floats and colors as 8-bit RGB.
- Offloads file writing to a threaded job queue.
- Supports shared and static library builds.

## Requirements

- CMake 3.27 or newer.
- A C++17 compiler.
- [vcpkg](https://github.com/microsoft/vcpkg) for dependency management.

Dependencies are declared in `vcpkg.json`:

- `cppzmq` (ZeroMQ C++ bindings)
- `tinyply`

## Building

The project uses CMake presets with vcpkg. Adjust the toolchain path in
`CMakePresets.json` to match your local vcpkg installation, then configure and
build:

```
cmake --preset default
cmake --build build --config Release
```

The output is placed in `build/bin` by default. Set `BUILD_SHARED_LIBS=OFF` to
build a static library instead of a shared one.

## Usage

Run the command-line tool by specifying the two input addresses and an output
directory:

```
plyXporter --addr_color tcp://*:5555 --addr_position tcp://*:5556 --save_dir ./output
```

Arguments:

| Argument          | Description                                            |
| ----------------- | ------------------------------------------------------ |
| `--addr_color`    | ZeroMQ endpoint to bind for the color stream.          |
| `--addr_position` | ZeroMQ endpoint to bind for the position stream.       |
| `--save_dir`      | Directory where exported PLY files are written.        |

Each received frame pair is written as `binary-0001.ply`, `binary-0002.ply`, and
so on, in zero-padded order.

## Library API

The export loop can also be embedded directly. Include the public header and
call the API:

```cpp
#include "plyxporter/plyxporter.hpp"

plyxporter::API::run("tcp://*:5555", "tcp://*:5556", "./output");
```

The position payload is expected to contain packed `float` triplets (`x`, `y`,
`z`) and the color payload packed `uint8_t` RGB triplets. Both messages are
interpreted with a point count derived from the position payload size. Sending
the string `DISCONNECT` on either socket stops the receiver.

## Project layout

```
src/lib                 Core library (API, communication, utils)
src/lib/communication   ZeroMQ portal and PLY export logic
src/lib/utils           Logging, timing, and thread queue utilities
src/app                 Command-line application
```

## License

See the repository for license information.
