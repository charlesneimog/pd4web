CXX = g++
CXXFLAGS = -std=c++23 -g -Wmissing-field-initializers

INCLUDES = -I/home/neimog/Documents/Git/pd4web/build/Sources/Pd4Web/nanovg/src

LIBS = -lglfw -lGLEW -lGL -lm -lfontconfig

NANOVG_SRC = /home/neimog/Documents/Git/pd4web/build/Sources/Pd4Web/nanovg/src/nanovg.cpp
NANOVG_OBJ = nanovg.o
NANOVG_LIB = libnanovg.a

SRC = main.cpp
TARGET = nanovg_test

all: $(NANOVG_LIB) $(TARGET)

$(NANOVG_OBJ): $(NANOVG_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(NANOVG_LIB): $(NANOVG_OBJ)
	ar rcs $@ $^

$(TARGET): $(SRC) $(NANOVG_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@ $(LIBS)

clean:
	rm -f $(TARGET) $(NANOVG_OBJ) $(NANOVG_LIB)

