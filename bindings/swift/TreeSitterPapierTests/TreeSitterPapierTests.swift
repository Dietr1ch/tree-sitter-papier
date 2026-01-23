import XCTest
import SwiftTreeSitter
import TreeSitterPapier

final class TreeSitterPapierTests: XCTestCase {
    func testCanLoadGrammar() throws {
        let parser = Parser()
        let language = Language(language: tree_sitter_papier())
        XCTAssertNoThrow(try parser.setLanguage(language),
                         "Error loading Papier grammar")
    }
}
