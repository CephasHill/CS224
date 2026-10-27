# CS 224: Computer Systems

Coursework and experiments from **BYU CS 224**, focused on the low-level
foundations of computer systems. The repository contains C programs, Y86-64
assembly exercises, and notes from the labs.

## Repository contents

| Path | Description |
| --- | --- |
| [`myxxd.c`](./myxxd.c) | A small `xxd`-style stdin dumper with hexadecimal and binary output modes |
| [`lab1/`](./lab1/) | Introductory C and Linux command-line exercises |
| [`lab8/`](./lab8/) | Y86-64 assembly exercises |
| [`project2/`](./project2/) | A BMP image filter supporting grayscale and threshold output |
| [`project3/`](./project3/) | Y86-64 instruction pipeline implementation |

The root-level text files are original lab notes or test input used while
working through the exercises.

## Building

The programs use a standard C compiler. From the repository root:

```sh
cc -std=c11 -Wall -Wextra -pedantic myxxd.c -o myxxd
cc -std=c11 -Wall -Wextra -pedantic lab1/src.c -o lab1/src
cc -std=c11 -Wall -Wextra -pedantic project2/bmpFilter.c -o project2/bmpFilter
```

Run the `myxxd` utility with standard input:

```sh
printf 'Hello, systems!\n' | ./myxxd
printf 'Hello, systems!\n' | ./myxxd -b
```

`project3/y86-64.c` is one component of the Y86-64 simulator and expects the
course-provided simulator support files (including `utils.h`) to be available
when compiling the complete project.

## Notes

- The source is preserved as coursework and is not presented as a production
  systems library.
- Compiled binaries, editor files, and locally downloaded course materials are
  ignored by Git. Existing local files remain available for study but are not
  required to build the source in this repository.
- Follow your course's collaboration and submission policies when reusing or
  extending these solutions.

## License

No separate open-source license is currently applied. The repository is
intended as a personal course archive.
