CXX=g++

CXXFLAGS:=-std=c++17 -Wall -Wextra -Werror -pthread

TARGET:=producer_consumer_demo

SRCS:=main.cpp

.PHONY:all run clean

all:$(TARGET)

$(TARGET):$(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run:$(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o