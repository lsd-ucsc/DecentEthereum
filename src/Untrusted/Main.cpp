// Copyright (c) 2022 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.


#include <memory>
#include <vector>

#include <DecentEnclave/Common/Platform/Print.hpp>
#include <DecentEnclave/Common/Sgx/MbedTlsInit.hpp>

#include <DecentEnclave/Untrusted/Config/AuthList.hpp>
#include <DecentEnclave/Untrusted/Config/EndpointsMgr.hpp>
#include <DecentEnclave/Untrusted/Hosting/BoostAsioService.hpp>
#include <DecentEnclave/Untrusted/Hosting/HeartbeatEmitterService.hpp>
#include <DecentEnclave/Untrusted/Hosting/LambdaFuncServer.hpp>

#include <DecentEthereum/Untrusted/HostBlockServiceTasks.hpp>

#include <SimpleConcurrency/Threading/ThreadPool.hpp>
#include <SimpleJson/SimpleJson.hpp>
#include <SimpleRlp/SimpleRlp.hpp>
#include <SimpleObjects/Internal/make_unique.hpp>
#include <SimpleObjects/SimpleObjects.hpp>
#include <SimpleSysIO/SysCall/Files.hpp>

#include "DecentEthereumEnclave.hpp"
#include "RunUntilSignal.hpp"


using namespace DecentEnclave;
using namespace DecentEnclave::Common;
using namespace DecentEnclave::Untrusted;
using namespace DecentEthereum;
using namespace SimpleConcurrency::Threading;
using namespace SimpleSysIO::SysCall;


std::shared_ptr<ThreadPool> GetThreadPool()
{
	static  std::shared_ptr<ThreadPool> threadPool =
		std::make_shared<ThreadPool>(5);

	return threadPool;
}


static void StartSendingBlocks(
	HostBlockService& blkSvc,
	uint64_t startBlockNum
)
{
	if (blkSvc.GetCurrBlockNum() != 0)
	{
		throw std::runtime_error("Block update service already started.");
	}

	blkSvc.SetUpdSvcStartBlock(startBlockNum);
	auto blkSvcSPtr = blkSvc.GetSharedPtr();

	auto blkUpdStatusSvc = std::unique_ptr<HostBlockStatusLogTask>(
		new HostBlockStatusLogTask(blkSvcSPtr, 10 * 1000)
	);
	auto blkUpdSvc = std::unique_ptr<BlockUpdatorServiceTask>(
		new BlockUpdatorServiceTask(blkSvcSPtr, 1 * 1000)
	);

	std::shared_ptr<ThreadPool> threadPool = GetThreadPool();

	threadPool->AddTask(std::move(blkUpdStatusSvc));
	threadPool->AddTask(std::move(blkUpdSvc));
}


int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	// Init MbedTLS
	Common::Sgx::MbedTlsInit::Init();


	// Thread pool
	std::shared_ptr<ThreadPool> threadPool = GetThreadPool();


	// Read in components config
	auto configFile = RBinaryFile::Open(
		"../../src/components_config.json"
	);
	auto configJson = configFile->ReadBytes<std::string>();
	auto config = SimpleJson::LoadStr(configJson);
	std::vector<uint8_t> authListAdvRlp = Config::ConfigToAuthListAdvRlp(config);


	// Boost IO Service
	std::unique_ptr<Hosting::BoostAsioService> asioService =
		SimpleObjects::Internal::make_unique<Hosting::BoostAsioService>();


	// Endpoints Manager
	auto endpointMgr = Config::EndpointsMgr::GetInstancePtr(
		&config,
		asioService->GetIoService()
	);


	// Host block service
	std::shared_ptr<HostBlockService> hostBlkSvc = HostBlockService::Create(
		"http://localhost:8545"
	);


	// Sync Event Contract Address
	const EclipseMonitor::Eth::ContractAddr decentSyncV2Addr = {
		0X74U, 0XBEU, 0X86U, 0X7FU, 0XBDU, 0X89U, 0XBCU, 0X35U,
		0X07U, 0XF1U, 0X45U, 0XB3U, 0X6BU, 0XA7U, 0X6CU, 0XD0U,
		0XB1U, 0XBFU, 0X4FU, 0X1AU,
	};

	// Enclave
	// uint64_t startBlockNum = 8620000;
	// uint64_t startBlockNum = 0;
	// uint64_t startBlockNum = 8814199;
	uint64_t startBlockNum = 8875000;
	std::shared_ptr<DecentEthereumEnclave> enclave =
		std::make_shared<DecentEthereumEnclave>(
			EclipseMonitor::BuildEthereumMonitorConfig(),
			startBlockNum,
			decentSyncV2Addr,
			"SyncMsg(bytes16,bytes32)",
			hostBlkSvc,
			authListAdvRlp
		);
	hostBlkSvc->BindReceiver(enclave);
	StartSendingBlocks(*hostBlkSvc, startBlockNum);


	// API call server
	Hosting::LambdaFuncServer lambdaFuncSvr(
		endpointMgr,
		threadPool
	);
	// Setup Lambda call handlers and start to run multi-threaded-ly
	lambdaFuncSvr.AddFunction("DecentEthereum", enclave);


	// Heartbeat emitter
	auto heartbeatEmitter = std::unique_ptr<Hosting::HeartbeatEmitterService>(
		new Hosting::HeartbeatEmitterService(enclave, 2000)
	);
	threadPool->AddTask(std::move(heartbeatEmitter));


	// Start IO service
	threadPool->AddTask(std::move(asioService));


	RunUntilSignal(
		[&]()
		{
			threadPool->Update();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	);


	threadPool->Terminate();


	return 0;
}


extern "C" sgx_status_t ocall_decent_ethereum_get_receipts(
	const void* host_blk_svc,
	uint64_t blk_num,
	uint8_t** out_buf,
	size_t* out_buf_size
)
{
	using _ListBytesType = SimpleObjects::ListT<SimpleObjects::Bytes>;

	const HostBlockService* blkSvc =
		static_cast<const HostBlockService*>(host_blk_svc);

	try
	{
		_ListBytesType listOfReceipts =
			blkSvc->GetReceiptsRlpByNum<_ListBytesType>(blk_num);

		std::vector<uint8_t> bytes = SimpleRlp::WriteRlp(listOfReceipts);

		*out_buf = new uint8_t[bytes.size()];
		*out_buf_size = bytes.size();

		std::copy(bytes.begin(), bytes.end(), *out_buf);

		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_decent_ethereum_get_receipts failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}

extern "C" sgx_status_t ocall_decent_ethereum_get_latest_blknum(
	const void* host_blk_svc,
	uint64_t* out_blk_num
)
{
	const HostBlockService* blkSvc =
		static_cast<const HostBlockService*>(host_blk_svc);

	try
	{
		*out_blk_num = blkSvc->GetLatestBlockNum();

		return SGX_SUCCESS;
	}
	catch (const std::exception& e)
	{
		DecentEnclave::Common::Platform::Print::StrDebug(
			"ocall_decent_ethereum_get_latest_blknum failed with error " +
			std::string(e.what())
		);
		return SGX_ERROR_UNEXPECTED;
	}
}
