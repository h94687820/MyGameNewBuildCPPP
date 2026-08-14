{pkgs}: {
  deps = [
    pkgs.ninja
    pkgs.android-tools
    pkgs.gradle
    pkgs.jdk17
    pkgs.cmake
    pkgs.pkg-config
    pkgs.raylib
  ];
}
