// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <array>
#include <string>
#include <vector>

#include <DecentEnclave/Common/Logging.hpp>
#include <DecentEnclave/Untrusted/CUrl.hpp>
#include <EclipseMonitor/Eth/DataTypes.hpp>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleObjects/Codec/Hex.hpp>
#include <SimpleObjects/SimpleObjects.hpp>


namespace DecentEthereum
{
namespace Untrusted
{


class GethRequester
{
public: // static members:


	using Logger = typename DecentEnclave::Common::LoggerFactory::LoggerType;


public:


	GethRequester(const std::string& url) :
		m_logger(DecentEnclave::Common::LoggerFactory::GetLogger(
			"DecentEthereum::Untrusted::GethRequester"
		)),
		m_url(url)
	{}


	~GethRequester() = default;


	std::vector<uint8_t> GetHeaderRlpByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"debug_getRawHeader",
		//       "params":["0x1"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqBodyValGetHdlRlp =
			"debug_getRawHeader";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqBodyValGetHdlRlp,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespSingleBytes<std::vector<uint8_t> >(respBodyJson);
	}


	std::vector<uint8_t> GetBodyRlpByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"debug_getRawBlock",
		//       "params":["0x1"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqMethodGetBlkRlp =
			"debug_getRawBlock";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqMethodGetBlkRlp,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespSingleBytes<std::vector<uint8_t> >(respBodyJson);
	}


