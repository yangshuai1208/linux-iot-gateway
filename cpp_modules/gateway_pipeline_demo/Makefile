CXX := g++

MODULE_ROOT := ..

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread \
            -I$(MODULE_ROOT)/log_manager/include \
            -I$(MODULE_ROOT)/protocol_parser/include

TARGET := gateway_pipeline_demo

SRCS := main.cpp \
	$(MODULE_ROOT)/log_manager/src/log_manager.cpp \
	$(MODULE_ROOT)/protocol_parser/src/protocol_parser.cpp

HEADERS := $(MODULE_ROOT)/log_manager/include/log_manager.h \
	$(MODULE_ROOT)/protocol_parser/include/protocol_parser.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f gateway_pipeline_demo.log
	rm -f *.o