CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -Isrc
SRC_DIR  = src

all: server client

server: $(SRC_DIR)/server.cpp $(SRC_DIR)/thread_pool.hpp $(SRC_DIR)/shared_state.hpp $(SRC_DIR)/protocol.hpp
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/server.cpp -o server

client: $(SRC_DIR)/client.cpp $(SRC_DIR)/protocol.hpp
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/client.cpp -o client

clean:
	rm -f server client

run: server
	./server
