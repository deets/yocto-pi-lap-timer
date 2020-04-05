### Project info
cmake_minimum_required(VERSION 3.15)

project(spi-daemon)

include_directories(spi-daemon PRIVATE include src)

add_executable(
  spi-daemon
  src/arguments.cpp
  src/datagram.cpp
  src/parser.cpp
  src/realtime.cpp
  src/spi-daemon.cpp
  src/tx.cpp
  )


target_compile_features(spi-daemon PRIVATE cxx_std_17)
target_link_libraries(spi-daemon PRIVATE nanomsg pthread)
target_compile_options(spi-daemon PRIVATE -fconcepts)

install(TARGETS spi-daemon RUNTIME DESTINATION ${DESTDIR}/usr/bin)

add_executable(
  test-spi-daemon
  src/parser.cpp
  tests/tst_parser.cpp
  )

target_compile_features(test-spi-daemon PRIVATE cxx_std_17)
target_compile_options(test-spi-daemon PRIVATE -fconcepts)

install(TARGETS test-spi-daemon RUNTIME DESTINATION ${DESTDIR}/usr/bin)
