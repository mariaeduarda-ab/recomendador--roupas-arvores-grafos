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

# refaz as medidas de tempo e os graficos. a carga da maquina fica em resultados/tempos_f/carga.csv
tempos_f: recomendador
	nice -n 0 ./recomendador tempos_f
	python3 preprocess/graficos_tempos_f.py

clean:
	rm -rf $(BUILD) recomendador

.PHONY: all test clean tempos_f

-include $(wildcard $(BUILD)/*.d)
