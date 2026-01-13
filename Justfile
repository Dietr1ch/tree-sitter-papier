default: test


build:
	tree-sitter generate --log --json

test: build
	tree-sitter test

report:
	tree-sitter generate --report-states-for-rule -

fmt:
	nixfmt --check $(find . -name '*.nix')
	prettier --check --write $(find . -name '*.json')
	js-beautify --editorconfig --replace $(find . -name '*.js')

parse FILE:
	tree-sitter parse --rebuild --cst {{FILE}}
