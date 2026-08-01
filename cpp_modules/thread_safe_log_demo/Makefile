CXX := g++

MODULE_ROOT := ..

CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pthread \
            -I$(MODULE_ROOT)/log_manager/include

TARGET := thread_safe_log_demo

SRCS := main.cpp \
        $(MODULE_ROOT)/log_manager/src/log_manager.cpp

HEADERS := $(MODULE_ROOT)/log_manager/include/log_manager.h

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f thread_safe_log_demo.log
	rm -f *.o