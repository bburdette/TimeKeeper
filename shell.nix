let 
  nixpkgs = import <nixpkgs> {};
in
  with nixpkgs;
  stdenv.mkDerivation {
    name = "timekeeper-env";
    buildInputs = [
      codeblocksFull
      poco
      pkgconfig
      gnome2.gtkmm
      gnome2.libglademm
      # enable for editing *.glade files
      # glade
      nix
      ];
  }
