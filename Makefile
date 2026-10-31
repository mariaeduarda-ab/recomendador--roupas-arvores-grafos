CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -MMD -MP

BUILD := build
FONTES := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
OBJETOS := $(FONTES:src/%.cpp=$(BUILD)/%.o)

all: recomendador

recomendador: $(OBJETOS) $(BUILD)/main.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/testes: $(OBJETOS) $(BUILD)/testes.o
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/testes.o: testes/testes.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -Isrc -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

test: $(BUILD)/testes
	./$(BUILD)/testes

clean:
	rm -rf $(BUILD) recomendador

.PHONY: all test clean

-include $(wildcard $(BUILD)/*.d)
