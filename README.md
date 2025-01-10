# treeviz

Parse indented text input and generate a visually appealing tree diagram.

## Installation

Clone this repository and run:

```bash
gcc -o treeviz treeviz.c
```

This command will produce an executable named `treeviz`.

## Usage

`treeviz` reads indented text from standard input and outputs a tree diagram to standard output. For example, to visualize a file named `input.txt`:

```bash
./treeviz < input.txt
```

The program will process the indented text in `input.txt` and print a corresponding tree diagram.

## Example

Given an `input.txt` with the following content:

```
Edit me to generate
  a
    nice
      tree
        diagram!
        :)
  Use indentation
    to indicate
      file
      and
        folder
        nesting
    do cool
        stuff
```

Running `./treeviz < input.txt` will output:

```
.
└─ Edit me to generate
   ├─ a
   │  └─ nice
   │     └─ tree
   │        ├─ diagram!
   │        └─ :)
   └─ Use indentation
      ├─ to indicate
      │  ├─ file
      │  └─ and
      │     ├─ folder
      │     └─ nesting
      └─ do cool
         └─ stuff
```
