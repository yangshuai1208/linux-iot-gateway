CXX := g++

MODULE_ROOT := ..

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread \
            -I$(MODULE_ROOT)/log_manager/include \
            -I$(MODULE_ROOT)/protocol_parser/include

TARGET := gateway_timeout_demo

SRCS := main.cpp \
        $(MODULE_ROOT)/log_manager/src/log_manager.cpp \
        $(MODULE_ROOT)/protocol_parser/src/protocol_parser.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f gateway_timeout_demo.log
	rm -f *.o