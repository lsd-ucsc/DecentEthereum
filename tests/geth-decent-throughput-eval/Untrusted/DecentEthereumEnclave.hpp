// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Common/Sgx/Exceptions.hpp>
#include <DecentEnclave/Untrusted/Sgx/DecentSgxEnclave.hpp>

#include <DecentEthereum/Untrusted/BlockReceiver.hpp>
#include <DecentEthereum/Untrusted/HostBlockService.hpp>


extern "C" sgx_status_t ecall_decent_ethereum_init(
	sgx_enclave_id_t eid,
	sgx_status_t*    retval,
	void*            host_blk_svc
);
extern "C" sgx_status_t ecall_decent_ethereum_set_receipt_rate(
	sgx_enclave_id_t eid,
	sgx_status_t*    retval,
	double           receipt_rate
);
extern "C" sgx_status_t ecall_decent_ethereum_recv_block(
	sgx_enclave_id_t eid,
	sgx_status_t*    retval,
	const uint8_t*   blk_data,
	size_t           blk_size
);


namespace DecentEthereum
{

class DecentEthereumEnclave :
	public DecentEnclave::Untrusted::Sgx::DecentSgxEnclave,
	public BlockReceiver
{
public: // static members:

	using Base = DecentEnclave::Untrusted::Sgx::DecentSgxEnclave;


public:


	DecentEthereumEnclave(
		std::shared_ptr<HostBlockService> hostBlockService,
		const std::vector<uint8_t>& authList,
		const std::string& enclaveImgPath = DECENT_ENCLAVE_PLATFORM_SGX_IMAGE,
		const std::string& launchTokenPath = DECENT_ENCLAVE_PLATFORM_SGX_TOKEN
	) :
		Base(authList, enclaveImgPath, launchTokenPath),
		m_hostBlockService(hostBlockService)
	{
		DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
			ecall_decent_ethereum_init,
			m_encId,
			m_hostBlockService.get()
		);
	}


	virtual void RecvBlock(const std::vector<uint8_t>& blockRlp) override
	{
		DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
			ecall_decent_ethereum_recv_block,
			m_encId,
			blockRlp.data(),
			blockRlp.size()
		);
	}


	void SetReceiptRate(double receiptRate)
	{
		DECENTENCLAVE_SGX_ECALL_CHECK_ERROR_E_R(
			ecall_decent_ethereum_set_receipt_rate,
			m_encId,
			receiptRate
		);
	}


private:
	std::shared_ptr<HostBlockService> m_hostBlockService;
}; // class DecentEthereumEnclave

} // namespace DecentEthereum
