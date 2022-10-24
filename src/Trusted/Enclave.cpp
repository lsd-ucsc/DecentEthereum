// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include <DecentEnclave/Common/Platform/Print.hpp>

using namespace DecentEnclave::Common;

extern "C" void ecall_decent_ethereum_init()
{
	Platform::Print::Str("Hello, world!\n");
}
