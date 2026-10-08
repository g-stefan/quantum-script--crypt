---
name: quantum-script--crypt
description: >-
  How to use the Quantum Script Crypt extension (quantum-script--crypt), the
  symmetric encryption functions loaded with Script.requireExtension("Crypt"):
  Crypt.encrypt(key, data) and Crypt.decrypt(key, data) (Buffer, or
  undefined), Crypt.encryptFile(key, fileIn, fileOut) and
  Crypt.decryptFile(key, fileIn, fileOut) (true / false); keys are used as
  given, never hashed (SHA512.hashToBuffer(password) is the 64 byte
  convention, key files via Shell.fileGetContentsBuffer, Buffer.fromHex);
  only String / Buffer arguments are accepted (anything else -> undefined);
  decrypt verifies a signature first and returns undefined for a wrong key or
  any change; output size 136 + (floor(n/64)+1)*64; storing results as text
  with Base64; encrypted config files; interop with the file-crypt tool
  (--key-sha512, --key-read, --key-hex, --key); what it is not for
  (password storage, weak passwords, standard formats, streaming); the C++
  side (registerInternalExtension, initExecutive, Buffer registered first,
  XYO::Cryptography::Crypt encrypt / decrypt / encryptFile / decryptFile /
  checkIntegrity). Use when writing or reviewing Quantum Script code that
  calls Crypt, C++ code that includes <XYO/QuantumScript.Extension/Crypt.hpp>,
  a fabricare.json depending on "quantum-script--crypt", or when working
  inside the quantum-script--crypt repository.
---

# quantum-script--crypt

Symmetric encryption extension of Quantum Script (see the `quantum-script`
skill for the language and its differences from JavaScript, and the
`quantum-script--buffer` skill for the `Buffer` type; their rules apply).
Purpose: **encrypt and sign data and files from scripts** — encrypted
config files, secrets stored as text, files exchanged with the `file-crypt`
tool. It binds `XYO::Cryptography::Crypt` (`xyo-cryptography`): a SHA-512
based stream cipher with a keyed SHA-512 signature and a random seed per
call.

Full documentation: `docs/` in the quantum-script--crypt repository
(`X:\Storage\XYO\Gitea\CPP\quantum-script--crypt\docs` on this machine):
README, getting-started, **script-api** (keys, accepted arguments, exact
results, sizes, errors), **recipes** (password / key file encryption, text
and JSON secrets, encrypted config, file-crypt interop, re-keying, what not
to use it for), cpp-api, reference (includes the data format). Read the
matching page when you need more than this summary. The whole
implementation is `source/XYO/QuantumScript.Extension/Crypt/Library.cpp`
(~190 lines) on top of `xyo-cryptography` (`docs/crypt.md` there describes
the format).

## Script API

```javascript
Script.requireExtension("SHA512");
Script.requireExtension("Crypt");                  // also loads Buffer

var key = SHA512.hashToBuffer(password);           // 64 byte key, the XYO convention
var enc = Crypt.encrypt(key, "text or Buffer");    // Buffer; different bytes every call
var dec = Crypt.decrypt(key, enc);                 // Buffer, or undefined
if (Script.isUndefined(dec)) { throw(new Error("wrong key or damaged data")); };
var text = dec.toString();

if (!Crypt.encryptFile(key, "a.txt", "a.txt.crypt")) { throw(new Error("encrypt failed")); };
if (!Crypt.decryptFile(key, "a.txt.crypt", "a.txt")) { throw(new Error("decrypt failed")); };
```

## Hard rules

1. **Only four functions**: `encrypt`, `decrypt`, `encryptFile`,
   `decryptFile`. No `new Crypt`, no streaming, no `checkIntegrity` in
   scripts, no algorithm / mode / IV options. `Crypt` is a plain object.
   (`OpenSSL`, when loaded, adds RSA `Crypt.publicEncrypt` /
   `privateDecrypt` / ... to the same object — a different thing.)
2. **The key is used exactly as given — never hashed.** `"secret"` as key is
   6 raw bytes; it does not open data encrypted with
   `SHA512.hashToBuffer("secret")`. Always derive the key the same way on
   both sides; prefer `SHA512.hashToBuffer(password)` (64 bytes, matches
   `file-crypt --key-sha512`). Key file: `Shell.fileGetContentsBuffer(name)`
   (`--key-read`); hex: `Buffer.fromHex(hex)` (`--key-hex`); raw string =
   `--key`.
3. **`key` and `data` must be `String` or `Buffer`.** Anything else
   (number, `null`, `undefined`, object, missing) makes the function return
   `undefined` silently — no `toString()` conversion, unlike `SHA512.hash`.
   Encode first: `JSON.encode(obj)`, `"" + n`. A `Buffer` contributes its
   first `length` bytes.
4. **Always check results.** `encrypt` is `undefined` only for bad argument
   types; `decrypt` is `undefined` for a wrong key, any changed / added /
   removed byte, data shorter than 136 bytes or not Crypt data;
   `encryptFile` / `decryptFile` return `false` on read / write failure (and
   `decryptFile` on wrong key / damage), `undefined` for a bad key type.
   None throws. A failed `decryptFile` leaves `fileOut` untouched.
