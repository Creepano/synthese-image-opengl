CXX      = g++
CXXFLAGS = -Wall -Wextra -O2
LDLIBS   = -lGL -lGLU -lglut -ljpeg -lm
TARGET   = araignee
SRC      = jpegtex.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) *.o

.PHONY: all run clean
