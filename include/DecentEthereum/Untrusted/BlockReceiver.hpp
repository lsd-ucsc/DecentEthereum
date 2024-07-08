// Copyright (c) 2023 Haofan Zheng
// Use of this source code is governed by an MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT.

#pragma once


#include <cstdint>

#include <vector>


namespace DecentEthereum
{
namespace Untrusted
{

class BlockReceiver
{
public:

	BlockReceiver() = default;

	// LCOV_EXCL_START
	virtual ~BlockReceiver() = default;
	// LCOV_EXCL_STOP

	virtual void RecvBlock(const std::vector<uint8_t>& blockRlp) = 0;

};


} // namespace Untrusted
} // namespace DecentEthereum

