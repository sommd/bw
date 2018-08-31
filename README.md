# bw [![License: GPL v3](https://img.shields.io/badge/License-GPL%20v3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

`bw` is a Unix filter for performing bitwise operations on files and streams. It supports various bitwise operations (such as `&`, `|`, `^`, `<<` and `>>`), operating on either two files/streams or with a single file/stream and a single-byte operand.

## Usage

```
Usage: bw [OPTION...] OPERATOR [OPERAND]
Perform bitwise operations on files and streams.

OPERATOR is one of: |, o[r], &, a[nd], ^, x[or], ~, n[ot], <[<], l[shift],
>[>], r[shift]. OPERAND is a file or byte value.

  -e, --eof-mode=EOF_MODE    How to handle the operand file being shorter than
                             input. One of: e[rror] (default), t[runcate],
                             l[oop], z[ero], o[ne]
  -i, --input=FILE           File to read input from, or '-' to use stdin
                             (default)
  -o, --output=FILE          File to write output to, or '-' to use stdout
                             (default)
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.
```

### Operators

Operator | Operand | Description
--- | --- | ---
`\|`, `o`, `or` | file or byte | Bitwise OR each byte of input with the operand.
`&`, `a`, `and` | file or byte | Bitwise AND each byte of input with the operand.
`^`, `x`, `xor` | file or byte | Bitwise XOR each byte of input with the operand.
`~`, `n`, `not` | none | Bitwise NOT (invert) each byte of input.
`<`, `<<`, `l`, `lshift` | positive integer | Bitwise (logical) shift entire input left by OPERAND bits. Bits will be carried to the previous byte and zero-bits will be shifted in at the end.
`>`, `>>`, `r`, `rshift` | positive integer | Bitwise (logical) shift entire input right by OPERAND bits. Bits will be carried to the next byte and zero-bits will be shifted in at the start.

### Operands

Integer and byte operands can be in any format supported by the `%i` specifier (i.e. decimal, octal preceded by `0`, or hex preceded by `0x`). Additionally, byte operands can be binary preceded by `0b` or octal preceded by `0o`.

Any operands which cannot be parsed as integers or bytes will considered files. To pass a file name that matches an integer or byte, use a relative path, e.g. `./123`.

### EOF Modes

When using a file operand, various methods can be used to handle the case where the operand file is shorter than the input file.

EOF Mode | Description
--- | ---
`e`, `error` | Output an error message and exit unsuccessfully.
`t`, `truncate` | Truncate input and output only as many bytes as are in the operand file.
`l`, `loop` | Seek back to the start of the operand file and continue. Operand file must be seekable.
`z`, `zero` | Stop reading from the operand file and use zero-bits.
`o`, `one` | Stop reading from the operand file and use one-bits.

## Examples

Emulate a "`/dev/one`", similar to `/dev/zero` but returns one-bits instead of zero-bits.

```sh
$ bw -i /dev/zero '~' | xxd | head -1
00000000: ffff ffff ffff ffff ffff ffff ffff ffff  ................
```

Basix XOR cipher.

```sh
$ bw ^ 0b11110011 > super_secret.txt
Hello, World.
$ cat super_secret.txt
������Ӥ������
$ bw -i super_secret.txt ^ 0b11110011
Hello, World.
```

## Building

Run `make` to build bw and run the unit tests. Should build on any Unix/Unix like environment with GNU glibc (since the default front-end depends on Argp).

## Contributing

1. Fork
2. Make some changes and commit.
4. Create a pull request.

## License

```
Copyright (C) 2018 David Sommerich

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

See [LICENSE](/LICENSE).
