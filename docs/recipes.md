# Recipes

All examples assume:

```javascript
Script.requireExtension("Console");
Script.requireExtension("SHA512");
Script.requireExtension("Crypt");
```

## Encrypt and decrypt text with a password

```javascript
function encryptText(password, text) {
	return Crypt.encrypt(SHA512.hashToBuffer(password), text);     // Buffer
};

function decryptText(password, encrypted) {
	var dec = Crypt.decrypt(SHA512.hashToBuffer(password), encrypted);
	if (Script.isUndefined(dec)) {
		return undefined;                                           // wrong password or damaged
	};
	return dec.toString();
};

var enc = encryptText("correct horse battery staple", "attack at dawn");
Console.writeLn(decryptText("correct horse battery staple", enc));   // attack at dawn
Console.writeLn(decryptText("wrong", enc));                          // undefined
```

Hash the password once and reuse the key when encrypting many values.

## Store encrypted data as text

The output of `Crypt.encrypt` is binary. For JSON, a database field, an
environment variable or a URL, encode it:

```javascript
Script.requireExtension("Base64");

var key = SHA512.hashToBuffer(password);

var stored = Base64.encode(Crypt.encrypt(key, "api-token-value"));   // text, about 268 characters

var data = Base64.decodeToBuffer(stored);                            // Buffer, or undefined if not Base64
var dec = data ? Crypt.decrypt(key, data) : undefined;
if (Script.isUndefined(dec)) {
	throw(new Error("cannot decrypt the stored token"));
};
var token = dec.toString();
```

`enc.toHex()` / `Buffer.fromHex(text)` work too and are twice the size of
the binary data.

## Encrypt a JSON object

`Crypt` takes only strings and buffers; encode objects first:

```javascript
Script.requireExtension("JSON");

var settings = {user: "admin", token: "s3cr3t"};
var enc = Crypt.encrypt(key, JSON.encode(settings));

var dec = Crypt.decrypt(key, enc);
var back = Script.isUndefined(dec) ? null : JSON.decode(dec.toString());
```

## Encrypted configuration file

Keep `config.json.crypt` in the project and decrypt it in memory when the
tool starts; the plain file never touches the disk:

```javascript
Script.requireExtension("Shell");
Script.requireExtension("JSON");

function loadEncryptedConfig(fileName, key) {
	var data = Shell.fileGetContentsBuffer(fileName);
	if (!data) {
		return null;                                     // missing or empty
	};
	var dec = Crypt.decrypt(key, data);
	if (Script.isUndefined(dec)) {
		throw(new Error("cannot decrypt " + fileName + ": wrong key or damaged"));
	};
	return JSON.decode(dec.toString());
};

function saveEncryptedConfig(fileName, key, config) {
	var enc = Crypt.encrypt(key, JSON.encodeWithIndentation(config));
	if (!Shell.filePutContentsBuffer(fileName, enc)) {
		throw(new Error("cannot write " + fileName));
	};
};

var password = Shell.getenv("MY_TOOL_PASSWORD");
if (password.length == 0) {
	throw(new Error("MY_TOOL_PASSWORD is not set"));
};
var key = SHA512.hashToBuffer(password);
var config = loadEncryptedConfig("config.json.crypt", key);
```

Reading the password from an environment variable keeps it out of the
script. `Shell.getenv` returns `""` when the variable is not set, and an
empty password is accepted by `Crypt` and protects nothing, so check it.

## Encrypt a file and remove the original

```javascript
Script.requireExtension("Shell");

function encryptAndRemove(key, name) {
	var out = name + ".crypt";
	if (!Crypt.encryptFile(key, name, out)) {
		return false;
	};
	// prove the encrypted copy opens before deleting the original
	var check = Crypt.decrypt(key, Shell.fileGetContentsBuffer(out));
	if (Script.isUndefined(check)) {
		Shell.removeFile(out);
		return false;
	};
	return Shell.removeFile(name);
};
```

Removing a file does not wipe its old content from the disk.

## Decrypt to a temporary file

