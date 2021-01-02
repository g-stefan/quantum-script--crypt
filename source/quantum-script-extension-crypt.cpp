//
// Quantum Script Extension Crypt
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quantum-script-extension-crypt-license.hpp"
#include "quantum-script-extension-crypt.hpp"
#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_NO_VERSION
#include "quantum-script-extension-crypt-version.hpp"
#endif

#include "quantum-script-variableboolean.hpp"
#include "quantum-script-variablenumber.hpp"
#include "quantum-script-variablestring.hpp"
#include "quantum-script-extension-buffer-variablebuffer.hpp"

//#define QUANTUM_SCRIPT_VM_DEBUG_RUNTIME

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Crypt {

				using namespace XYO;
				using namespace Quantum::Script;
				using namespace Quantum::Script::Extension::Buffer;

				static TPointer<Variable> passwordEncrypt(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- crypt-password-encrypt\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					TPointerX<Variable> &dataV = arguments->index(1);

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;
					const uint8_t *data = nullptr;
					size_t dataSize = 0;

					if(VariableString::isVariableString(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if(VariableBuffer::isVariableBuffer(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if(VariableString::isVariableString(dataV)) {
						data = (const uint8_t *)(((VariableString *)dataV.value())->value.value());
						dataSize = ((VariableString *)dataV.value())->value.length();
					};

					if(VariableBuffer::isVariableBuffer(dataV)) {
						data = (const uint8_t *)(((VariableBuffer *)dataV.value())->buffer.buffer);
						dataSize = ((VariableBuffer *)dataV.value())->buffer.length;
					};

					if(password != nullptr) {
						if(data != nullptr) {
							XYO::Buffer output;
							XYO::Crypt::passwordEncrypt(password, passwordSize, data, dataSize, output);
							return VariableBuffer::newVariable(output.buffer, output.length);
						};
					};

					return Context::getValueUndefined();
				};

				static TPointer<Variable> passwordDecrypt(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- crypt-password-decrypt\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					TPointerX<Variable> &dataV = arguments->index(1);

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;
					const uint8_t *data = nullptr;
					size_t dataSize = 0;

					if(VariableString::isVariableString(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if(VariableBuffer::isVariableBuffer(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if(VariableString::isVariableString(dataV)) {
						data = (const uint8_t *)(((VariableString *)dataV.value())->value.value());
						dataSize = ((VariableString *)dataV.value())->value.length();
					};

					if(VariableBuffer::isVariableBuffer(dataV)) {
						data = (const uint8_t *)(((VariableBuffer *)dataV.value())->buffer.buffer);
						dataSize = ((VariableBuffer *)dataV.value())->buffer.length;
					};

					if(password != nullptr) {
						if(data != nullptr) {
							XYO::Buffer output;
							if(XYO::Crypt::passwordDecrypt(password, passwordSize, data, dataSize, output)) {
								return VariableBuffer::newVariable(output.buffer, output.length);
							};
						};
					};

					return Context::getValueUndefined();
				};

				static TPointer<Variable> passwordEncryptFile(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- crypt-password-encrypt-file\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					String fileIn = (arguments->index(1))->toString();
					String fileOut = (arguments->index(2))->toString();

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;

					if(VariableString::isVariableString(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if(VariableBuffer::isVariableBuffer(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if(password != nullptr) {
						return VariableBoolean::newVariable(XYO::Crypt::passwordEncryptFile(password, passwordSize, fileIn, fileOut));
					};

					return Context::getValueUndefined();
				};

				static TPointer<Variable> passwordDecryptFile(VariableFunction *function, Variable *this_, VariableArray *arguments) {
#ifdef QUANTUM_SCRIPT_DEBUG_RUNTIME
					printf("- crypt-password-decrypt-file\n");
#endif
					TPointerX<Variable> &passwordV = arguments->index(0);
					String fileIn = (arguments->index(1))->toString();
					String fileOut = (arguments->index(2))->toString();

					const uint8_t *password = nullptr;
					size_t passwordSize = 0;

					if(VariableString::isVariableString(passwordV)) {
						password = (const uint8_t *)(((VariableString *)passwordV.value())->value.value());
						passwordSize = ((VariableString *)passwordV.value())->value.length();
					};

					if(VariableBuffer::isVariableBuffer(passwordV)) {
						password = (const uint8_t *)(((VariableBuffer *)passwordV.value())->buffer.buffer);
						passwordSize = ((VariableBuffer *)passwordV.value())->buffer.length;
					};

					if(password != nullptr) {
						return VariableBoolean::newVariable(XYO::Crypt::passwordDecryptFile(password, passwordSize, fileIn, fileOut));
					};

					return Context::getValueUndefined();
				};


				void registerInternalExtension(Executive *executive) {
					executive->registerInternalExtension("Crypt", initExecutive);
				};

				void initExecutive(Executive *executive, void *extensionId) {
					String info = "Crypt\r\n";
					info << License::shortContent();

					executive->setExtensionName(extensionId, "Crypt");
					executive->setExtensionInfo(extensionId, info);
#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_NO_VERSION
					executive->setExtensionVersion(extensionId, Extension::Crypt::Version::versionWithBuild());
#endif
					executive->setExtensionPublic(extensionId, true);

					executive->compileStringX("Script.requireExtension(\"Buffer\");");

					executive->compileStringX(
						"var Crypt={};"
					);

					executive->setFunction2("Crypt.passwordEncrypt(key,data)", passwordEncrypt);
					executive->setFunction2("Crypt.passwordDecrypt(key,data)", passwordDecrypt);
					executive->setFunction2("Crypt.passwordEncryptFile(key,fileIn,fileOut)", passwordEncryptFile);
					executive->setFunction2("Crypt.passwordDecryptFile(key,fileIn,fileOut)", passwordDecryptFile);
				};

			};
		};
	};
};

#ifdef XYO_COMPILE_DYNAMIC_LIBRARY
extern "C" QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT void quantumScriptExtension(Quantum::Script::Executive *executive, void *extensionId) {
	Quantum::Script::Extension::Crypt::initExecutive(executive, extensionId);
};
#endif

