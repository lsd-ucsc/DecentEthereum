// Copyright (c) 2023 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <DecentEnclave/Trusted/DecentLambdaSvr.hpp>


namespace DecentEthereum
{
namespace Trusted
{


using LambdaMsgSocket =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::SocketType;
using LambdaMsgSocketPtr =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::SocketPtrType;
using LambdaMsgIdExt =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::MsgIdExtType;
using LambdaMsgContent =
	typename DecentEnclave::Trusted::LambdaHandlerMgr::MsgContentType;


} // namespace Trusted
} // namespace DecentEthereum
