# Carmen

**Carmen** compiler.

## Requirements

- `Linux` system.
- `c99` compiler (e.g. `gcc`).
- `x86_64` architecture.

## Build Compile & Run

### Build

Compile the `carmen` compiler:
```bash
gcc -std=c99 -D_POSIX_C_SOURCE=199309L -o ./carmen ./main.c ./src/*
```

### Compile

Use `carmen` to compile source code to assembly:
```bash
./carmen ./code.carmen ./code.s
```

Then assemble and link the output:
```bash
gcc -O0 -g -m64 -no-pie -o ./bin ./code.s
```

### Run

Run the compiled binary:
```bash
./bin; echo "$?"
```

