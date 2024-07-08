// Copyright (c) 2024 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <memory>
#include <string>
#include <vector>

#include <DecentEnclave/Common/Logging.hpp>

#include <SimpleObjects/Codec/Hex.hpp>

#include "BlockchainMgr.hpp"
#include "DataType.hpp"


namespace DecentEthereum
{
namespace Trusted
{


namespace Transaction
{


template<typename _NetConfig>
inline void SendRaw(
	std::shared_ptr<BlockchainMgr<_NetConfig> > bcMgrPtr,
	LambdaMsgSocketPtr& socket,
	const LambdaMsgIdExt& msgIdExt,
	const LambdaMsgContent& msgContentAdvRlp
)
{
	static DecentEnclave::Common::Logger s_logger =
		DecentEnclave::Common::LoggerFactory::GetLogger(
			"DecentEthereum::Trusted::Transaction::SendRaw"
		);

	(void)socket;
	(void)msgIdExt;

	std::vector<uint8_t> rawTxn(
		msgContentAdvRlp.begin(),
		msgContentAdvRlp.end()
	);

	auto txnHash = bcMgrPtr->GetHostBlockService().SendRawTransaction(rawTxn);
	auto txnHashStr = SimpleObjects::Codec::Hex::Encode<std::string>(txnHash);

	s_logger.Info("Transaction with hash: " + txnHashStr + " sent");
}


} // namespace Transaction


} // namespace Trusted
} // namespace DecentEthereum

