CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -I$(CURDIR)/include

TARGET := log_manager_demo

SRCS := src/log_manager.cpp \
        demo/main.cpp

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -f log_manager_demo.log