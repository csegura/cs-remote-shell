CCOPTS=-Wall -std=gnu99
b-server: b-server.c b-funcs.c
	$(CC) $(CCOPTS) -g -pthread -o $@ $^  

clean:
	rm b-server