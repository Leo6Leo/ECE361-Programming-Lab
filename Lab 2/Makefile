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
CC=gcc
all: server deliver haha
server: server.o
deliver: deliver.o
haha: haha.o
clean:
		rm -f *.o server deliver haha