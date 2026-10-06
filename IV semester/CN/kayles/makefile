CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

COMMON = common kayles err
SERVER = kayles_server
CLIENT = kayles_client

COMMON_OBJ = common.o kayles.o err.o
SERVER_OBJ = kayles_server.o $(COMMON_OBJ)
CLIENT_OBJ = kayles_client.o $(COMMON_OBJ)

.PHONY: all clean

all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER) $(CLIENT) *.o *~

