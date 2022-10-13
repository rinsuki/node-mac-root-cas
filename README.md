# mac-root-cas

**NOTE:** This module requires macOS 10.14 or later to run; since some modern APIs require that version.

List / Install Root CAs that installed in macOS Keychain by System Administrator or User.

It would be useful in Electron apps; since it wont read it from Keychain.

about Homebrew Node.js: It shouldn't needed, because Homebrew Node.js uses Homebrew ca-certificates, which including Keychain's Root CAs.