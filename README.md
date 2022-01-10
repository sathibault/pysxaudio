# pySxaudio Python wrapper

## Build instructions

Clone the `pybind11` library as a subdirectory in this project:
```bash
git clone https://github.com/pybind/pybind11.git
```

Create build directory and generate Visual Studio Projects:
```bash
mkdir build
cd build
"\Program Files (x86)\Microsoft Visual Studio 14.0\vc\vcvarsall.bat" x64
cmake ..
```

Open the `pySxaudio.sln` solution, change the configuration to
`Release - x64` and build the solution.  This will create the
`Release\pySxaudio.cp37-win_amd64.pyd` python module.