```javascript
Script.requireExtension("Shell");

var tmp = "temp/report.pdf";
Shell.mkdirRecursivelyIfNotExists("temp");
if (!Crypt.decryptFile(key, "report.pdf.crypt", tmp)) {
	throw(new Error("cannot decrypt report.pdf.crypt"));
};
// ... use temp/report.pdf ...
Shell.removeFile(tmp);
```

When the content is only read by the script, decrypt into memory instead
(`Crypt.decrypt(key, Shell.fileGetContentsBuffer(name))`).

## Use a key file

A random key file gives full strength, independent of any password. Create
it once outside Quantum Script (scripts have no secure random generator):

```bash
openssl rand -out my.key 64
```

```javascript
Script.requireExtension("Shell");

var key = Shell.fileGetContentsBuffer("my.key");
if (!key || key.length != 64) {
	throw(new Error("my.key is missing or is not a 64 byte key"));
};
Crypt.encryptFile(key, "backup.zip", "backup.zip.crypt");
```

A key derived from a password can be saved the same way, so later runs do
not need the password: `Shell.filePutContentsBuffer("my.key",
SHA512.hashToBuffer(password))` (what `file-crypt --gen-key-sha512-write`
does). Protect the key file like the password.

## Interoperate with file-crypt

`file-crypt` uses the same format. The key option decides the key bytes:

| file-crypt | Script key |
|------------|------------|
| `--key-sha512 "pass"` | `SHA512.hashToBuffer("pass")` |
| `--key-read my.key` | `Shell.fileGetContentsBuffer("my.key")` |
| `--key-hex 00ff...` | `Buffer.fromHex("00ff...")` |
| `--key "pass"` | `"pass"` (raw string, not hashed) |

```bash
file-crypt --encrypt --key-sha512 "pass" data.bin data.bin.crypt
```

```javascript
Crypt.decryptFile(SHA512.hashToBuffer("pass"), "data.bin.crypt", "data.bin");   // true
```

## Change the key

There is no re-key operation: decrypt with the old key, encrypt with the new
one, and replace the file only when both steps worked.

```javascript
Script.requireExtension("Shell");

function rekeyFile(name, oldKey, newKey) {
	var dec = Crypt.decrypt(oldKey, Shell.fileGetContentsBuffer(name));
	if (Script.isUndefined(dec)) {
		return false;
	};
	var tmp = name + ".new";
	if (!Shell.filePutContentsBuffer(tmp, Crypt.encrypt(newKey, dec))) {
		return false;
	};
	Shell.removeFile(name);
	return Shell.rename(tmp, name);
};
```

## Check a file without writing it

To know whether a key opens a file, decrypt into memory and drop the result:

```javascript
Script.requireExtension("Shell");

function canDecrypt(key, name) {
	var data = Shell.fileGetContentsBuffer(name);
	return !Script.isUndefined(data) && !Script.isUndefined(Crypt.decrypt(key, data));
};
```

## Detect tampering

Every encrypted buffer is signed with the key. Changing a single byte makes
`decrypt` fail:

```javascript
var enc = Crypt.encrypt(key, "pay 10");
enc.setU8(150, enc.getU8(150) ^ 1);         // flip one bit of the encrypted data
Crypt.decrypt(key, enc);                     // undefined
```

The signature proves the data was made by someone who has the key; it does
not tell *who*. For signatures between different parties use public key
cryptography (`OpenSSL`).

## What Crypt is not for

- **Storing passwords.** To check a password later you need a password hash
  (Argon2, scrypt, bcrypt, PBKDF2), not encryption. No Quantum Script
  extension provides one.
- **Weak passwords.** A key from `SHA512.hashToBuffer(password)` is only as
  strong as the password; there is no salt or work factor.
- **Talking to other software.** The format is specific to
  `xyo-cryptography`; other languages and tools cannot read it. Use TLS
  (`OpenSSL`, `HTTP`) for network traffic and standard formats for exchange.
- **Very large files.** Everything is processed in memory at once.
- **Hiding the size.** The encrypted size reveals the input size to within
  64 bytes.
