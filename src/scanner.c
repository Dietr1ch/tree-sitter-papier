#include <assert.h>
#include <stdarg.h>
#include <stdint.h>

#include "tree_sitter/alloc.h"
#include "tree_sitter/array.h"
#include "tree_sitter/parser.h"

// External scanner for Papier
//
// This implements the block boundaries, which are required
// for the so called !!here_document named block boundaries.
//
// * References @ref {
//   - TS| External Scanners:
//       https://tree-sitter.github.io/tree-sitter/creating-parsers/4-external-scanners.html
//   - Writing a whitespace-sensitive grammar:
//       https://blog.viktomas.com/graph/whitespace-sensitive-treesitter-grammar/
//   - !!here_document: https://en.wikipedia.org/wiki/Here_document
// }

/// Tokens recognised by the Scanner
///
/// NOTE: These MUST be duplicated in `../grammar.js`s `externals: $ = > [],`
enum TokenType {
  _DOC_START, // Matches `^(#).*{$`
};

// Helpers
// =======
typedef Array(char) String;

/// Moves the cursor forward keeping the character in the current token
static inline void lx_advance(TSLexer *lexer) { lexer->advance(lexer, false); }
/// Moves the cursor forward, discarding the current chacter as whitespace
static inline void lx_skip(TSLexer *lexer) { lexer->advance(lexer, true); }
/// Marks the end of the token
static inline void lx_terminate(TSLexer *lexer) { lexer->mark_end(lexer); }
/// Gets the column number
static inline uint32_t lx_get_column(TSLexer *lexer) {
  return lexer->get_column(lexer);
}
static inline void lx_log(const TSLexer *lexer, const char *text, ...) {
  va_list args;
  va_start(args, text);
  lexer->log(lexer, text, args);
  va_end(args);
}

typedef struct {
} Scanner;

// Implementations
// ---------------

// Scanner
// Initialisation
static inline Scanner *create() {
  Scanner *const s = ts_malloc(sizeof(Scanner));
  return s;
}
static inline void destroy(Scanner *scanner) { ts_free(scanner); }

// Serialisation
static inline unsigned serialize(const Scanner *const scanner, //
                                 uint8_t *buffer) {
  const unsigned bytes_used = 0;
  assert(bytes_used <= TREE_SITTER_SERIALIZATION_BUFFER_SIZE);

  return bytes_used;
}
static inline void deserialize(Scanner *scanner,   //
                               const char *buffer, //
                               unsigned length) {}

// Scanning
#define TOKEN_LEXED true
#define NO_TOKEN_LEXED false
static inline bool scan(Scanner *scanner, //
                        TSLexer *lexer,   //
                        const bool *valid_symbols) {
  if (lexer->eof(lexer)) {
    // TODO: Figure out what to do
    return NO_TOKEN_LEXED;
  }

  if (lx_get_column(lexer) == 0) {
    lx_log(lexer, "At the beginning of the line");

    switch (lexer->lookahead) {
      // Found `^#`
    case '#': {
      lx_advance(lexer);
      lx_terminate(lexer); // _DOC_START would finish here

      if (lexer->eof(lexer)) {
        return NO_TOKEN_LEXED;
      }

      switch (lexer->lookahead) {
      case ' ': {
        // Found `^(#) `. This is a sub-document heading
        lx_skip(lexer);
        lexer->result_symbol = _DOC_START;
        return TOKEN_LEXED;
      }

      default: {
        // Found `^(#)[^ ]`
        lx_log(lexer, "Found a tag or something weird");
        return NO_TOKEN_LEXED; // Whoops, failed to find the heading start
      }
      } // ..switch(lexer->lookahead)
    } // ..switch:'#'

    default: { // Found `^[^#]`. Uninteresting.
      return NO_TOKEN_LEXED;
    } // ..switch:_

    } // ..switch(lexer->lookahead)
  } // ..if (lx_get_column(lexer) == 0)

  return NO_TOKEN_LEXED;

} // ..scan()

// tree-sitter interface
// =====================
void *tree_sitter_papier_external_scanner_create() { return create(); }
void tree_sitter_papier_external_scanner_destroy(void *payload) {
  if (payload) {
    Scanner *scanner = (Scanner *)payload;
    destroy(scanner);
  }
}
unsigned tree_sitter_papier_external_scanner_serialize(const void *payload,
                                                       uint8_t *buffer) {
  assert(payload != NULL);
  const Scanner *const scanner = (const Scanner *)payload;
  return serialize(scanner, buffer);
}
void tree_sitter_papier_external_scanner_deserialize(void *payload,
                                                     const char *buffer,
                                                     unsigned length) {
  assert(payload != NULL);
  Scanner *const scanner = (Scanner *)payload;
  deserialize(scanner, buffer, length);
}
bool tree_sitter_papier_external_scanner_scan(void *payload, TSLexer *lexer,
                                              const bool *valid_symbols) {
  assert(payload != NULL);
  Scanner *const scanner = (Scanner *)payload;
  return scan(scanner, lexer, valid_symbols);
}
