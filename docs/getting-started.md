# Getting started

## 1. Build and install

The extension is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. `quantum-script` (and everything
below it: `xyo-system`, `xyo-encoding`, ...), `quantum-script--console`,
`quantum-script--buffer` and `xyo-cryptography` must be installed to the SDK
first. From the repository root:

```bash
fabricare make       # build into output/
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

`fabricare.json` has one project:

| Project | Make | Result |
|---------|------|--------|
| `quantum-script--crypt` | `dll-or-lib` | `quantum-script--crypt.dll` / `libquantum-script--crypt.so` on dynamic platforms (`win64-msvc-2026`, Linux), a static library `quantum-script--crypt.lib` on static ones (`win64-msvc-2026.static`) |

After `fabricare install` on a dynamic platform the DLL sits in the SDK `bin`
folder next to `quantum-script.exe`, which is where
`Script.requireExtension("Crypt")` finds it. Release archives
(`release/xyo.quantum-script--crypt.v<version>.<platform>.{bin,dev,static.dev}.zip`)
hold the same files.

The repository has no `test/` project; check a build with a short script
(section 2).

## 2. Use it from a script

```javascript
Script.requireExtension("Console");
Script.requireExtension("SHA512");
Script.requireExtension("Crypt");

var key = SHA512.hashToBuffer("correct horse battery staple");   // 64 byte key

var enc = Crypt.encrypt(key, "attack at dawn");
Console.writeLn(enc.length);                    // 200: 136 + 64 for 14 bytes of input

var dec = Crypt.decrypt(key, enc);
if (Script.isUndefined(dec)) {
	Console.writeLn("wrong key or damaged data");
} else {
	Console.writeLn(dec.toString());            // attack at dawn
};

Console.writeLn(Crypt.decrypt(SHA512.hashToBuffer("wrong"), enc));   // undefined
```

Run it with:

```bash
quantum-script hello-crypt.js
```

`Script.requireExtension("Crypt")` looks for an external
`quantum-script--crypt` library first (the file as named, then every include
path folder: next to the interpreter, next to the script), then for an
internal extension registered by the host. Loading twice does nothing. A
missing extension throws `Unable to open "Crypt"`.

Loading `Crypt` also loads `Buffer` (the extension runs
`Script.requireExtension("Buffer")` while it initializes), so the `Buffer`
global exists afterwards. `SHA512`, `Shell` and `Base64`, used in the
examples to make keys and to store results, are separate extensions and are
loaded only when the script asks for them.

### Files

```javascript
Script.requireExtension("SHA512");
Script.requireExtension("Crypt");

var key = SHA512.hashToBuffer(password);
if (!Crypt.encryptFile(key, "notes.txt", "notes.txt.crypt")) {
	throw(new Error("cannot encrypt notes.txt"));
};
if (!Crypt.decryptFile(key, "notes.txt.crypt", "notes.decrypted.txt")) {
	throw(new Error("wrong key, damaged file or cannot write the output"));
};
```

The same file can be opened from the command line:

```bash
file-crypt --decrypt --key-sha512 "correct horse battery staple" notes.txt.crypt notes.txt
```

### fabricare build scripts

fabricare does **not** embed `Crypt` and cannot load extension DLLs, so
`Script.requireExtension("Crypt")` fails inside fabricare scripts. Run a
separate `quantum-script` process, or call `file-crypt`, from the build
script instead.

## 3. Register it in a C++ host

A host that embeds Quantum Script makes `Crypt` available as an internal
extension by registering it, together with `Buffer`, in the init callback:

```cpp
#include <XYO/QuantumScript.hpp>
#include <XYO/QuantumScript.Extension/Console.hpp>
#include <XYO/QuantumScript.Extension/Buffer.hpp>
#include <XYO/QuantumScript.Extension/SHA512.hpp>
#include <XYO/QuantumScript.Extension/Crypt.hpp>

using namespace XYO::QuantumScript;

void initExecutive(Executive *executive) {
	Extension::Console::registerInternalExtension(executive);
	Extension::Buffer::registerInternalExtension(executive);
	Extension::SHA512::registerInternalExtension(executive);
	Extension::Crypt::registerInternalExtension(executive);
};

int main(int cmdN, char *cmdS[]) {
	if (ExecutiveX::initExecutive(cmdN, cmdS, initExecutive)) {
		if (!ExecutiveX::executeString(
		        "Script.requireExtension(\"Console\");"
		        "Script.requireExtension(\"SHA512\");"
		        "Script.requireExtension(\"Crypt\");"
		        "var key = SHA512.hashToBuffer(\"pass\");"
		        "Console.writeLn(Crypt.decrypt(key, Crypt.encrypt(key, \"ok\")).toString());")) {
			printf("%s\n", (ExecutiveX::getError()).value());
			printf("%s", (ExecutiveX::getStackTrace()).value());
		};
		ExecutiveX::endProcessing();
	};
	return 0;
};
```

`Buffer` must be registered too (or be loadable as a DLL): `Crypt` requires
it during its own initialization. `SHA512` is only needed by scripts that
derive keys from passwords.

Registering only makes the extension *available*: scripts still call
`Script.requireExtension("Crypt")`. With the DLL build of the engine an
external `quantum-script--crypt.dll` found on the include path wins over the
internal one for `requireExtension`; use
`Script.requireInternalExtension("Crypt")` to force the internal one.

In the host's `fabricare.json`:

```json
{
	"name": "my-host",
	"make": "exe",
	"sourcePath": "XYO/MyHost",
	"dependency": [
		"quantum-script--crypt",
		"quantum-script--sha512"
	]
}
```

`quantum-script--crypt` brings `quantum-script`, `quantum-script--console`,
`quantum-script--buffer` and `xyo-cryptography` with it.
`quantum-script--magnet` is an example of a host that registers `Crypt`
this way; `quantum-script--openssl` depends on it and adds its RSA functions
to the same `Crypt` object.

## 4. Static builds

There is no separate `quantum-script--crypt.static` project. On a static
platform (`win64-msvc-2026.static`) the `dll-or-lib` project is built as a
static library, `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` is not defined, so the
`quantumScriptExtension` DLL entry point is left out. A host built on that
platform depends on `quantum-script--crypt` as usual and must register the
extension (and `Buffer`) with `registerInternalExtension` (section 3):
external DLLs cannot be loaded into a host that does not use the engine DLL.

## 5. Threads

Each thread that runs scripts has its own engine, so every thread loads the
extension itself with `Script.requireExtension("Crypt")`. The functions keep
no state between calls; each call reads its own arguments and returns a new
buffer, so there is nothing to share or lock. Two threads must not write the
same output file at the same time.
