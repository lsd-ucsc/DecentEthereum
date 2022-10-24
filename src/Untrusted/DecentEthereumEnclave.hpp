// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Untrusted/Sgx/DecentSgxEnclave.hpp>

#include "../Enclave_u.h"

namespace DecentEthereum
{

class DecentEthereumEnclave :
	public DecentEnclave::Untrusted::Sgx::DecentSgxEnclave
{
public: // static members:

	using Base = DecentEnclave::Untrusted::Sgx::DecentSgxEnclave;


public:

	using Base::Base;

	void Init()
	{
		ecall_decent_ethereum_init(m_encId);
	}


}; // class DecentEthereumEnclave

} // namespace DecentEthereum
