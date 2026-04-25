# Orange

A Programming Language made for the citrus vm.

## Build

To build the following program, run the following:

```
make
```

This should create the binary in the following path `./build`

## Usage

The following is the usage for the compiler:

```
USAGE: orange [OPTION] <filepath>

ABOUT:
    orange is a compiler for a statically typed
    programming language for citrus VM

OPTION:
    -h, --help     This screen
    --only-tokens  Print only the tokens

HINTS:
    1. If you want to read from stdin, then make filepath == '-'
       echo hello | orange -; this should read hello from the stdin
```

## Grammar

Check out the following for the [grammar](./grammar)

