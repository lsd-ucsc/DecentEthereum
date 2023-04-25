// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstddef>

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <EclipseMonitor/Eth/DataTypes.hpp>
#include <SimpleConcurrency/Threading/TickingTask.hpp>
#include <SimpleRlp/SimpleRlp.hpp>

#include "BlockReceiver.hpp"
#include "GethRequester.hpp"


namespace DecentEthereum
{


class HostBlockService :
	public std::enable_shared_from_this<HostBlockService>
{
public: // static members:

	static std::shared_ptr<HostBlockService> Create(
		const std::string& gethUrl
	)
	{
		return std::shared_ptr<HostBlockService>(
			new HostBlockService(gethUrl)
		);
	}

private: // Constructor - not allowed to be called directly

	HostBlockService(
		const std::string& gethUrl
	) :
		m_gethReq(gethUrl),
		m_blockReceiver(),
		//m_isUpdSvcStarted(false),
		m_currBlockNum(0)
	{}

public:

	~HostBlockService() = default;

	size_t GetCurrBlockNum() const
	{
		return m_currBlockNum;
	}

	void SetUpdSvcStartBlock(size_t startBlockNum)
	{
		m_currBlockNum = startBlockNum;
	}

	// bool GetIsUpdSvcStarted() const
	// {
	// 	return m_isUpdSvcStarted;
	// }

	std::shared_ptr<HostBlockService> GetSharedPtr()
	{
		return shared_from_this();
	}

	/**
	 * @brief Bind a BlockReceiver to this HostBlockService.
	 *        NOTE: it's a 1:1 binding, so the previous BlockReceiver will be
	 *        replaced by the new one, if there is any.
	 *
	 * @param blockReceiver The new BlockReceiver to bind.
	 */
	void BindReceiver(std::shared_ptr<BlockReceiver> blockReceiver)
	{
		m_blockReceiver = blockReceiver;
	}

	bool TryPushNewBlock()
	{
		// if (!m_isUpdSvcStarted)
		// {
		// 	throw std::runtime_error(
		// 		"HostBlockService - BlockUpdateService is not started"
		// 	);
		// }
		try
		{
			auto headerRlp = m_gethReq.GetHeaderRlpByNum(m_currBlockNum);
			++m_currBlockNum;

			std::shared_ptr<BlockReceiver> blockReceiver =
				m_blockReceiver.lock();
			if (blockReceiver == nullptr)
			{
				throw std::runtime_error(
					"HostBlockService - BlockReceiver is not available"
				);
			}
			else
			{
				blockReceiver->RecvBlock(headerRlp);
			}

			return true;
		}
		catch(const std::exception& e)
		{
			return false;
		}
	}

	template<typename _RetType>
	_RetType GetReceiptsRlpByNum(
		uint64_t blockNum
	) const
	{
		return m_gethReq.GetReceiptsRlpByNum<_RetType>(blockNum);
	}


	uint64_t GetLatestBlockNum() const
	{
		auto hdrRlp = m_gethReq.GetHeaderRlpByParam("latest");
		auto hdr = SimpleRlp::EthHeaderParser().Parse(hdrRlp);
		return
			EclipseMonitor::Eth::BlkNumTypeTrait::FromBytes(hdr.get_Number());
	}


private:
	GethRequester m_gethReq;
	std::weak_ptr<BlockReceiver> m_blockReceiver;
	//std::atomic_bool m_isUpdSvcStarted;
	std::atomic_size_t m_currBlockNum;

}; // class HostBlockService


class BlockUpdatorServiceTask :
	public SimpleConcurrency::Threading::TickingTask<int64_t>
{
public: // static members:

	using Base = SimpleConcurrency::Threading::TickingTask<int64_t>;

	static constexpr int64_t sk_taskUpdIntervalMliSec = 200;

public:
	BlockUpdatorServiceTask(
		std::shared_ptr<HostBlockService> blockUpdator,
		int64_t retryIntervalMilSec
	) :
		Base(),
		m_blockUpdator(blockUpdator),
		m_retryIntervalMilSec(retryIntervalMilSec)
	{}

	virtual ~BlockUpdatorServiceTask() = default;


protected:

	virtual void Tick() override
	{
		auto blockUpdator = m_blockUpdator.lock();
		if (blockUpdator)
		{
			if (blockUpdator->TryPushNewBlock())
			{
				// Successfully pushed a new block to the enclave
				// keep pushing without delay
				if (Base::IsTickIntervalEnabled())
				{
					Base::DisableTickInterval();
				}
			}
			else
			{
				// Failed to push a new block to the enclave
				// wait for a while before retry
				if (!Base::IsTickIntervalEnabled())
				{
					Base::SetInterval(
						sk_taskUpdIntervalMliSec,
						m_retryIntervalMilSec
					);
				}
			}
		}
		else
		{
			throw std::runtime_error(
				"BlockUpdatorServiceTask - HostBlockService is not available"
			);
		}
	}


	virtual void SleepFor(int64_t mliSec) const override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(mliSec));
	}


private:
	std::weak_ptr<HostBlockService> m_blockUpdator;
	int64_t m_retryIntervalMilSec;

}; // class BlockUpdatorServiceTask


class HostBlockStatusLogTask :
	public SimpleConcurrency::Threading::TickingTask<int64_t>
{
public: // static members:

	using Base = SimpleConcurrency::Threading::TickingTask<int64_t>;

	static constexpr int64_t sk_taskUpdIntervalMliSec = 200;

public:

	HostBlockStatusLogTask(
		std::shared_ptr<HostBlockService> blockUpdator,
		int64_t updIntervalMliSec
	) :
		Base(sk_taskUpdIntervalMliSec, updIntervalMliSec),
		m_blockUpdator(blockUpdator),
		m_lastBlockNum(0),
		m_updIntervalSec(updIntervalMliSec / 1000.0)
	{}

	virtual ~HostBlockStatusLogTask() = default;


protected:

	virtual void Tick() override
	{
		auto blockUpdator = m_blockUpdator.lock();
		if (blockUpdator)
		{
			const size_t currBlockNum = blockUpdator->GetCurrBlockNum();

			size_t diff = currBlockNum - m_lastBlockNum;
			float rate = diff / m_updIntervalSec;

			std::cout << "HostBlockServiceStatus: " <<
				"BlockNum=" << currBlockNum << ", " <<
				"Rate=" << rate << " blocks/sec" <<
				std::endl;

			m_lastBlockNum = currBlockNum;
		}
		else
		{
			throw std::runtime_error(
				"HostBlockStatusLogTask - HostBlockService is not available"
			);
		}
	}


	virtual void SleepFor(int64_t mliSec) const override
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(mliSec));
	}


private:
	std::weak_ptr<HostBlockService> m_blockUpdator;
	size_t m_lastBlockNum;
	float m_updIntervalSec;
}; // class HostBlockStatusLogTask


} // namespace DecentEthereum
