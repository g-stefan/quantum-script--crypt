# C++ API

```cpp
#include <XYO/QuantumScript.Extension/Crypt.hpp>

namespace XYO::QuantumScript::Extension::Crypt {
	void initExecutive(Executive *executive, void *extensionId);
	void registerInternalExtension(Executive *executive);
};
```

The extension exposes only what a host needs to load it. The encryption is
done by `xyo-cryptography`; C++ code that wants to encrypt calls that
library directly (section 4) instead of going through a script.

## 1. registerInternalExtension

```cpp
void registerInternalExtension(Executive *executive);
```

Registers `Crypt` as an internal extension of `executive`
(`executive->registerInternalExtension("Crypt", initExecutive)`). Call it in
the host's init callback passed to `ExecutiveX::initExecutive`, after
registering `Buffer`:

```cpp
void initExecutive(Executive *executive) {
	Extension::Console::registerInternalExtension(executive);
	Extension::Buffer::registerInternalExtension(executive);
	Extension::Crypt::registerInternalExtension(executive);
};
```

Nothing runs until a script calls `Script.requireExtension("Crypt")` (or
`requireInternalExtension`); then `initExecutive` is called once for that
executive.

## 2. initExecutive

```cpp
void initExecutive(Executive *executive, void *extensionId);
```

The extension's initialization, called by the engine when the extension is
loaded. It:

1. sets the extension name (`Crypt`), info (name plus short MIT license),
   version (`Version::versionWithBuild()`) and marks it public;
2. compiles `Script.requireExtension("Buffer");` — keys and data may be
   `VariableBuffer`s and the results are `VariableBuffer`s, which need the
   per thread `Buffer` prototype;
3. compiles `var Crypt={};`;
4. registers the native functions:

```cpp
executive->setFunction2("Crypt.encrypt(key,data)", encrypt);
executive->setFunction2("Crypt.decrypt(key,data)", decrypt);
executive->setFunction2("Crypt.encryptFile(key,fileIn,fileOut)", encryptFile);
executive->setFunction2("Crypt.decryptFile(key,fileIn,fileOut)", decryptFile);
```

In the DLL build the exported C entry point forwards to it:

```cpp
extern "C" void quantumScriptExtension(XYO::QuantumScript::Executive *executive, void *extensionId);
```

The entry point is compiled only when `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY`
is defined and `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_LIBRARY` is not.

Another extension can add functions to the same object after requiring
`Crypt`, as `quantum-script--openssl` does:

```cpp
executive->compileStringX("Script.requireExtension(\"Crypt\");");
executive->setFunction2("Crypt.publicEncrypt(publicKey,data)", cryptPublicEncrypt);
```

## 3. Export macros and metadata

`Crypt/Dependency.hpp` defines `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_EXPORT`:

| Define | Effect |
|--------|--------|
| `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_INTERNAL` (or `QUANTUM_SCRIPT__CRYPT_INTERNAL`) | building the DLL: symbols exported |
| none | using the DLL: symbols imported |
| `XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_LIBRARY` | static use: export macro empty, no `quantumScriptExtension` entry point |

Library metadata, generated from `version.json` and the project info:

```cpp
namespace XYO::QuantumScript::Extension::Crypt::Version {
	const char *version();            // for example "5.9.0"
	const char *build();              // for example "7"
	const char *versionWithBuild();   // for example "5.9.0.7"
	const char *datetime();           // for example "2026-09-16 23:03:10"
};
namespace XYO::QuantumScript::Extension::Crypt::License {
	std::string license();
	std::string shortLicense();
};
namespace XYO::QuantumScript::Extension::Crypt::Copyright {
	const char *copyright();
	const char *publisher();
	const char *company();
	const char *contact();
};
```

## 4. Encryption from C++ (xyo-cryptography)

The functions behind the script API, usable from any C++ code that depends on
`xyo-cryptography`:

```cpp
#include <XYO/Cryptography.hpp>

using namespace XYO;
using namespace XYO::Cryptography;

uint8_t key[64];
SHA512::hashToU8("correct horse battery staple", key);   // the key convention: 64 bytes

// memory: what Crypt.encrypt / Crypt.decrypt do
String text("attack at dawn");
Buffer encrypted;
Crypt::encrypt(key, 64, reinterpret_cast<const uint8_t *>(text.value()), text.length(), encrypted);

Buffer decrypted;
if (!Crypt::decrypt(key, 64, encrypted.buffer, encrypted.length, decrypted)) {
	// wrong key, modified, truncated or not Crypt data: nothing was decrypted
};

// files: what Crypt.encryptFile / Crypt.decryptFile do
if (!Crypt::encryptFile(key, 64, "notes.txt", "notes.txt.crypt")) {
	// cannot read the input or write the output
};
if (!Crypt::decryptFile(key, 64, "notes.txt.crypt", "notes.txt")) {
	// cannot read, wrong key or damaged: the output file is not written
};

// not exposed to scripts: is this exactly the encrypted data signed earlier?
uint8_t signature[64];
memcpy(signature, &encrypted.buffer[64], 64);
bool same = Crypt::checkIntegrity(key, 64, encrypted.buffer, encrypted.length, signature);
```

- `encrypt` cannot fail; `output` is resized and its previous content lost.
- `decrypt` checks the signature before decrypting; `output` is written only
  on success. The input and the output must not be the same `Buffer`.
- The parameter is named `password` but is used as raw key bytes; hash the
  user password first. See `docs/crypt.md` in `xyo-cryptography` for the
  format and the security notes.

## 5. Reading a key or data argument in your own native function

The pattern the extension uses: accept a `String` or a `Buffer`, use its
bytes, return `undefined` for anything else.

```cpp
#include <XYO/QuantumScript.Extension/Buffer.hpp>
#include <XYO/Cryptography.hpp>

using namespace XYO::QuantumScript::Extension::Buffer;

static bool getBytes(Variable *value, const uint8_t *&bytes, size_t &size) {
	if (TIsType<VariableString>(value)) {
		bytes = (const uint8_t *)(((VariableString *)value)->value.value());
		size = ((VariableString *)value)->value.length();
		return true;
	};
	if (TIsType<VariableBuffer>(value)) {
		bytes = (const uint8_t *)(((VariableBuffer *)value)->buffer.buffer);
		size = ((VariableBuffer *)value)->buffer.length;
		return true;
	};
	return false;
};

static TPointer<Variable> seal(VariableFunction *function, Variable *this_, VariableArray *arguments) {
	const uint8_t *key, *data;
	size_t keySize, dataSize;
	if (!getBytes(arguments->index(0).value(), key, keySize) || !getBytes(arguments->index(1).value(), data, dataSize)) {
		return Context::getValueUndefined();
	};
	XYO::System::Buffer output;
	XYO::Cryptography::Crypt::encrypt(key, keySize, data, dataSize, output);
	return VariableBuffer::newVariable(output.buffer, output.length);
};
```

Requires `quantum-script--buffer` and `Script.requireExtension("Buffer")` in
the extension's `initExecutive`.
