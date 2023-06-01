cmake_minimum_required(VERSION 3.24)
project(sentrytry)

if(NOT EXISTS ${PROJECT_SOURCE_DIR}/lddpro-deps)
	if(NOT EXISTS ${PROJECT_SOURCE_DIR}/package.zip)
		# Download lddpro-deps
		if(APPLE)
		    find_program(uname_program uname REQUIRED)
		    execute_process(COMMAND ${uname_program} -m
		        OUTPUT_VARIABLE uname_output
		        COMMAND_ERROR_IS_FATAL ANY)
		   	string(STRIP "${uname_output}" uname_output)
		   	if(uname_output STREQUAL "x86_64")
				set(lddproDepsProfile macos-x86_64-appleclang14)
		   	elseif(uname_output STREQUAL "arm64")
				set(lddproDepsProfile macos-armv8-appleclang14)
		   	else()
		        message(FATAL_ERROR "Unexpected architecture: ${uname_output}")
		    endif()
		elseif(WIN32)
			set(lddproDepsProfile windows-x86_64-vs17-mt)
		else()
			message(FATAL_ERROR "Platform not supported.")
		endif()

		set(lddproDepsTag 20230531)

		set(url https://gitlab.legogroup.io/api/v4/projects/5072/packages/generic/${lddproDepsProfile}/${lddproDepsTag}/${lddproDepsProfile}-${lddproDepsTag}.zip)
		message(STATUS "Downloading ${url}")
		file(REMOVE_RECURSE ${PROJECT_SOURCE_DIR}/package)
		file(DOWNLOAD
			"${url}"
			${PROJECT_SOURCE_DIR}/package.zip
			USERPWD gitlab-read-token:QuNim5V_MzBesqvEBr2V
			SHOW_PROGRESS
			STATUS download_result
		)

		list(GET download_result 0 download_result_error_code)
		if(NOT (download_result_error_code STREQUAL "0"))
			list(JOIN download_result " - " download_result)
			message(FATAL_ERROR "ERROR: " ${download_result})
		endif()
	endif()

	file(ARCHIVE_EXTRACT INPUT ${PROJECT_SOURCE_DIR}/package.zip DESTINATION ${PROJECT_SOURCE_DIR})
	file(REMOVE ${PROJECT_SOURCE_DIR}/package.zip)
	file(RENAME ${PROJECT_SOURCE_DIR}/package ${PROJECT_SOURCE_DIR}/lddpro-deps)
endif()

list(APPEND CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/lddpro-deps)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG 1)

find_package(Threads REQUIRED)
find_package(sentry REQUIRED)

if(XCODE)
    set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
endif()

add_executable(sentrytry main.cpp SentryClient.cpp SentryClient.h)
target_link_libraries(sentrytry PRIVATE sentry::sentry)

find_program(crashpad_handler_program crashpad_handler
	PATHS ${CMAKE_PREFIX_PATH}/bin
	REQUIRED)

set(sentry_database_parent_path ${PROJECT_SOURCE_DIR}/sentry_database)

add_compile_definitions(
	CRASHPAD_HANDLER_PROGRAM=\"${crashpad_handler_program}\"
	SENTRY_DATABASE_PARENT_PATH=\"${sentry_database_parent_path}\"
)
