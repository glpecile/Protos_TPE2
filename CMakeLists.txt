cmake_minimum_required(VERSION 3.16)
project(Protos-TP2E C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS    " -pthread --std=c99 -fsanitize=address -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L")

set(SOURCE_FILES    src/args.c      src/buffer.c        src/hello.c      src/logger.c        src/netutils.c      src/parser.c        src/parser_utils.c      src/request.c       src/selector.c      src/socks_handler.c     src/socks_nio.c     src/stm.c
                    include/args.h  include/buffer.h    include/hello.h  include/logger.h    include/netutils.h  include/parser.h    include/parser_utils.h  include/request.h   include/selector.h  include/socks_handler.h include/socks_nio.h include/stm.h src/copy_events.c include/copy_events.h src/connecting_events.c include/connecting_events.h)

add_executable(Protos-TP2E ${SOURCE_FILES} main.c)
