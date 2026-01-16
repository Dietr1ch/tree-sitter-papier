#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <wctype.h>

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
/// NOTE: These MUST be duplicated in ../grammar.js's `externals: $ = > [],`
/// TODO: Synchronise external token list
enum TokenType {
  /// `{`, `!EoF{`
  BLOCK_START,

  /// Block content
  BLOCK_CONTENT,

  /// `}`, `}EoF!`
  BLOCK_END,
};

// Helpers
// =======
typedef Array(char) String;

/// Moves the cursor forward keeping the character in the current token
static inline void advance(TSLexer *lexer) { lexer->advance(lexer, false); }
/// Moves the cursor forward, discarding the current chacter as whitespace
static inline void skip(TSLexer *lexer) { lexer->advance(lexer, true); }

/**
 * Consume a "word" in POSIX parlance, and returns it unquoted.
 *
 * This is an approximate implementation that doesn't deal with any
 * POSIX-mandated substitution, and assumes the default value for
 * IFS.
 */
static bool advance_word(TSLexer *lexer, String *unquoted_word) {
  bool empty = true;

  int32_t quote = 0;
  if (lexer->lookahead == '\'' || lexer->lookahead == '"') {
    quote = lexer->lookahead;
    advance(lexer);
  }

  while (lexer->lookahead &&
         !(quote ? lexer->lookahead == quote || lexer->lookahead == '\r' ||
                       lexer->lookahead == '\n'
                 : iswspace(lexer->lookahead))) {
    if (lexer->lookahead == '\\') {
      advance(lexer);
      if (!lexer->lookahead) {
        return false;
      }
    }
    empty = false;
    array_push(unquoted_word, lexer->lookahead);
    advance(lexer);
  }
  array_push(unquoted_word, '\0');

  if (quote && lexer->lookahead == quote) {
    advance(lexer);
  }

  return !empty;
}

static inline void reset_string(String *string) {
  if (string->size > 0) {
    memset(string->contents, 0, string->size);
    array_clear(string);
  }
}

// Types
// =====
typedef struct {
  bool is_raw;
  bool started;
  bool allows_indent;
  String delimiter;
  String current_leading_word;
} Heredoc;

typedef struct {
  uint8_t level;
  uint8_t pending_blocks;

  Array(Heredoc) heredocs;
} Scanner;

// Implementations
// ---------------

// Heredoc

#define heredoc_new()                                                          \
  {                                                                            \
      .is_raw = false,                                                         \
      .started = false,                                                        \
      .allows_indent = false,                                                  \
      .delimiter = array_new(),                                                \
      .current_leading_word = array_new(),                                     \
  };

static inline void reset_heredoc(Heredoc *heredoc) {
  heredoc->is_raw = false;
  heredoc->started = false;
  heredoc->allows_indent = false;
  reset_string(&heredoc->delimiter);
}

static bool scan_heredoc_start(Heredoc *heredoc, TSLexer *lexer) {
  while (iswspace(lexer->lookahead)) {
    skip(lexer);
  }

  lexer->result_symbol = BLOCK_START;
  heredoc->is_raw = lexer->lookahead == '\'' || lexer->lookahead == '"' ||
                    lexer->lookahead == '\\';

  bool found_delimiter = advance_word(lexer, &heredoc->delimiter);
  if (!found_delimiter) {
    reset_string(&heredoc->delimiter);
    return false;
  }
  return found_delimiter;
}

static bool scan_heredoc_end_identifier(Heredoc *heredoc, TSLexer *lexer) {
  reset_string(&heredoc->current_leading_word);
  // Scan the first 'n' characters on this line, to see if they match the
  // heredoc delimiter
  int32_t size = 0;
  if (heredoc->delimiter.size > 0) {
    while (lexer->lookahead != '\0' && lexer->lookahead != '\n' &&
           (int32_t)*array_get(&heredoc->delimiter, size) == lexer->lookahead &&
           heredoc->current_leading_word.size < heredoc->delimiter.size) {
      array_push(&heredoc->current_leading_word, lexer->lookahead);
      advance(lexer);
      size++;
    }
  }
  array_push(&heredoc->current_leading_word, '\0');
  return heredoc->delimiter.size == 0
             ? false
             : strcmp(heredoc->current_leading_word.contents,
                      heredoc->delimiter.contents) == 0;
}

// Scanner

// Initialisation
static inline Scanner *create() {
  Scanner *const s = ts_malloc(sizeof(Scanner));
  s->level = 0;
  s->pending_blocks = 0;
  return s;
}
static inline void destroy(Scanner *scanner) { ts_free(scanner); }

// Serialisation
static inline unsigned serialize(const Scanner *const scanner, //
                                 uint8_t *buffer) {
  buffer[0] = scanner->level;
  buffer[1] = scanner->pending_blocks;

  const unsigned bytes_used = 2;
  assert(bytes_used <= TREE_SITTER_SERIALIZATION_BUFFER_SIZE);

  return bytes_used;
}
static inline void deserialize(Scanner *scanner,   //
                               const char *buffer, //
                               unsigned length) {
  if (length > 0) {
    scanner->level = buffer[0];
    scanner->pending_blocks = buffer[1];
  }
}

// Scanning
#define TOKEN_LEXED true
#define NO_TOKEN_LEXED false
static inline bool scan(const Scanner *scanner, //
                        TSLexer *lexer,         //
                        const bool *valid_symbols) {
  // TODO: How to tell we are reading the title?

  // NOTE: How to Lex a symbol?
  /* lexer->result_symbol = BLOCK_START; */
  /* return TOKEN_LEXED; */

  if (lexer->eof(lexer)) {
    // NOTE: EoD Check
    // TODO: Figure out what to do
  }

  // NOTE: How to advance?
  /* lexer->advance(lexer, false); */

  // NOTE: How to peek?
  switch (lexer->lookahead) {
  case '!': {
    return NO_TOKEN_LEXED;
  } // ..switch:'!'

    // Regular block
  case '{': {
    return NO_TOKEN_LEXED;
  } // ..switch:'{'
  case '}': {
    return NO_TOKEN_LEXED;
  } // ..switch:'}'

  case '\n': {
    skip(lexer);
    switch (lexer->lookahead) {
    // * title {
    case '*': {
      advance(lexer);
      if (lexer->lookahead == ' ') {
        /* lexer->result_symbol = SUB_DOCUMENT; */
        return TOKEN_LEXED;
      }

      if (lexer->lookahead != '{') {
        // NOTE: Here begins the anonymous block
      }
      // NOTE: Here begins the title
      return NO_TOKEN_LEXED;
    }
    default: {
      return NO_TOKEN_LEXED;
    }
    }
  } // ..switch:'\n'

  default: {
    return NO_TOKEN_LEXED;
  } // ..switch:_

  } // ..switch
} // ..scan()

static inline bool scan_heredoc_content(const Scanner *scanner, //
                                        TSLexer *lexer,         //
                                        const bool *valid_symbols) {
  /* Heredoc *heredoc = array_get(&scanner->open_heredocs, 0); */
  /* bool look_for_heredoc_end = true; */

  /* for (;;) { */
  /* } */

  return NO_TOKEN_LEXED;
}

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
  const Scanner *const scanner = (const Scanner *)payload;
  return scan(scanner, lexer, valid_symbols);
}
