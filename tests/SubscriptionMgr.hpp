// // Copyright (c) 2023 Haofan Zheng
// // Use of this source code is governed by an MIT-style
// // license that can be found in the LICENSE file or at
// // https://opensource.org/licenses/MIT.

// #pragma once


// #include <array>
// #include<functional>
// #include <memory>
// #include <mutex>
// #include <unordered_map>
// #include <vector>

// #include <EclipseMonitor/Eth/Keccak256.hpp>
// #include <EclipseMonitor/Eth/ReceiptMgr.hpp>
// #include <EclipseMonitor/Eth/Trie/Trie.hpp>
// #include <SimpleObjects/Internal/make_unique.hpp>
// #include <SimpleRlp/SimpleRlp.hpp>

// #include "../HostBlockService.hpp"


// namespace DecentEthereum
// {
// namespace Trusted
// {
// namespace Pubsub
// {

// struct SubDescription
// {
// 	using ContractAddrType =
// 		typename EclipseMonitor::Eth::ReceiptLogEntry::ContractAddrType;
// 	using TopicType =
// 		typename EclipseMonitor::Eth::ReceiptLogEntry::TopicType;
// 	using HashType = std::array<uint8_t, 32>;

// 	using HeaderMgr       = EclipseMonitor::Eth::HeaderMgr;
// 	using ReceiptLogEntry = EclipseMonitor::Eth::ReceiptLogEntry;
// 	using NotifyCallbackType =
// 		std::function<void(const HeaderMgr&, const ReceiptLogEntry&)>;

// 	SubDescription(
// 		ContractAddrType       contractAddr,
// 		std::vector<TopicType> topics,
// 		NotifyCallbackType     notifyCallback
// 	) :
// 		m_contractAddr(std::move(contractAddr)),
// 		m_topics(std::move(topics)),
// 		m_hashes(),
// 		m_notifyCallback(std::move(notifyCallback))
// 	{
// 		m_hashes.reserve(1 + m_topics.size());
// 		m_hashes.emplace_back(EclipseMonitor::Eth::Keccak256(m_contractAddr));
// 		for (const auto& topic : m_topics)
// 		{
// 			m_hashes.emplace_back(EclipseMonitor::Eth::Keccak256(topic));
// 		}
// 	}

// 	SubDescription(SubDescription&& other) :
// 		m_contractAddr(std::move(other.m_contractAddr)),
// 		m_topics(std::move(other.m_topics)),
// 		m_hashes(std::move(other.m_hashes)),
// 		m_notifyCallback(std::move(other.m_notifyCallback))
// 	{}

// 	~SubDescription() = default;

// 	ContractAddrType       m_contractAddr;
// 	std::vector<TopicType> m_topics;
// 	std::vector<HashType>  m_hashes;
// 	NotifyCallbackType     m_notifyCallback;
// }; // struct SubDescription


// class SubscriptionMgr
// {
// public: // static members:

// 	using SubscribeIdType = std::uintptr_t;

// 	using SubscribeMapType = std::unordered_map<
// 		SubscribeIdType,
// 		std::unique_ptr<SubDescription>
// 	>;

// public:

// 	SubscriptionMgr() :
// 		m_subMapMutex(),
// 		m_subMap()
// 	{}

// 	~SubscriptionMgr() = default;

// 	SubscribeIdType Subscribe(SubDescription&& subDesc)
// 	{
// 		std::lock_guard<std::mutex> lock(m_subMapMutex);

// 		std::unique_ptr<SubDescription> subDescPtr =
// 			SimpleObjects::Internal::make_unique<SubDescription>(
// 				std::move(subDesc)
// 			);
// 		SubscribeIdType id =
// 			reinterpret_cast<SubscribeIdType>(subDescPtr.get());
// 		m_subMap.emplace(id, std::move(subDescPtr));

// 		return id;
// 	}

// 	void CheckEventsAndNotify(
// 		const EclipseMonitor::Eth::HeaderMgr& headerMgr,
// 		const HostBlockService& hostBlkSvc
// 	) const
// 	{
// 		using _ReceiptMgr = EclipseMonitor::Eth::ReceiptMgr;
// 		using _LogEntriesKRefType =
// 			typename _ReceiptMgr::LogEntriesKRefType;
// 		using _NotifyPair = std::pair<
// 			std::vector<_LogEntriesKRefType>,
// 			typename SubDescription::NotifyCallbackType
// 		>;

