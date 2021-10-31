.PHONY : all clean test cleanTest server
COMPILER=gcc
CFLAGS = --std=c11 -fsanitize=address -pedantic -pedantic-errors -Wall -Wextra -Werror -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L -L /lib64 -l pthread
all: main
clean:
	- rm -f *.o  server

COMMON =  ./src/args.c ./src/buffer.c ./src/hello.c ./src/netutils.c ./src/parser.c ./src/parser_utils.c ./src/request.c ./src/selector.c ./src/stm.c
main:
	$(COMPILER) $(CFLAGS) -o main ./main.c $(COMMON)

test: clean all
	mkdir tests; valgrind --leak-check=full -v ./main 2>> tests/results.valgrind; cppcheck --quiet --enable=all --force --inconclusive main.c 2>> tests/output.cppOut

cleanTest:
	rm -rf tests/output.cppOut tests/report.tasks tests/results.valgrind tests