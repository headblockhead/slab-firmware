# interchange-firmware
Version 4

## Tasks

### Build

Directory: build

```bash
cmake ..
cmake --build . -j $(nproc)
```

### Clean

```bash
rm -rf build
mkdir build
```

### Upload

Requires: build
Directory: build

```bash
cp ./prototype/prototype.uf2 $(findmnt -S LABEL=RPI-RP2 -o TARGET -fn)
```
