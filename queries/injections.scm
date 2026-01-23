; References:
; - https://github.com/tree-sitter/tree-sitter-embedded-template/blob/master/queries/

; (sub_document
;   (type (language) @injection.language)
;   (sub_document_content) @injection.content)

; TODO: Get dynamic injection by reading the @fmt declaration?
((contents) @injection.content
 (#set! injection.language "md")
 (#set! injection.combined))

((raw_contents) @injection.content
 (#set! injection.language "md")
 (#set! injection.combined))
