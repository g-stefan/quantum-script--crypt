# Script API

```javascript
Script.requireExtension("Crypt");

Crypt;                                     // plain object holding the functions
Crypt.encrypt(key, data);                  // Buffer, or undefined
Crypt.decrypt(key, data);                  // Buffer, or undefined
Crypt.encryptFile(key, fileIn, fileOut);   // true / false, or undefined
Crypt.decryptFile(key, fileIn, fileOut);   // true / false, or undefined
```

`Crypt` is a global variable created by the extension (`var Crypt={};`) with
four native functions. It is not a constructor and has no state: every call
is independent. There is no streaming (`update` / `final`) interface; data
and files are processed whole, in memory.

## Loading

```javascript
Script.requireExtension("Crypt");
```

- Loads `quantum-script--crypt` (external DLL, or an internal extension
  registered by the host) and runs its `initExecutive`.
- `initExecutive` runs `Script.requireExtension("Buffer")` first, so `Buffer`
  is available afterwards, then defines `Crypt` and its functions.
- Requiring it again does nothing.
- The extension is public, versioned and listed by `Script.getExtensionList()`
  with the name `Crypt`.
- `Script.requireExtension("OpenSSL")` loads `Crypt` too and adds
  `Crypt.publicEncrypt`, `Crypt.privateDecrypt`, ... (RSA) to the same
  object; those are documented with the `OpenSSL` extension.

## Accepted arguments

`key` (all four functions) and `data` (`encrypt`, `decrypt`) must be a
`String` or a `Buffer`:

| Argument | Bytes used |
|----------|------------|
| `String` | its bytes as stored (UTF-8 for script text), no conversion |
| `Buffer` | its first `length` bytes (not `size`) |
| anything else: `Number`, `Boolean`, `null`, `undefined`, missing, `Array`, `Object` | **not accepted**: the function returns `undefined`, no error is thrown |

There is no `toString()` conversion here, unlike `SHA512.hash`:
`Crypt.encrypt(key, 123)` is `undefined`, not the encryption of `"123"`.
Convert explicitly (`"" + value`, `JSON.encode(value)`) when needed.

`fileIn` and `fileOut` are converted with `toString()`. Paths are relative to
the current working directory of the process, not to the script.

Extra arguments are ignored.

## Keys

**The key bytes are used exactly as given.** The extension does not hash,
stretch, pad or check the key; any length is accepted, including an empty
string. The convention of every XYO tool is a **64 byte** key:

| Key source | Script | `file-crypt` equivalent |
|------------|--------|-------------------------|
| A password, hashed | `SHA512.hashToBuffer(password)` | `--key-sha512 password` |
| A key file (raw bytes) | `Shell.fileGetContentsBuffer("my.key")` | `--key-read my.key` |
| Hex text | `Buffer.fromHex(hex)` | `--key-hex hex` |
| A string used directly | `"text"` | `--key text` |

Encryption and decryption must produce the same key bytes. A file encrypted
with `SHA512.hashToBuffer("secret")` is **not** opened by the key `"secret"`,
and the other way around. Prefer `SHA512.hashToBuffer(password)` over the raw
password: it gives the 64 bytes other tools expect.

Strength:

- A password key is one SHA-512 with no salt or work factor: whoever has the
  encrypted data can try passwords very fast. Use long passphrases.
- A random 64 byte key file is full strength. Quantum Script has no secure
  random generator for scripts (`Random` is Mersenne Twister, not for keys);
  create the file with an external tool, for example
  `openssl rand -out my.key 64`, and keep it safe.
- An empty key (`""`, an empty buffer) is accepted and protects nothing.

## Crypt.encrypt(key, data)

Encrypts and signs the bytes of `data` and returns them as a new `Buffer`
(`size` == `length`).

```javascript
var key = SHA512.hashToBuffer("pass");
var enc = Crypt.encrypt(key, "hello");
enc.length;                               // 200
Crypt.encrypt(key, "hello").toHex() == enc.toHex();   // false: a new random seed every call
```

- Output size for `n` input bytes: `136 + (Math.floor(n / 64) + 1) * 64`, so
  0 to 63 bytes give 200, 64 to 127 give 264, and so on. The overhead is
  137 to 200 bytes; the exact input length is hidden to within 64 bytes.
