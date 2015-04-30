#Project Name
PRJ_NAME = SharedPtr

#Target Name of the prj
TARGET = SharedPtr

#Objects of project
OBJS = \
  SharedPtr_test.o

#Source files
SRC = \
  SharedPtr.hpp\
  SharedPtr.cpp

.PHONY: clean debug pack clang

CC = g++
CPPFLAGS = -g -Wall -std=c++11 -Wextra -pedantic
LDFLAGS= -pthread

#%.o: %.cpp %.hpp
#	$(CC) $(CPPFLAGS) $(LDFLAGS) -c $< -o $@

#build the prj
$(TARGET):  SharedPtr_test.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< -o $@

#build the debug enabled object
debug: CPPFLAGS = -g -Wall -std=c++11 -DDEBUG
debug: SharedPtr_test.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< -o $(TARGET)

#build the using clang++
clang: CC = clang++
clang: SharedPtr_test.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) $< -o $(TARGET)

#clean the objects
clean:
	rm -rf $(OBJS) $(TARGET) $(PRJ_NAME).tar.gz *~

