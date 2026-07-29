CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -I$(CURDIR)/include

TARGET := protocol_parser_demo

SRCS := src/protocol_parser.cpp \
        demo/main.cpp

HEADERS := include/protocol_parser.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o