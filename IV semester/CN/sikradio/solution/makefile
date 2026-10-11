CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
LDFLAGS = -lssl -lcrypto

SIKRADIO = sikradio
COMMON= err.cpp network.cpp

SIKRADIO_OBJ = sikradio.o $(COMMON_OBJ)
COMMON_OBJ = err.o network.o

.PHONY: all clean

all: $(SIKRADIO)

$(SIKRADIO): $(SIKRADIO_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SIKRADIO) *.o *~

