# API reference

## Script

| Symbol | Returns | Description |
|--------|---------|-------------|
| `Script.requireExtension("Crypt")` | | load the extension (also loads `Buffer`) |
| `Crypt` | `Object` | holder of the functions below, not a constructor |
| `Crypt.encrypt(key, data)` | `Buffer` or `undefined` | encrypt and sign `data`; a different result every call; `undefined` only for invalid argument types |
| `Crypt.decrypt(key, data)` | `Buffer` or `undefined` | verify, then decrypt; `undefined` for a wrong key, changed, truncated or foreign data, invalid argument types |
| `Crypt.encryptFile(key, fileIn, fileOut)` | `Boolean` or `undefined` | encrypt a whole file; `false` if it cannot read or write; `undefined` if `key` is invalid |
| `Crypt.decryptFile(key, fileIn, fileOut)` | `Boolean` or `undefined` | decrypt a whole file; `false` on read / write error, wrong key or damaged data (`fileOut` untouched); `undefined` if `key` is invalid |

Arguments:

| Argument | Accepted | Bytes used |
|----------|----------|------------|
| `key`, `data` | `String` | its bytes as stored |
| `key`, `data` | `Buffer` | its first `length` bytes |
| `key`, `data` | anything else | not accepted: result `undefined` |
| `fileIn`, `fileOut` | any | `toString()`, relative to the process working directory |

Keys (used as given, never hashed by `Crypt`):

| Key | Script | `file-crypt` |
|-----|--------|--------------|
| password | `SHA512.hashToBuffer(password)` (64 bytes) | `--key-sha512 password` |
| key file | `Shell.fileGetContentsBuffer(name)` | `--key-read name` |
| hex | `Buffer.fromHex(hex)` | `--key-hex hex` |
| raw string | `"text"` | `--key text` |

Sizes:

| Input bytes `n` | Encrypted bytes |
|-----------------|-----------------|
| formula | `136 + (Math.floor(n / 64) + 1) * 64` |
| 0 .. 63 | 200 |
| 64 .. 127 | 264 |
| 1000 | 1160 |

## Data format

From `XYO::Cryptography::Crypt` (`xyo-cryptography`, `docs/crypt.md`):

```
offset  size     content
0       64       seed: SHA512(key || data || time in ms || 64 bytes from SystemRandom)
64      64       signature: SHA512(seed || keyBase || encrypted length || encrypted data)
128     8        data length, little endian, encrypted
136     m * 64   data, encrypted, padded; m = floor(n / 64) + 1

keyBase      = SHA512(seed || key)
stream(c)    = SHA512(seed || keyBase || c)          c = 64 bit little endian counter
length field = n XOR stream(0) XOR stream(1)
block i      = data[i] XOR stream(2 + 2i) XOR stream(3 + 2i)
```

## C++ — `XYO::QuantumScript::Extension::Crypt`

| Symbol | Header | Description |
|--------|--------|-------------|
| `void registerInternalExtension(Executive *executive)` | `Crypt/Library.hpp` | register `Crypt` as an internal extension |
| `void initExecutive(Executive *executive, void *extensionId)` | `Crypt/Library.hpp` | extension initialization: metadata, `Buffer`, `Crypt` object and functions |
| `extern "C" void quantumScriptExtension(Executive *, void *)` | `Crypt/Library.cpp` | DLL entry point, forwards to `initExecutive` |
| `const char *Version::version()` / `build()` / `versionWithBuild()` / `datetime()` | `Crypt/Version.hpp` | version info from `version.json` |
| `std::string License::license()` / `shortLicense()` | `Crypt/License.hpp` | MIT license text |
| `const char *Copyright::copyright()` / `publisher()` / `company()` / `contact()` | `Crypt/Copyright.hpp` | copyright info |
| `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_EXPORT` | `Crypt/Dependency.hpp` | import / export macro |
| `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_INTERNAL` | | define when building the DLL |
| `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_LIBRARY` | | define for static use: empty export macro, no DLL entry point |

Umbrella header: `<XYO/QuantumScript.Extension/Crypt.hpp>`.
Whole extension in one translation unit: `Crypt.Amalgam.cpp`.

## C++ — `XYO::Cryptography::Crypt` (used by the extension)

| Symbol | Description |
|--------|-------------|
| `void encrypt(const uint8_t *password, size_t passwordSize, const uint8_t *data, size_t dataSize, Buffer &output)` | encrypt and sign, cannot fail |
| `bool decrypt(const uint8_t *password, size_t passwordSize, const uint8_t *data, size_t dataSize, Buffer &output)` | verify, then decrypt; `false` without touching `output` |
| `bool encryptFile(const uint8_t *password, size_t passwordSize, const char *fileNameIn, const char *fileNameOut)` | whole file, in memory |
| `bool decryptFile(const uint8_t *password, size_t passwordSize, const char *fileNameIn, const char *fileNameOut)` | whole file, output written only on success |
| `bool checkIntegrity(const uint8_t *password, size_t passwordSize, const uint8_t *data, size_t dataSize, const uint8_t *integrity)` | signature valid and equal to `integrity` (64 bytes); not exposed to scripts |

## fabricare.json

| Project | Make | Depends on |
|---------|------|------------|
| `quantum-script--crypt` | `dll-or-lib` (DLL on dynamic platforms, static library on static ones) | `quantum-script`, `quantum-script--console`, `quantum-script--buffer`, `xyo-cryptography` |

Known users: `quantum-script--magnet` (registers it as internal),
`quantum-script--openssl` (requires it and adds RSA functions to `Crypt`).
Not embedded in fabricare.

## Related

| Extension / tool | Relation |
|------------------|----------|
| `SHA512` | `SHA512.hashToBuffer(password)` makes the usual 64 byte key |
| `Buffer` | type of keys, input and results |
| `Base64`, `Base16` | store encrypted buffers as text |
| `Shell` | `fileGetContentsBuffer` / `filePutContentsBuffer` for key files and encrypted data |
| `OpenSSL` | `Crypt.publicEncrypt` / `publicDecrypt` / `privateEncrypt` / `privateDecrypt` and the `...File` forms (RSA) on the same object |
| `file-crypt` | command line tool, same format |
