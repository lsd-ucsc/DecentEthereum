// Copyright (c) 2023 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Common/Sgx/Exceptions.hpp>
#include <DecentEnclave/Trusted/Sgx/UntrustedBuffer.hpp>
#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleRlp/SimpleRlp.hpp>


extern "C" sgx_status_t ocall_decent_ethereum_get_receipts(
	sgx_status_t* retval,
	const void*   host_blk_svc,
	uint64_t      blk_num,
	uint8_t**     out_buf,
	size_t*       out_buf_size
);

extern "C" sgx_status_t ocall_decent_ethereum_get_latest_blknum(
	sgx_status_t* retval,
	const void*   host_blk_svc,
	uint64_t*     out_blk_num
);

extern "C" sgx_status_t ocall_decent_ethereum_send_raw_transaction(
	sgx_status_t* retval,
	const void*   host_blk_svc,
	const uint8_t* in_txn,
	size_t in_txn_size,
	uint8_t* out_txn_hash
);


namespace DecentEthereum
{
namespace Trusted
{


class HostBlockService
{
public:
	HostBlockService(void* hostBlkSvc) :
		m_ptr(hostBlkSvc)
	{}

	~HostBlockService() = default;

	SimpleObjects::Object GetReceiptsRlpByNum(uint64_t blockNum) const
	{
		DecentEnclave::Trusted::Sgx::UntrustedBuffer<uint8_t> ub;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_ethereum_get_receipts,
			m_ptr,
			blockNum,
			&(ub.m_data),
			&(ub.m_size)
		);

		auto rlp = ub.CopyToContainer<std::vector<uint8_t> >();

		return SimpleRlp::GeneralParser().Parse(rlp);
	}

	uint64_t GetLatestBlockNum() const
	{
		uint64_t ret;
		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_ethereum_get_latest_blknum,
			m_ptr,
			&ret
		);

		return ret;
	}

	std::array<uint8_t, 32> SendRawTransaction(
		const std::vector<uint8_t>& bytes
	) const
	{
		auto txnHash = std::array<uint8_t, 32>();

		DECENTENCLAVE_SGX_OCALL_CHECK_ERROR_E_R(
			ocall_decent_ethereum_send_raw_transaction,
			m_ptr,
			bytes.data(),
			bytes.size(),
			txnHash.data()
		);

		return txnHash;
	}

private:

	void* m_ptr;
}; // class HostBlockService


} // namespace Trusted
} // namespace DecentEthereum
