# Copyright (c) 2022 Haofan Zheng
# Use of this source code is governed by an MIT-style
# license that can be found in the LICENSE file or at
# https://opensource.org/licenses/MIT.


cmake_minimum_required(VERSION 3.18)

project(DecentEthereum LANGUAGES C CXX VERSION 0.0.1)


################################################################################
# Set compile options
################################################################################

if(MSVC)
	set(COMMON_OPTIONS /W4 /WX /EHsc /MP /GR /Zc:__cplusplus
		/wd4815 # Mitigate issue in SGXSDK about zero-sized array
		/wd4310 # Mitigate issue in SGXSDK about cast truncates in #include <limits>
	)
	set(DEBUG_OPTIONS /MTd /Od /Zi /DDEBUG)
	set(RELEASE_OPTIONS /MT /Ox /Oi /Ob2 /fp:fast)# /DNDEBUG
#	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG)
else()
	set(COMMON_OPTIONS -pthread -Wall -Wextra -Werror)
	set(DEBUG_OPTIONS -O0 -g -DDEBUG)
	set(RELEASE_OPTIONS -O2) #-DNDEBUG defined by default
endif()

set(DEBUG_OPTIONS ${COMMON_OPTIONS} ${DEBUG_OPTIONS})
set(RELEASE_OPTIONS ${COMMON_OPTIONS} ${RELEASE_OPTIONS})

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
endif()

################################################################################
# Setup CMake environment for enclave targets
################################################################################

#Removed Basic Runtime Checks in MSVC
if(MSVC)
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
	STRING (REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
endif()

#Remove all standard libraries dependency here so that enclave DLL can be
# compiled properly. And it will be added back later for non-enclave apps.
set(UNTRUSTED_CXX_STANDARD_LIBRARIES ${CMAKE_CXX_STANDARD_LIBRARIES_INIT})
set(UNTRUSTED_C_STANDARD_LIBRARIES ${CMAKE_C_STANDARD_LIBRARIES_INIT})
set(CMAKE_CXX_STANDARD_LIBRARIES "")
set(CMAKE_C_STANDARD_LIBRARIES "")

# Add DebugSimulation to CMake configuration types
set(CMAKE_CONFIGURATION_TYPES Release Debug DebugSimulation)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug DebugSimulation)

set(CMAKE_CXX_FLAGS_DEBUGSIMULATION           ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_DEBUGSIMULATION             ${CMAKE_C_FLAGS_DEBUG})
set(CMAKE_EXE_LINKER_FLAGS_DEBUGSIMULATION    ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
set(CMAKE_SHARED_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_SHARED_LINKER_FLAGS_DEBUG})
set(CMAKE_STATIC_LINKER_FLAGS_DEBUGSIMULATION ${CMAKE_STATIC_LINKER_FLAGS_DEBUG})

################################################################################
# Fetch external dependencies
################################################################################

include(FetchContent)

## SimpleCMakeScripts
FetchContent_Declare(
	git_simplecmakescripts
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleCMakeScripts.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simplecmakescripts)
simplecmakescripts_enable()

## SimpleUtf
FetchContent_Declare(
	git_simpleutf
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleUtf.git
	GIT_TAG        main
)
FetchContent_MakeAvailable(git_simpleutf)

## SimpleObjects
FetchContent_Declare(
	git_simpleobjects
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleObjects.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simpleobjects)

## SimpleRlp
FetchContent_Declare(
	git_simplerlp
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleRlp.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simplerlp)

## SimpleJson
FetchContent_Declare(
	git_simplejson
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleJson.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simplejson)

## SimpleSysIO
FetchContent_Declare(
	git_simplesysio
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleSysIO.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simplesysio)

## SimpleConcurrency
FetchContent_Declare(
	git_simpleconcurrency
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleConcurrency.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simpleconcurrency)

## SimpleBoostSelector
set(SIMPLEBOOST_BOOST_VER 1.81.0 CACHE STRING "Boost version to use." FORCE)
FetchContent_Declare(
	git_simpleboostselector
	GIT_REPOSITORY https://github.com/zhenghaven/SimpleBoostSelector.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_simpleboostselector)
simpleboost_enable_sublib(asio)

## EclipseMonitor
FetchContent_Declare(
	git_eclipsemonitor
	GIT_REPOSITORY https://github.com/zhenghaven/EclipseMonitor.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_eclipsemonitor)

## DecentEnclave
FetchContent_Declare(
	git_decentenclave
	GIT_REPOSITORY https://github.com/zhenghaven/DecentEnclave.git
	GIT_TAG        dev
)
FetchContent_MakeAvailable(git_decentenclave)

## mbedTLScpp
FetchContent_Declare(
  git_mbedtlscpp
  GIT_REPOSITORY https://github.com/zhenghaven/mbedTLScpp.git
  GIT_TAG        dev
)
FetchContent_MakeAvailable(git_mbedtlscpp)

## MbedTLS
set(ENABLE_TESTING OFF CACHE BOOL "Build mbed TLS tests." FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build mbed TLS programs." FORCE)
if (MSVC)
	set(
		MSVC_STATIC_RUNTIME
		ON
		CACHE BOOL
		"Build the libraries with /MT compiler flag"
		FORCE
	)
endif()
FetchContent_Declare(
  git_mbedtls
  GIT_REPOSITORY https://github.com/zhenghaven/mbedtls.git
  GIT_TAG        decent-enclave-v3.5.2
)
FetchContent_MakeAvailable(git_mbedtls)
mbedTLScpp_Decentize_Normal(mbedcrypto)
mbedTLScpp_Decentize_Normal(mbedx509)
mbedTLScpp_Decentize_Normal(mbedtls)
mbedTLScpp_Decentize_Enclave(mbedcrypto_enclave)
mbedTLScpp_Decentize_Enclave(mbedx509_enclave)
mbedTLScpp_Decentize_Enclave(mbedtls_enclave)
set(MBEDTLS_INCLUDE_DIRS ${MBEDTLS_DIR}/include/)
set(MBEDCRYPTO_LIBRARY mbedcrypto)
set(MBEDX509_LIBRARY mbedx509)
set(MBEDTLS_LIBRARY mbedtls)

## curl
include(SimpleCUrlConfig)
simple_curl_https_pre_configuration()
FetchContent_Declare(
	git_curl
	GIT_REPOSITORY https://github.com/curl/curl.git
	GIT_TAG        curl-7_86_0
)
FetchContent_MakeAvailable(git_curl)

################################################################################
# Add source directories
################################################################################

add_subdirectory(src)
add_subdirectory(tests)
