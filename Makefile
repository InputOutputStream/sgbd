SRC = src
INCLUDE = include
OBJ = obj
BIN = bin
TARGET = lightbd

LIB_SRC = $(wildcard $(SRC)/*.cpp)

LIB_OBJ = $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(LIB_SRC))

CC = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -g -I$(INCLUDE)

all: directories compile

run: all
	./$(BIN)/$(TARGET)

compile: $(LIB_OBJ)
	$(CC) -o $(BIN)/$(TARGET) $(LIB_OBJ) $(CDFLAGS)

directories:
	mkdir -p $(OBJ) $(BIN)

$(OBJ)/%.o: $(SRC)/%.cpp #$(INCLUDE)
	$(CC) $(CXXFLAGS) -c $< -o $@

$(OBJ)/%.o: $(TEST)/%.cpp #$(INCLUDE)
	$(CC) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ) $(BIN)

install:
	make
	cp $(BIN)/lightbd.out /usr/local/bin/lightbd
	
uninstall:
	rm -f $(DESTDIR)/usr/local/bin/lightbd

.PHONY: all directories tests clean install uninstall run-tests
	
