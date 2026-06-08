CXX = g++
CXXFLAGS = -std=c++11 -pthread
TARGET = studyos

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.txt

install:
	sudo cp $(TARGET) /usr/local/bin/

help:
	@echo "Commands: make, make run, make clean, make install"

.PHONY: all run clean install help