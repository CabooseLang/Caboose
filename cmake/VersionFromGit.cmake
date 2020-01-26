function(version_from_git)
    execute_process(
            COMMAND git rev-list --tags --max-count=1
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE git_result
            OUTPUT_VARIABLE git_rev
            ERROR_VARIABLE git_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
            COMMAND git describe --tags ${git_rev}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE git_result
            OUTPUT_VARIABLE git_rev_tag
            ERROR_VARIABLE git_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
    )

    if (NOT git_result EQUAL 0)
        message(FATAL_ERROR "Failed to execute Git: ${git_error}")
    endif()

    if(NOT "$ENV{GIT_TAG}" STREQUAL "")
        set(git_rev_tag, "$ENV{GIT_TAG}")
    endif()

    set(git_resolved_version ${git_rev_tag} PARENT_SCOPE)
endfunction(version_from_git)
