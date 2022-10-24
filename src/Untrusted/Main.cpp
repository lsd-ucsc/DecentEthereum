// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#include "DecentEthereumEnclave.hpp"

using namespace DecentEthereum;

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	DecentEthereumEnclave enclave;
	enclave.Init();

	return 0;
}
