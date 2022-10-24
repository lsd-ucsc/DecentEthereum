# Copyright (c) 2022 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


include(DecentEnclaveIntelSgx)


decent_enclave_print_config_sgx()


decent_enclave_add_target_sgx(decent_ethereum_client
	UNTRUSTED_SOURCE
		${CMAKE_CURRENT_LIST_DIR}/Untrusted/Main.cpp
		${DECENTENCLAVE_INCLUDE}/DecentEnclave/SgxEdgeSources/SysIO_u.cpp
	UNTRUSTED_DEF
		"SIMPLESYSIO_ENABLE_SYSCALL"
	UNTRUSTED_COMP_OPT
		$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
		$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
		$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	UNTRUSTED_LINK_OPT ""
	UNTRUSTED_LINK_LIB
		SimpleObjects
		SimpleSysIO
		DecentEnclave
	TRUSTED_SOURCE
		${CMAKE_CURRENT_LIST_DIR}/Trusted/Enclave.cpp
	TRUSTED_DEF        ""
	TRUSTED_COMP_OPT
		$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>
		$<$<CONFIG:DebugSimulation>:${DEBUG_OPTIONS}>
		$<$<CONFIG:Release>:${RELEASE_OPTIONS}>
	TRUSTED_LINK_OPT   ""
	TRUSTED_LINK_LIB
		SimpleObjects
		SimpleRlp
		DecentEnclave
		EclipseMonitor
	EDL_PATH
		${CMAKE_CURRENT_LIST_DIR}/Trusted/Enclave.edl
	EDL_INCLUDE
		${DECENTENCLAVE_INCLUDE}
	EDL_OUTPUT_DIR
		${CMAKE_CURRENT_LIST_DIR}
	SIGN_CONFIG
		${CMAKE_CURRENT_LIST_DIR}/Enclave.config.xml
	SIGN_KEY
		${CMAKE_CURRENT_LIST_DIR}/Enclave_private.pem
)
