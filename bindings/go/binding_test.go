package tree_sitter_papier_test

import (
	"testing"

	tree_sitter "github.com/tree-sitter/go-tree-sitter"
	tree_sitter_papier "papier-lang.github.io/papier/bindings/go"
)

func TestCanLoadGrammar(t *testing.T) {
	language := tree_sitter.NewLanguage(tree_sitter_papier.Language())
	if language == nil {
		t.Errorf("Error loading Papier grammar")
	}
}
