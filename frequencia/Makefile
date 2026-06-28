CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude
TARGET   = freq
SRC      = src/Main.cpp

.PHONY: all clean run help

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET) dictionary all dom-casmurro

help:
	@echo "Comandos disponiveis:"
	@echo "  make        Compila o executavel 'freq'"
	@echo "  make run    Executa benchmark com dom-casmurro"
	@echo "  make clean  Remove o executavel"
