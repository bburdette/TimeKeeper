let 
  nixpkgs = import <nixpkgs> {};
in
  with nixpkgs;
  stdenv.mkDerivation {
    name = "timekeeper-env";

  
    buildInputs = [
      codeblocks
      # codeblocksFull
      poco
      pkgconfig
      gnome2.gtkmm
      gnome2.libglademm
      gnome2.libglade
      # enable for editing *.glade files
      # glade
      nix
      ];

  NIX_CFLAGS_COMPILE = "-isystem ${lib.getDev gnome2.libglade}/include/libglade-2.0";

  }
