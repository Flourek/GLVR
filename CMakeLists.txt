cmake_minimum_required( VERSION 3.6 )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )



project( GLVR )

find_package( OpenCV REQUIRED)
find_package( OpenGL REQUIRED )
include_directories(${OpenCV_INCLUDE_DIRS})

set( LIBS_DIR ${CMAKE_CURRENT_LIST_DIR}/libs)

set ( project_files
        main.cpp
        shader.h
        camera.h
        ${LIBS_DIR}/glad/src/glad.c
)


include_directories(  ${LIBS_DIR}/glew/include )

set( IMGUI_DIR ${LIBS_DIR}/imgui )

set( imgui_files
       ${LIBS_DIR}/imgui/imconfig.h
       ${LIBS_DIR}/imgui/imgui.cpp
       ${LIBS_DIR}/imgui/imgui.h
       ${LIBS_DIR}/imgui/imgui_demo.cpp
       ${LIBS_DIR}/imgui/imgui_draw.cpp
       ${LIBS_DIR}/imgui/imgui_internal.h
       ${LIBS_DIR}/imgui/imgui_tables.cpp
       ${LIBS_DIR}/imgui/imgui_widgets.cpp
       ${LIBS_DIR}/imgui/imstb_rectpack.h
       ${LIBS_DIR}/imgui/imstb_textedit.h
       ${LIBS_DIR}/imgui/imstb_truetype.h
       ${LIBS_DIR}/imgui/misc/cpp/imgui_stdlib.h
       ${LIBS_DIR}/imgui/misc/cpp/imgui_stdlib.cpp
)

set( imgui_impl_files
        ${LIBS_DIR}/imgui/backends/imgui_impl_glfw.h
        ${LIBS_DIR}/imgui/backends/imgui_impl_glfw.cpp
        ${LIBS_DIR}/imgui/backends/imgui_impl_opengl3.h
        ${LIBS_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

include_directories( ${IMGUI_DIR} )
include_directories( ${IMGUI_DIR}/backends )
include_directories( ${IMGUI_DIR}/examples )

add_executable( GLVR
        ${project_files}
        ${imgui_files}
        ${imgui_impl_files}
)

message(w ${OpenCV_DIR})
target_link_libraries( GLVR ${OpenCV_DIR}/x64/vc16/lib/opencv_world470.lib  )


# OpenVR
set(OPENVR_DIR ${LIBS_DIR}/openvr)
include_directories(${OPENVR_DIR}/headers)
target_link_libraries(GLVR ${OPENVR_DIR}/lib/win64/openvr_api.lib)

# GLFW
set( GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set( GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set( GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${LIBS_DIR}/glfw)
target_link_libraries( GLVR glfw)
target_link_libraries( GLVR OpenGL::GL)
include_directories( ${LIBS_DIR}/glfw/include)

include_directories(${LIBS_DIR}/glm)
include_directories(${LIBS_DIR}/glad/include)
include_directories( ${OPENGL_INCLUDE_DIR})
target_link_libraries( GLVR ${OPENGL_LIBRARIES} )



function(copy_file file)
    add_custom_command(
            TARGET GLVR PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/${file}
            ${CMAKE_CURRENT_BINARY_DIR}/${file}
    )
endfunction()

copy_file("openvr_api.dll")
copy_file("camera.vs")
copy_file("camera.fs")
