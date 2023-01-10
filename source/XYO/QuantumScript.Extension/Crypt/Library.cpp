// Quantum Script Extension Crypt
// Copyright (c) 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/QuantumScript.Extension/Crypt/Library.hpp>
#include <XYO/QuantumScript.Extension/Crypt/Copyright.hpp>
#include <XYO/QuantumScript.Extension/Crypt/License.hpp>
#include <XYO/QuantumScript.Extension/Crypt/Version.hpp>
#include <XYO/QuantumScript.Extension/Buffer/VariableBuffer.hpp>
#include <XYO/Cryptography.hpp>

namespace XYO::QuantumScript::Extension::Crypt {
	using namespace XYO::QuantumScript::Extension::Buffer;

	static TPointer<Variable> encrypt(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
					printf("- crypt-encrypt\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					TPointerX<Variable> &dataV = arguments->index(1);

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;
					const uint8_t *data = nullptr;
					size_t dataSize = 0;

					if (TIsType<VariableString>(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if (TIsType<VariableBuffer>(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if (TIsType<VariableString>(dataV)) {
						data = (const uint8_t *)(((VariableString *)dataV.value())->value.value());
						dataSize = ((VariableString *)dataV.value())->value.length();
					};

					if (TIsType<VariableBuffer>(dataV)) {
						data = (const uint8_t *)(((VariableBuffer *)dataV.value())->buffer.buffer);
						dataSize = ((VariableBuffer *)dataV.value())->buffer.length;
					};

					if (password != nullptr) {
						if (data != nullptr) {
							XYO::System::Buffer output;
							XYO::Cryptography::Crypt::encrypt(password, passwordSize, data, dataSize, output);
							return VariableBuffer::newVariable(output.buffer, output.length);
						};
					};

					return Context::getValueUndefined();
				};

				static TPointer<Variable> decrypt(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
					printf("- crypt-decrypt\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					TPointerX<Variable> &dataV = arguments->index(1);

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;
					const uint8_t *data = nullptr;
					size_t dataSize = 0;

					if (TIsType<VariableString>(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if (TIsType<VariableBuffer>(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if (TIsType<VariableString>(dataV)) {
						data = (const uint8_t *)(((VariableString *)dataV.value())->value.value());
						dataSize = ((VariableString *)dataV.value())->value.length();
					};

					if (TIsType<VariableBuffer>(dataV)) {
						data = (const uint8_t *)(((VariableBuffer *)dataV.value())->buffer.buffer);
						dataSize = ((VariableBuffer *)dataV.value())->buffer.length;
					};

					if (password != nullptr) {
						if (data != nullptr) {
							XYO::System::Buffer output;
							if (XYO::Cryptography::Crypt::decrypt(password, passwordSize, data, dataSize, output)) {
								return VariableBuffer::newVariable(output.buffer, output.length);
							};
						};
					};

					return Context::getValueUndefined();
				};

				static TPointer<Variable> encryptFile(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
					printf("- crypt-encrypt-file\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					String fileIn = (arguments->index(1))->toString();
					String fileOut = (arguments->index(2))->toString();

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;

					if (TIsType<VariableString>(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if (TIsType<VariableBuffer>(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if (password != nullptr) {
						return VariableBoolean::newVariable(XYO::Cryptography::Crypt::encryptFile(password, passwordSize, fileIn, fileOut));
					};

					return Context::getValueUndefined();
				};

				static TPointer<Variable> decryptFile(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef XYO_QUANTUMSCRIPT_DEBUG_RUNTIME
					printf("- crypt-decrypt-file\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					String fileIn = (arguments->index(1))->toString();
					String fileOut = (arguments->index(2))->toString();

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;

					if (TIsType<VariableString>(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if (TIsType<VariableBuffer>(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if (password != nullptr) {
						return VariableBoolean::newVariable(XYO::Cryptography::Crypt::decryptFile(password, passwordSize, fileIn, fileOut));
					};

					return Context::getValueUndefined();
				};

	void registerInternalExtension(Executive *executive) {
		executive->registerInternalExtension("Crypt", initExecutive);
	};

	void initExecutive(Executive *executive, void *extensionId) {
		String info = "Crypt\r\n";
		info << License::shortLicense();

		executive->setExtensionName(extensionId, "Crypt");
		executive->setExtensionInfo(extensionId, info);
		executive->setExtensionVersion(extensionId, Extension::Crypt::Version::versionWithBuild());
		executive->setExtensionPublic(extensionId, true);

		executive->compileStringX("Script.requireExtension(\"Buffer\");");
		executive->compileStringX("var Crypt={};");

		executive->setFunction2("Crypt.encrypt(key,data)", encrypt);
		executive->setFunction2("Crypt.decrypt(key,data)", decrypt);
		executive->setFunction2("Crypt.encryptFile(key,fileIn,fileOut)", encryptFile);
		executive->setFunction2("Crypt.decryptFile(key,fileIn,fileOut)", decryptFile);
	};

};

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
extern "C" XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_EXPORT void quantumScriptExtension(XYO::QuantumScript::Executive *executive, void *extensionId) {
	XYO::QuantumScript::Extension::Crypt::initExecutive(executive, extensionId);
};
#endif
