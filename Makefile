CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

TARGET = student_system
SRCS = main.cpp avl_tree.cpp bplus_tree.cpp

all: $(TARGET)

$(TARGET): $(SRCS) student.h avl_tree.h bplus_tree.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