5. **Results are binary `Buffer`s.** `dec.toString()` for text. To store
   ciphertext as text: `Base64.encode(enc)` and
   `Base64.decodeToBuffer(text)` (or `toHex` / `Buffer.fromHex`). To write:
   `Shell.filePutContentsBuffer(name, enc)`; to read:
   `Shell.fileGetContentsBuffer(name)`. Never `Shell.filePutContents` /
   `fileGetContents` text round trips for ciphertext.
6. **Non deterministic by design**: encrypting the same data twice gives
   different bytes (random seed). Do not compare ciphertexts; decrypt and
   compare plaintexts, or hash the plaintext with `SHA512`.
7. **Size**: `136 + (Math.floor(n / 64) + 1) * 64` bytes (0..63 -> 200,
   64..127 -> 264); the input length leaks to within 64 bytes.
8. **Whole data in memory**: no streaming; `encryptFile` / `decryptFile`
   load the entire file. Paths are relative to the process working
   directory. `encryptFile` does not delete `fileIn`; verify the output before
   removing it (`Shell.removeFile` does not wipe).
9. **Key strength**: `SHA512.hashToBuffer(password)` is one unsalted fast
   hash — weak passwords are guessable offline. An empty key is accepted and
   protects nothing; check `Shell.getenv(...)` results (`""` when unset).
   Scripts have no secure random generator (`Random` is Mersenne Twister);
   make random 64 byte key files externally (`openssl rand -out my.key 64`).
10. **Not for**: storing passwords (needs Argon2 / scrypt / bcrypt / PBKDF2,
    none available), interoperating with non-XYO software (custom format —
    use TLS / standard formats), public key use (see `OpenSSL`). Signature
    check uses `memcmp` (not constant time); the design is tested but not
    externally reviewed.
11. **Not in fabricare**: fabricare does not embed `Crypt` and cannot load
    DLLs; run `quantum-script` or `file-crypt` from build scripts instead.
12. **One engine per thread**: each thread requires `Crypt` itself; the
    functions keep no state.

## file-crypt interop

```javascript
// file-crypt --encrypt --key-sha512 "pass" data.bin data.bin.crypt
Crypt.decryptFile(SHA512.hashToBuffer("pass"), "data.bin.crypt", "data.bin");   // true
// file-crypt --decrypt --key-read my.key x.crypt x
Crypt.encryptFile(Shell.fileGetContentsBuffer("my.key"), "x", "x.crypt");
```

## C++

```cpp
#include <XYO/QuantumScript.Extension/Buffer.hpp>
#include <XYO/QuantumScript.Extension/Crypt.hpp>
using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {                  // host init callback
	Extension::Buffer::registerInternalExtension(executive);   // required: Crypt requires Buffer while initializing
	Extension::Crypt::registerInternalExtension(executive);    // scripts still call requireExtension("Crypt")
};
```

Encrypting in C++ without a script (`#include <XYO/Cryptography.hpp>`,
dependency `xyo-cryptography`):

```cpp
uint8_t key[64]; XYO::Cryptography::SHA512::hashToU8(password, key);
XYO::Cryptography::Crypt::encrypt(key, 64, data, dataSize, output);           // cannot fail
bool ok = XYO::Cryptography::Crypt::decrypt(key, 64, in, inSize, output);     // false: output untouched
XYO::Cryptography::Crypt::encryptFile(key, 64, "in", "out");                  // bool
XYO::Cryptography::Crypt::decryptFile(key, 64, "in", "out");                  // bool, out written only on success
XYO::Cryptography::Crypt::checkIntegrity(key, 64, in, inSize, signature64);   // bytes 64..127 of a previous encryption
```

- `fabricare.json` dependency: `"quantum-script--crypt"` (brings
  `quantum-script`, `quantum-script--console`, `quantum-script--buffer`,
  `xyo-cryptography`; `dll-or-lib`: DLL on dynamic platforms, static library
  on `win64-msvc-2026.static`). There is no `.static` project. Static hosts
  register it (and `Buffer`) as internal. Export macro:
  `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_EXPORT`; `..._INTERNAL` when building
  the DLL, `..._LIBRARY` for static use (empty export, no
  `quantumScriptExtension` entry point).
- Users: `quantum-script--magnet` (internal), `quantum-script--openssl`
  (requires it, extends `Crypt`).

## Working in this repository

- Build: `fabricare make`, `fabricare install`, `fabricare clean` (see the
  `fabricare` skill). `quantum-script`, `quantum-script--console`,
  `quantum-script--buffer` and `xyo-cryptography` must be installed first.
  There is no `test/` project.
- Quick check with the installed DLL: `quantum-script script.js` where the
  script encrypts and decrypts a round trip; cross-check with
  `file-crypt --decrypt --key-sha512 pass in out`.
- Native functions live in `Crypt/Library.cpp` as
  `static TPointer<Variable> name(VariableFunction *, Variable *this_, VariableArray *arguments)`
  and are registered in `initExecutive` with
  `executive->setFunction2("Crypt.name(args)", name)`. Arguments are read
  with `TIsType<VariableString>` / `TIsType<VariableBuffer>`.
- Changing the API: update `README.md`, `docs/script-api.md`,
  `docs/reference.md`, `docs/recipes.md` when relevant, and this skill. The
  format belongs to `xyo-cryptography`; changing it breaks `file-crypt`
  compatibility and existing encrypted files.
- Code style: tabs (width 8), `.clang-format`, CRLF, statements and blocks
  end with `};`, camelCase. SPDX header: MIT for `source/` and `docs/`,
  Unlicense for `.claude/` (see `.reuse/dep5`).
