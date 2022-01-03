CCOPTS=-Wall -Wextra -std=gnu99 -g -pthread
b-server: b-server.c b-protocol.c b-protocol.h b-tools.c b-tools.h
	$(CC) $(CCOPTS) -o $@ $(filter-out %.h,$^)  

clean:
	rm b-server