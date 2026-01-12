default: test


build:
	tree-sitter generate

test: build
	tree-sitter test

report:
	tree-sitter generate --report-states-for-rule -

fmt:
	nixfmt --check $(find . -name '*.nix')
	prettier --check --write $(find . -name '*.json')

parse FILE:
	tree-sitter parse --rebuild --cst {{FILE}}
