/**
 * @file Papier, A strict, simple recursive document language
 * @author Dietrich Daroch <Dietrich@Daroch.me>
 * @license MIT
 */

/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

const NL = /\n/;
const TAB = /\t/;

const REGEX_LINE = /[^\n]+/;
const REGEX_CHUNK = /[^ \t\n]+/;

const REGEX_WORD = REGEX_CHUNK;
const REGEX_PUNCTUATION = /[.,;:!?]/; // TODO: Consider /[[:punct:]]/

const REGEX_UUID =
  /[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}/;
const REGEX_ALIAS = /[a-zA-Z][-_a-zA-Z0-9]*/;
const REGEX_REF = REGEX_CHUNK;
const REGEX_TAG = new RustRegex('(?i)[a-z0-9][-_a-z0-9]*');
const REGEX_FMT = /[a-z][-_a-z0-9]*/;

/**
 * A prefix-defined token (#tag, @at, /cmd, $VAR)
 * @param {string} name
 * @param {int} precedence
 * @param {RuleOrLiteral} prefix
 * @param {RuleOrLiteral} pattern
 */
function prefix_token(name, precedence, prefix, pattern) {
  return token(prec(precedence, seq(prefix, field(name, pattern))));
}

// NOTE: This is under development and does not even work.
module.exports = grammar({
  name: 'papier',

  extras: _ => [
    // NOTE: No \n
    ' ',
    '\r', // fuck this shit!
  ],

  // These must be in sync with TokenType in ./src/scanner.c
  externals: $ => [$.doc_start],

  rules: {
    papier: $ => repeat(choice(prec(1, $.sub_document), $.text)),

    // Text
    // ----
    text: $ => prec.right(repeat1($.line)),
    line: $ => seq(optional(repeat1($.word)), NL),

    word: $ =>
      seq(
        choice(
          $.uuid,
          $.alias,
          $.ref,
          $.tag,
          $.fmt,

          REGEX_WORD,
        ),
        optional(REGEX_PUNCTUATION),
      ),
    uuid: $ => prefix_token('uuid', 2, '!', REGEX_UUID),
    alias: $ => prefix_token('alias', 1, '!!', REGEX_ALIAS),
    ref: $ => prefix_token('ref', 1, '!?', REGEX_REF),
    tag: $ => prefix_token('tag', 1, '#', REGEX_TAG),
    fmt: $ => prefix_token('format', 1, '@', REGEX_FMT),

    // Sub-document
    // ------------
    sub_document: $ =>
      prec.left(
        seq(
          $.doc_start,
          optional(field('title', repeat1($.word))),
          field('opening_brace', '{'),
          NL,
          optional(field('contents', $.contents)),
          field('closing_brace', '}'),
          optional(NL),
        ),
      ),

    contents: $ =>
      repeat1(field('line', seq(optional(seq(TAB, REGEX_LINE)), NL))),
  },
});
