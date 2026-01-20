### AGENTS.md

#### Relevant files

Information:
- README: `./README.org`

Code:
- tree-sitter grammar: `./grammar.js`
- tree-sitter scanner: `./src/scanner.c`
  - This is used to read-ahead and disambiguate context-sensitive characters.

Testing:
- Sample Papier documents: `./examples/*.papier`
- tree-sitter test corpus: `./test/corpus/*.txt`

#### Build/Lint/Test/Parse Commands

To format the code:

```sh
just fmt
```

To build the project:

```sh
just build
```

To test the tree-sitter corpus:

```sh
just test
```


To parse a standalone file:

```sh
just parse <FILENAME>
```

