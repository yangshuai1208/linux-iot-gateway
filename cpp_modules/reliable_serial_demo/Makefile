CXX := g++

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror \
            -Iinclude

TARGET := reliable_serial_demo

SRCS := demo/main.cpp \
        src/reliable_protocol.cpp

HEADERS := include/reliable_protocol.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o