- Empty data (`""`, an empty buffer) is valid and gives 200 bytes.
- The result is binary. To store it as text use `Base64.encode(enc)` or
  `enc.toHex()`; to write it to a file use
  `Shell.filePutContentsBuffer(name, enc)`.
- Returns `undefined` only when `key` or `data` is not a `String` or
  `Buffer`. It cannot otherwise fail.

## Crypt.decrypt(key, data)

Checks the signature of `data` with `key` and, only if it is valid, decrypts
it and returns the original bytes as a new `Buffer`.

```javascript
var dec = Crypt.decrypt(key, enc);
if (Script.isUndefined(dec)) {
	throw(new Error("wrong key or damaged data"));
};
var text = dec.toString();                // "hello"
```

Returns `undefined` when:

- the key is wrong;
- any byte of `data` was changed, or bytes were added or removed;
- `data` is shorter than 136 bytes, or is not `Crypt` output at all;
- `key` or `data` is not a `String` or `Buffer`.

The signature is checked **before** decrypting, so a failure never yields
partial or garbage output. `data` may be a `Buffer` (the normal case) or a
`String` holding the same bytes (for example the result of
`Base64.decode(text)`); `Base64.decodeToBuffer(text)` is clearer.

The result is a `Buffer` even when the original was a string; call
`toString()` to get the text back (bytes are returned unchanged, so UTF-8
text comes back as UTF-8).

## Crypt.encryptFile(key, fileIn, fileOut)

Reads the whole file `fileIn`, encrypts it as `Crypt.encrypt` does and writes
the result to `fileOut`.

```javascript
if (!Crypt.encryptFile(key, "notes.txt", "notes.txt.crypt")) {
	throw(new Error("cannot encrypt notes.txt"));
};
```

- Returns `true` on success, `false` if `fileIn` cannot be read or `fileOut`
  cannot be written, `undefined` if `key` is not a `String` or `Buffer`.
- `fileOut` is created or overwritten. The original `fileIn` is left in place:
  delete it yourself (`Shell.removeFile(fileIn)`) if that is the intent, after
  checking the result.
- The file is loaded into memory whole; it suits files that fit in RAM.
- The output is the same format `file-crypt --encrypt` writes.

## Crypt.decryptFile(key, fileIn, fileOut)

Reads the whole file `fileIn`, decrypts it as `Crypt.decrypt` does and, only
on success, writes the original bytes to `fileOut`.

```javascript
if (!Crypt.decryptFile(key, "notes.txt.crypt", "notes.txt")) {
	throw(new Error("wrong key, damaged file, or cannot read / write"));
};
```

- Returns `true` on success; `false` if `fileIn` cannot be read, the key is
  wrong, the content is damaged or not `Crypt` data, or `fileOut` cannot be
  written; `undefined` if `key` is not a `String` or `Buffer`.
- When decryption fails `fileOut` is not touched: an existing file keeps its
  content and no partial file is created.
- The result does not say *why* it failed; check `Shell.fileExists(fileIn)`
  first if you need to tell a missing file from a wrong key.

## Comparing with the C++ library

| Script | `XYO::Cryptography::Crypt` |
|--------|----------------------------|
| `Crypt.encrypt(key, data)` | `encrypt(key, keySize, data, dataSize, output)` |
| `Crypt.decrypt(key, data)` | `decrypt(...)`, `undefined` for `false` |
| `Crypt.encryptFile(key, in, out)` | `encryptFile(key, keySize, in, out)` |
| `Crypt.decryptFile(key, in, out)` | `decryptFile(key, keySize, in, out)` |
| — | `checkIntegrity(...)` is not exposed to scripts |

## Errors

| Situation | Result |
|-----------|--------|
| `Crypt` used before `Script.requireExtension("Crypt")` | the usual undefined variable error |
| Extension library not found | `Script.requireExtension` throws `Unable to open "Crypt"` |
| `Buffer` extension not available (host registered `Crypt` but not `Buffer`, no Buffer DLL) | loading `Crypt` fails: its initialization requires `Buffer` |
| `key` or `data` not a `String` / `Buffer` | `undefined`, no error |
| Wrong key, damaged or foreign data | `decrypt` returns `undefined`, `decryptFile` returns `false` |
| File cannot be read or written | `encryptFile` / `decryptFile` return `false` |

None of the functions throws.
