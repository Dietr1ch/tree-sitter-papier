{
  description = "Papier's tree-sitter grammar";

  inputs = {
    nixpkgs = {
      url = "github:NixOS/nixpkgs";
    };
  };

  outputs =
    {
      nixpkgs,
      ...
    }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
      ];
      forEachSupportedSystem =
        f:
        nixpkgs.lib.genAttrs supportedSystems (
          system:
          f {
            pkgs = import nixpkgs {
              inherit system;
            };
          }
        );
    in
    {
      devShells = forEachSupportedSystem (
        { pkgs }:
        {
          default = pkgs.mkShell {
            packages = with pkgs; [
              just
              bacon

              # Tree sitter
              tree-sitter
              gcc
              nodejs

              # JSON
              prettier
              # JS
              js-beautify

              # Nix
              nixfmt
            ];
          };
        }
      );
    };
}
