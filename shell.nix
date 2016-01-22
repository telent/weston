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
  immutableJs = fetchurl {
    url = "https://raw.githubusercontent.com/facebook/immutable-js/d94141223d56ac5bd6f541b06baa3161c4edd01b/dist/immutable.js";
    sha256 = "158qq08v4rkj85yjbz5xaw645761zzgz6rn3fvy8ci5fwi8jkxf0";
  };
in rec {
  env = stdenv.lib.overrideDerivation weston (o: {
    name = "weston-env";
    preConfigure = "./autogen.sh NOCONFIGURE=1";
    IMMUTABLE_JS = "${immutableJs}";
    configureFlags = "--with-duktape-path=${duktape}";
    buildInputs = o.buildInputs ++
       [  automake pkgconfig libtool m4 autoconf duktape
        ];
  });
}
