with import <nixpkgs> {}; 
let 
  skipGeneratedFiles = path: type:
      (lib.hasPrefix (toString path) "target") ||
      (lib.hasPrefix (toString path) "m2repo") ||
      (lib.hasPrefix (toString path) "node_modules") ||
      (lib.hasSuffix (toString path) ".jar") ;
  duktape = stdenv.mkDerivation rec {
    version= "1.4.0";          
    name = "duktape-${version}";
    src = fetchurl {
        url = "http://duktape.org/${name}.tar.xz";
        sha256 = "1f5xim0y42b77blph1ic6p7vwacmxsj6n6b2wrmqb4r83sk61q3b";
    };
    postPatch = "substituteInPlace Makefile.sharedlibrary --replace /usr/local $out";
    makeFlags = "-f Makefile.sharedlibrary ";
    preInstall = ''
      mkdir -p $out/bin $out/lib $out/include
    '';
  };
in rec {
  env = stdenv.lib.overrideDerivation weston (o: {
    name = "weston-env";
    preConfigure = "./autogen.sh NOCONFIGURE=1";
    configureFlags = "--with-duktape-path=${duktape}";
    buildInputs = o.buildInputs ++
       [  automake pkgconfig libtool m4 autoconf duktape
        ];
  });
}
