# CC=gcc
# OBJS = deliver.o haha.o
# TARGETS = main
# $(TARGETS): $(OBJS)
# 	$(CC) $^ -o $@

# %.o: %.c
# 	$(CC) -c $< 


# clearn:
# 	rm -f *.o $(TARGETS)

# CC=gcc
CC=gcc -g -pthread
all: clean server client
server: server.o
clean:
		rm -f *.o server utility client
rebuild: clean all