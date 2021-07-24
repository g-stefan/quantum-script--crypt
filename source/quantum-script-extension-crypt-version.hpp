//
// Quantum Script Extension Crypt
//
// Copyright (c) 2020-2021 Grigore Stefan <g_stefan@yahoo.com>
// Created by Grigore Stefan <g_stefan@yahoo.com>
//
// MIT License (MIT) <http://opensource.org/licenses/MIT>
//

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_HPP
#define QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_HPP

#define QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_ABCD                 2,0,0,9
#define QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_STR                 "2.0.0"
#define QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_STR_BUILD           "9"
#define QUANTUM_SCRIPT_EXTENSION_CRYPT_VERSION_STR_DATETIME        "2021-07-23 16:34:02"

#ifndef XYO_RC

#ifndef QUANTUM_SCRIPT_EXTENSION_CRYPT__EXPORT_HPP
#include "quantum-script-extension-crypt--export.hpp"
#endif

namespace Quantum {
	namespace Script {
		namespace Extension {
			namespace Crypt {
				namespace Version {
					QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT const char *version();
					QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT const char *build();
					QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT const char *versionWithBuild();
					QUANTUM_SCRIPT_EXTENSION_CRYPT_EXPORT const char *datetime();
				};
			};
		};
	};
};

#endif
#endif
