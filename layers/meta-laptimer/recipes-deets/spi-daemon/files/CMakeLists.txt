### Project info
cmake_minimum_required(VERSION 3.15)

project(spi-daemon)

add_executable(
  spi-daemon
  src/spi-daemon.cpp
  src/tx.cpp
  src/datagram.cpp
  )

target_compile_features(spi-daemon PRIVATE cxx_std_17)
target_link_libraries(spi-daemon PRIVATE nanomsg pthread)
target_compile_options(spi-daemon PRIVATE -fconcepts)
