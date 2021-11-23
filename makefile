.PHONY : all clean test cleanTest server
COMPILER=gcc
CFLAGS = --std=c11 -fsanitize=address -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -g
LIBS = -l pthread
all: main
clean:
	- rm -f *.o  main

COMMON =  ./src/args.c ./src/buffer.c ./src/capa_events.c src/connecting_events.c src/copy_events.c ./src/dns_resolution_events.c ./src/greetings_events.c ./src/hello.c ./src/logger.c ./src/netutils.c ./src/parser.c ./src/parser_utils.c ./src/request.c ./src/selector.c ./src/socks_handler.c ./src/socks_nio.c ./src/stm.c ./src/cmd_queue.c
main:
	$(COMPILER) $(CFLAGS) -o main ./main.c $(COMMON) $(LIBS)

test: clean all
	mkdir tests; valgrind --leak-check=full -v ./main 2>> tests/results.valgrind; cppcheck --quiet --enable=all --force --inconclusive main.c 2>> tests/output.cppOut

cleanTest:
	rm -rf tests/output.cppOut tests/report.tasks tests/results.valgrind tests