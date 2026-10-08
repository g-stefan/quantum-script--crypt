# Quantum Script Extension Crypt — Documentation

`quantum-script--crypt` is the **symmetric encryption extension of Quantum
Script**. Loaded with `Script.requireExtension("Crypt")`, it adds a `Crypt`
object to scripts with four functions: encrypt and decrypt a string or a
buffer in memory, and encrypt and decrypt a whole file.

The cryptography itself is `XYO::Cryptography::Crypt` from
`xyo-cryptography`; this extension is the thin script binding on top of it.
The encrypted format is the same one the `file-crypt` command line tool
reads and writes, so data encrypted by a script can be opened by
`file-crypt` and the other way around (with the same key bytes).

- **`Crypt.encrypt(key, data)`** — encrypts and signs `data`, returns a new
  `Buffer` (136 to 200 bytes larger than the input). Every call gives a
  different result, even for the same key and data.
- **`Crypt.decrypt(key, data)`** — checks the signature first, then returns
  the original bytes as a `Buffer`; `undefined` for a wrong key, damaged,
  truncated or foreign data. It never returns garbage.
- **`Crypt.encryptFile(key, fileIn, fileOut)`** /
  **`Crypt.decryptFile(key, fileIn, fileOut)`** — the same on files,
  `true` / `false`. A failed decryption does not write the output file.

```
scripts: quantum-script .js, tools and hosts that embed the engine (quantum-script--magnet)
quantum-script--crypt     <-- this extension: Crypt.encrypt / decrypt / encryptFile / decryptFile
quantum-script--buffer    (the Buffer type for keys, input and output)
quantum-script            (Executive, Variable, Context)
xyo-cryptography          (XYO::Cryptography::Crypt, SHA512, SystemRandom)
xyo-system, xyo-encoding, xyo-multithreading, xyo-data-structures, xyo-managed-memory, xyo-platform
```

## Why it exists

| Need | What `Crypt` gives |
|------|--------------------|
| Keep a config file, token or notes encrypted on disk | `Crypt.encryptFile(key, "secrets.json", "secrets.json.crypt")` |
| Encrypt a value inside a script before storing or sending it | `Crypt.encrypt(key, text)`, then `Base64.encode(...)` for a text form |
| Know that encrypted data was not changed | the output is signed; `decrypt` returns `undefined` on any change |
| Exchange files with the `file-crypt` tool | same format, same key bytes |
| Different ciphertext every time | a fresh random seed per call, no nonce to manage |

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Load the extension | `Script.requireExtension("Crypt");` | also loads `Buffer` |
| A key from a password | `var key = SHA512.hashToBuffer(password);` | 64 bytes, the convention of every XYO tool (`file-crypt --key-sha512`) |
| A key from a key file | `var key = Shell.fileGetContentsBuffer("my.key");` | the raw bytes of the file (`file-crypt --key-read`) |
| Encrypt | `var enc = Crypt.encrypt(key, data);` | `data`: `String` or `Buffer`; result `Buffer` |
| Decrypt | `var dec = Crypt.decrypt(key, enc);` | `Buffer`, or `undefined` |
| Back to text | `dec.toString()` | the original bytes as a `String` |
| Encrypted data as text | `Base64.encode(enc)` / `Base64.decodeToBuffer(text)` | or `enc.toHex()` / `Buffer.fromHex(text)` |
| Files | `Crypt.encryptFile(key, in, out)` / `Crypt.decryptFile(key, in, out)` | `true` / `false` |

**The key is used exactly as given.** `Crypt` does not hash, stretch or check
it: a `String` key is its bytes, a `Buffer` key its first `length` bytes.
`Crypt.encrypt("secret", data)` uses the 6 bytes `"secret"` as the key and
cannot be opened with `SHA512.hashToBuffer("secret")`. Use a 64 byte key and
derive it the same way on both sides. See [Script API](script-api.md#keys).

Only `String` and `Buffer` arguments are accepted for `key` and `data`;
anything else (a number, `null`, a missing argument) makes the function
return `undefined` without an error.

## What it is not

- Not a password hash: `SHA512.hashToBuffer(password)` is one fast SHA-512
  with no salt, so a weak password can be guessed offline. Use long
  passphrases or random key files.
- Not a standard format (AES-GCM, ChaCha20-Poly1305, OpenPGP, age): it is a
  custom SHA-512 based design, tested but not externally reviewed. Other
  tools can read it only through `xyo-cryptography`.
- Not public key cryptography: for RSA see the `OpenSSL` extension, which
  adds `Crypt.publicEncrypt`, `Crypt.privateDecrypt`, ... to the same `Crypt`
  object.
- Not streaming: data and files are processed whole, in memory.

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build and install, load the extension from a script, register it in a C++ host, static builds |
| [Script API](script-api.md) | Keys, every function: accepted arguments, exact results, sizes, errors |
| [Recipes](recipes.md) | Password and key file encryption, text and JSON secrets, `file-crypt` interop, encrypted config files, re-keying |
| [C++ API](cpp-api.md) | `registerInternalExtension`, `initExecutive`, version info, encrypting from C++ with `xyo-cryptography` |
| [API reference](reference.md) | Every script and C++ symbol, the data format, on one page |

Quantum Script itself (the language, `Script.requireExtension`, embedding,
writing extensions) is documented in the `quantum-script` repository,
`docs/`; the `Buffer` type in the `quantum-script--buffer` repository,
`docs/`; the encryption format in the `xyo-cryptography` repository,
`docs/crypt.md`.

## Source map

```
source/XYO/QuantumScript.Extension/Crypt.hpp            umbrella header, include this from C++
source/XYO/QuantumScript.Extension/Crypt.Amalgam.cpp    the whole extension in one translation unit
source/XYO/QuantumScript.Extension/Crypt/
    Dependency.hpp                                      <XYO/QuantumScript.hpp>, export macro
    Library[.hpp/.cpp]                                  initExecutive, registerInternalExtension,
                                                        encrypt / decrypt / encryptFile / decryptFile
    Copyright / License / Version                       library metadata
```

## AI assistant skill

A Claude Code skill describing how to use this extension lives in
[`.claude/skills/quantum-script--crypt/`](../.claude/skills/quantum-script--crypt/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that use `Crypt`
(Quantum Script tools, hosts, other extensions).
