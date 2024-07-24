# Control Panel

WIP for OpenEuler. By iyenli.

## Build

Install dependencies:

```bash
cd deps
./build-all
sudo ./build-all install
```

Build Control Panel:

```bash
$ mkdir -p build && cd build

$ cmake -DCMAKE_BUILD_TYPE=Debug ..
$ cmake --build . --parallel

$ ./controlpanel
```

## Test

We use gtsam for testing. You can install it by:

```bash
ctest
```
## How to add a configuration

