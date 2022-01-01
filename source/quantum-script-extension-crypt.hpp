//
// Quantum Script Extension Crypt
//
// Copyright (c) 2020-2022 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_HPP
#define QUANTUM_SCRIPT_EXTENSION_CRYPT_HPP

#ifndef QUANTUM_SCRIPT_HPP
#include "quantum-script.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT__EXPORT_HPP
#include "quantum-script-extension-crypt--export.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_COPYRIGHT_HPP
#include "quantum-script-extension-crypt-copyright.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_LICENSE_HPP
#include "quantum-script-extension-crypt-license.hpp"
#endif

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_HPP
#include "quantum-script-extension-crypt-version.hpp"
#endif

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Crypt {

				using namespace Quantum::Script;

				QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT void initExecutive(Executive *executive, void *extensionId);
				QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT void registerInternalExtension(Executive *executive);

			};
		};
	};
};

#endif

