CCOPTS=-Wall -Wextra -g -pthread
b-client: b-client.c ../b-server/b-protocol.c ../b-server/b-protocol.h ../b-server/b-tools.c ../b-server/b-tools.h
	$(CC) $(CCOPTS) -o $@ $(filter-out %.h,$^)  
clean:
	rm b-client