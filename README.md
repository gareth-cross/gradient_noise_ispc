Messing around with gradient noise using Intel ISPC.

### Building

```
git clone https://github.com/gareth-cross/gradient_noise_ispc.git
cd gradient_noise_ispc
git submodule update --init --recursive
mkdir build; cd build
cmake .. -DISPC_EXECUTABLE=<path to ISPC compiler>
cmake -build . --config Release
```
Only tested on Windows.