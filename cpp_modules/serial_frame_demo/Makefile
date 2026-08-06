CXX := g++

MODULE_ROOT := ..

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror \
            -Iinclude \
            -I$(MODULE_ROOT)/protocol_parser/include

TARGET := serial_frame_demo

SRCS := demo/main.cpp \
        src/serial_frame.cpp \
        $(MODULE_ROOT)/protocol_parser/src/protocol_parser.cpp

HEADERS := include/serial_frame.h \
           $(MODULE_ROOT)/protocol_parser/include/protocol_parser.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o