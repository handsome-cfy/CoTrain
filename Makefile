CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -pthread

SRCS = main.cpp log.cpp thread.cpp socket.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = program

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)