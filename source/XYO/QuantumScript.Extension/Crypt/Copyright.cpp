// Quantum Script Extension Crypt
// Copyright (c) 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2016-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/QuantumScript.Extension/Crypt/Copyright.hpp>
#include <XYO/QuantumScript.Extension/Crypt/Copyright.rh>

namespace XYO::QuantumScript::Extension::Crypt::Copyright {

	static const char *copyright_ = XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_COPYRIGHT;
	static const char *publisher_ = XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_PUBLISHER;
	static const char *company_ = XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_COMPANY;
	static const char *contact_ = XYO_QUANTUMSCRIPT_EXTENSION_CRYPT_CONTACT;

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

};
