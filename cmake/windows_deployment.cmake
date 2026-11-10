include(GNUInstallDirs)

if(NOT CMAKE_INSTALL_BINDIR STREQUAL "bin" OR NOT CMAKE_INSTALL_LIBDIR STREQUAL "lib")
    message(FATAL_ERROR "PROCMT_WINDOWS_SPLIT_DEPLOYMENT requires CMAKE_INSTALL_BINDIR=bin and CMAKE_INSTALL_LIBDIR=lib")
endif()

function(procmt_install_windows_executable target)
    if(ARGC GREATER 1 AND ARGV1 STREQUAL "GUI")
        add_executable(${target}_windows_launcher WIN32 "${CMAKE_SOURCE_DIR}/cmake/windows_launcher.cpp")
        target_compile_definitions(${target}_windows_launcher PRIVATE PROCMT_GUI_LAUNCHER)
    else()
        add_executable(${target}_windows_launcher "${CMAKE_SOURCE_DIR}/cmake/windows_launcher.cpp")
    endif()
    set_property(TARGET ${target}_windows_launcher PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    get_target_property(target_sources ${target} SOURCES)
    foreach(source IN LISTS target_sources)
        if(source MATCHES "\\.rc$")
            target_sources(${target}_windows_launcher PRIVATE "${source}")
        endif()
    endforeach()
    install(TARGETS ${target} RUNTIME DESTINATION libexec)
    install(FILES "$<TARGET_FILE:${target}_windows_launcher>"
        DESTINATION "${CMAKE_INSTALL_BINDIR}"
        RENAME "$<TARGET_FILE_NAME:${target}>")
endfunction()

install(FILES "${CMAKE_SOURCE_DIR}/cmake/windows_qt.conf" DESTINATION libexec RENAME qt.conf)
