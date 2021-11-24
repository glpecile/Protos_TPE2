.PHONY : all clean test cleanTest

COMPILER = ${CC}

CFLAGS = --std=c11 -fsanitize=address -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -g

LIBS = -l pthread

all: client main;

clean:
	- rm -f *.o src/*.o pop3filter pop3ctl

COMMON =  ./src/admin.o src/admin_utils.o ./src/args.o ./src/buffer.o ./src/capa_events.o src/connecting_events.o src/copy_events.o ./src/dns_resolution_events.o ./src/greetings_events.o  ./src/logger.o ./src/netutils.o ./src/parser.o ./src/parser_utils.o  ./src/request_events.o ./src/response_events.o ./src/selector.o ./src/socks_handler.o ./src/socks_nio.o ./src/stm.o ./src/cmd_queue.o

main: $(COMMON)
	$(COMPILER) $(CFLAGS) -o pop3filter ./main.c $(COMMON) $(LIBS)
		rm -f src/*.o


client: $(COMMON)
	$(COMPILER) $(CFLAGS) -o pop3ctl ./client/admin_client.c $(LIBS)

test: clean all
	mkdir tests; valgrind --leak-check=full -v ./main 2>> tests/results.valgrind; cppcheck --quiet --enable=all --force --inconclusive main.c 2>> tests/output.cppOut

cleanTest:
	rm -rf tests/output.cppOut ./tests/report.tasks tests/results.valgrind tests