// 		std::vector<_NotifyPair> notifyList;
// 		std::vector<_ReceiptMgr> receiptMgrs;

// 		{
// 			std::lock_guard<std::mutex> lock(m_subMapMutex);

// 			// find if any subscription is found via the bloom filter.
// 			std::vector<SubscribeMapType::const_iterator> foundSubs;
// 			for (auto it = m_subMap.cbegin(); it != m_subMap.cend(); ++it)
// 			{
// 				bool isFound = headerMgr.GetBloomFilter().AreHashesInBloom(
// 					it->second->m_hashes.begin(),
// 					it->second->m_hashes.end()
// 				);
// 				if (isFound)
// 				{
// 					foundSubs.emplace_back(it);
// 				}
// 			}

// 			// nothing found in bloom filter;
// 			// By the nature of bloom filter, there is no false negative.
// 			// Thus, stop here
// 			if (foundSubs.empty())
// 			{
// 				return;
// 			}

// 			// otherwise, check the receipt root, and check the receipt logs
// 			// we must verify the receipt root first, because we also want to
// 			// ensure if the event is not found in the receipt, it is really
// 			// not there.
// 			auto receiptsRlp =
// 				hostBlkSvc.GetReceiptsRlpByNum(headerMgr.GetNumber());
// 			const auto& receiptsList = receiptsRlp.AsList();
// 			VerifyReceiptRoot(
// 				receiptsList,
// 				headerMgr.GetRawHeader().get_ReceiptsRoot()
// 			);

// 			// build list of receipt managers
// 			receiptMgrs.reserve(receiptsList.size());
// 			for (const auto& receipt : receiptsList)
// 			{
// 				receiptMgrs.emplace_back(
// 					_ReceiptMgr::FromBytes(receipt.AsBytes())
// 				);
// 			}

// 			// search through the receipt managers
// 			for (const auto& receiptMgr: receiptMgrs)
// 			{
// 				for (const auto& foundSub : foundSubs)
// 				{
// 					auto logIts = receiptMgr.SearchEvents(
// 						foundSub->second->m_contractAddr,
// 						foundSub->second->m_topics.cbegin(),
// 						foundSub->second->m_topics.cend()
// 					);
// 					if (!logIts.empty())
// 					{
// 						// it's confirmed that the event is found
// 						// add to the notify list
// 						notifyList.emplace_back(
// 							std::move(logIts),
// 							foundSub->second->m_notifyCallback
// 						);
// 					}
// 				}
// 			}
// 		}

// 		// Now we've finished searching through the receipt managers
// 		// and the subscription map is unlocked.

// 		// start to notify
// 		for (const auto& notifyPair : notifyList)
// 		{
// 			for (const auto& logIt : notifyPair.first)
// 			{
// 				notifyPair.second(headerMgr, logIt);
// 			}
// 		}
// 	}

// private: // static members:

// 	static void VerifyReceiptRoot(
// 		const SimpleObjects::ListBaseObj& receipts,
// 		const SimpleObjects::Bytes& expReceiptRoot
// 	)
// 	{
// 		using IntWriter = SimpleRlp::EncodePrimitiveIntValue<
// 			uint64_t,
// 			SimpleRlp::Endian::native,
// 			false
// 		>;

// 		EclipseMonitor::Eth::Trie::PatriciaTrie trie;

// 		SimpleObjects::Bytes keyBigEndian;
// 		for(uint64_t i = 0; i < receipts.size(); i++)
// 		{
// 			keyBigEndian.resize(0);
// 			IntWriter::Encode(i, std::back_inserter(keyBigEndian));
// 			SimpleObjects::Bytes encodedKey = SimpleObjects::Bytes(
// 				SimpleRlp::WriteRlp(keyBigEndian)
// 			);

// 			trie.Put(encodedKey, receipts[i].AsBytes());
// 		}

// 		if (trie.Hash() != expReceiptRoot)
// 		{
// 			throw std::runtime_error("Receipt root does not match.");
// 		}
// 	}

// private:

// 	mutable std::mutex m_subMapMutex;
// 	SubscribeMapType   m_subMap;
// }; // class SubscriptionMgr


// } // namespace Pubsub
// } // namespace Trusted
// } // namespace DecentEthereum
