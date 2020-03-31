### Project info
cmake_minimum_required(VERSION 3.15)

project(spi-daemon)

add_executable(
  spi-daemon
  src/datagram.cpp
  src/realtime.cpp
  src/spi-daemon.cpp
  src/tx.cpp
  )

target_compile_features(spi-daemon PRIVATE cxx_std_17)
target_link_libraries(spi-daemon PRIVATE nanomsg pthread)
target_compile_options(spi-daemon PRIVATE -fconcepts)

install(TARGETS spi-daemon RUNTIME DESTINATION ${DESTDIR}/usr/bin)
