all: server subscriber

# Compileaza server.cpp
server: server.cpp
	g++ server.cpp -Wall -o server
# Compileaza subscriber.cpp
subscriber: subscriber.cpp
	g++ subscriber.cpp -Wall -o subscriber

.PHONY: clean 

clean:
	rm -f server subscriber
