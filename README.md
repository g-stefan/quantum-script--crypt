# Quantum Script Extension Crypt

Quantum Script extension
- Symmetric encryption for scripts: `Crypt.encrypt` / `Crypt.decrypt` on
strings and buffers, `Crypt.encryptFile` / `Crypt.decryptFile` on files.
- Every result is signed: `decrypt` returns `undefined` for a wrong key or
any change, never garbage.
- A new random seed per call: the same data encrypts differently each time.
- The key bytes are used as given; `SHA512.hashToBuffer(password)` gives the
usual 64 byte key.
- Same format as the `file-crypt` tool.

```javascript
Script.requireExtension("Crypt");

Crypt;
Crypt.encrypt(key,data);
Crypt.decrypt(key,data);
Crypt.encryptFile(key,fileIn,fileOut);
Crypt.decryptFile(key,fileIn,fileOut);
```

Built on `quantum-script`, `quantum-script--buffer` and `xyo-cryptography`,
part of the XYO C++ SDK.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, load from a script, register in a C++ host, static builds
- [Script API](docs/script-api.md) - keys, every function: accepted arguments, exact results, sizes, errors
- [Recipes](docs/recipes.md) - password and key file encryption, text and JSON secrets, encrypted config, file-crypt interop, what Crypt is not for
- [C++ API](docs/cpp-api.md) - `registerInternalExtension`, `initExecutive`, encrypting from C++ with `xyo-cryptography`
- [API reference](docs/reference.md)

A Claude Code skill for this extension is in
[.claude/skills/quantum-script--crypt](.claude/skills/quantum-script--crypt/SKILL.md).

## License

Copyright (c) 2016-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