	template<typename _RetType>
	_RetType GetReceiptsRlpByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"debug_getRawReceipts",
		//       "params":["0x1"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqMethodGetRawRec =
			"debug_getRawReceipts";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqMethodGetRawRec,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespListOfBytes<_RetType>(respBodyJson);
	}


	std::array<uint8_t, 32> SendRawTransactionByParam(
		const std::string& param
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"eth_sendRawTransaction",
		//       "params":["0xabcdef"], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqMethodSentRawTxn =
			"eth_sendRawTransaction";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqMethodSentRawTxn,
			{
				SimpleObjects::String(param),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);
		m_logger.Debug("Received response: " + respBodyJson);

		auto txnHash = ProcRespSingleBytesArray<32>(respBodyJson);

		// WaitAndGetTransactionReceipt(txnHash);

		return txnHash;
	}


	template<typename _BytesType>
	std::array<uint8_t, 32> SendRawTransactionByBytes(
		const _BytesType& bytes
	) const
	{
		return SendRawTransactionByParam(
			SimpleObjects::Codec::Hex::Encode<std::string>(bytes, "0x")
		);
	}


	std::vector<uint8_t> GetHeaderRlpByNum(
		EclipseMonitor::Eth::BlockNumber blockNum
	) const
	{
		return GetHeaderRlpByParam(ConvertBlkNumToHex(blockNum));
	}


	std::vector<uint8_t> GetBodyRlpByNum(
		EclipseMonitor::Eth::BlockNumber blockNum
	) const
	{
		return GetBodyRlpByParam(ConvertBlkNumToHex(blockNum));
	}


	template<typename _RetType>
	_RetType GetReceiptsRlpByNum(
		EclipseMonitor::Eth::BlockNumber blockNum
	) const
	{
		return GetReceiptsRlpByParam<_RetType>(ConvertBlkNumToHex(blockNum));
	}


	std::string GetTransactionReceipt(
		const std::array<uint8_t, 32>& txnHash
	) const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"eth_getTransactionReceipt",
		//       "params":["0xabcdef..."], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqMethodGetTxnRec =
			"eth_getTransactionReceipt";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqMethodGetTxnRec,
			{
				SimpleObjects::Codec::Hex::Encode<SimpleObjects::String>(txnHash, "0x"),
			}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);
		m_logger.Debug("Received response: " + respBodyJson);

		return ProcRespObject(respBodyJson);
	}


	std::string WaitAndGetTransactionReceipt(
		const std::array<uint8_t, 32>& txnHash
	) const
	{
		std::string objStr = "null";
		while (true)
		{
			objStr = GetTransactionReceipt(txnHash);
			if (objStr == "null")
			{
				// waiting for the transaction to be mined
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			else
			{
				return objStr;
			}
		}
	}


	uint64_t GetBlockNumber() const
	{
		// curl "http://127.0.0.1:8545/" -X POST
		//   -H "Content-Type: application/json"
		//   --data
		//     '{ "method":"eth_blockNumber",
		//       "params":[], "id":1, "jsonrpc":"2.0" }'

		static const SimpleObjects::String sk_reqMethodGetBlkNum =
			"eth_blockNumber";

		std::string reqBodyJson = BuildRequestBody(
			sk_reqMethodGetBlkNum,
			{}
		);

		std::string respBodyJson = PostRequest(reqBodyJson);

		return ProcRespInt<uint64_t>(respBodyJson);
	}


protected:


	static std::string BuildRequestBody(
		SimpleObjects::String method,
		SimpleObjects::List params
	)
	{
		static const SimpleObjects::String sk_reqBodyLabelMethod = "method";
		static const SimpleObjects::String sk_reqBodyLabelParams = "params";
		static const SimpleObjects::String sk_reqBodyLabelId = "id";
		static const SimpleObjects::String sk_reqBodyLabelJsonRpc = "jsonrpc";

		static const SimpleObjects::UInt8 sk_reqBodyValId =
			SimpleObjects::UInt8(1);
		static const SimpleObjects::String sk_reqBodyValJsonRpc =
			"2.0";

		SimpleObjects::Dict reqBody;
		reqBody[sk_reqBodyLabelMethod]  = std::move(method);
		reqBody[sk_reqBodyLabelParams]  = std::move(params);
		reqBody[sk_reqBodyLabelId]      = sk_reqBodyValId;
		reqBody[sk_reqBodyLabelJsonRpc] = sk_reqBodyValJsonRpc;

		std::string reqBodyJson = SimpleJson::DumpStr(reqBody);

		return reqBodyJson;
	}


	std::string PostRequest(
		const std::string& reqBody
	) const
	{
		// m_logger.Debug("Sending request: " + reqBody);

		std::string respBody;
		DecentEnclave::Untrusted::CUrlContentCallBack contentCallback =
			[&respBody]
			(char* ptr, size_t size, size_t nmemb, void*) -> size_t
			{
				respBody += std::string(ptr, size * nmemb);

				return size * nmemb;
			};

		DecentEnclave::Untrusted::CUrlRequestExpectRespCode(
			m_url,
			"POST",
			{
				"Content-Type: application/json",
			},
			reqBody,
			nullptr,
			&contentCallback,
			200
		);

		// m_logger.Debug("Received response: " + respBody);
		return respBody;
	}


	template<typename _RetType, typename _InType>
	static _RetType DecodeHexStr(const _InType& hexStr)
	{
		if (
			hexStr.size() < 3 ||
			hexStr[0] != '0' ||
			hexStr[1] != 'x'
		)
		{
			throw std::runtime_error("Invalid response from Geth");
		}

		return SimpleObjects::Codec::Hex::Decode<_RetType>(
			hexStr.begin() + 2,
			hexStr.end()
		);
	}


	template<typename _RetType>
	static _RetType ProcRespSingleBytes(
		const std::string& respBody
	)
	{
		static const SimpleObjects::String sk_respBodyLabelResult = "result";

		auto respBodyJson = SimpleJson::LoadStr(respBody);
		const auto& resHex =
			respBodyJson.AsDict()[sk_respBodyLabelResult].AsString();

		return DecodeHexStr<_RetType>(resHex.AsString());
	}


	template<size_t _ArrSize>
	static std::array<uint8_t, _ArrSize> ProcRespSingleBytesArray(
		const std::string& respBody
	)
	{
		auto vec = ProcRespSingleBytes<std::vector<uint8_t> >(respBody);
		if (vec.size() != _ArrSize)
		{
			throw std::runtime_error(
				"Geth returned a byte string that is not of the length of " +
				std::to_string(_ArrSize)
			);
		}
		auto arr = std::array<uint8_t, _ArrSize>();
		std::copy(vec.begin(), vec.end(), arr.begin());
		return arr;
	}


	template<typename _RetType>
	static _RetType ProcRespListOfBytes(
		const std::string& respBody
	)
	{
		using _RetTypeValType = typename _RetType::value_type;

		static const SimpleObjects::String sk_respBodyLabelResult = "result";

		auto respBodyJson = SimpleJson::LoadStr(respBody);
		const auto& resList =
			respBodyJson.AsDict()[sk_respBodyLabelResult].AsList();

		_RetType res;
		res.reserve(resList.size());
		for (const auto& resHex : resList)
		{
			res.push_back(DecodeHexStr<_RetTypeValType>(resHex.AsString()));
		}

		return res;
	}


	static std::string ProcRespObject(
		const std::string& respBody
	)
	{
		static const SimpleObjects::String sk_respBodyLabelResult = "result";

		auto respBodyJson = SimpleJson::LoadStr(respBody);
		const auto& resObj = respBodyJson.AsDict()[sk_respBodyLabelResult];

		return SimpleJson::DumpStr(resObj);
	}


	template<typename _IntType>
	static _IntType ProcRespInt(
		const std::string& respBody
	)
	{
		static const SimpleObjects::String sk_respBodyLabelResult = "result";

		auto respBodyJson = SimpleJson::LoadStr(respBody);
		const auto& resHex =
			respBodyJson.AsDict()[sk_respBodyLabelResult].AsString();

		auto resBytes = DecodeHexStr<SimpleObjects::Bytes>(resHex.AsString());

		return EclipseMonitor::Eth::PrimitiveTypeTrait<_IntType>::
			FromBytes(resBytes);
	}


	static std::string ConvertBlkNumToHex(
		EclipseMonitor::Eth::BlockNumber blockNum
	)
	{
		return SimpleObjects::Codec::Hex::
			template Encode<std::string>(blockNum);
	}


private:

	Logger m_logger;
	std::string m_url;


}; // class GethRequester


} // namespace Untrusted
} // namespace DecentEthereum
