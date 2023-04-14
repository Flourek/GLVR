# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src/glfw"
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src/glfw-build"
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw"
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/tmp"
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src/glfw-stamp"
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src"
  "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src/glfw-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src/glfw-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/Flourek/CLionProjects/Stereorizer/libs/glfw/src/glfw-stamp${cfgdir}") # cfgdir has leading slash
endif()
