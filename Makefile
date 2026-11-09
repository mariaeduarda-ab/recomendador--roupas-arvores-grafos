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

# exemplo pequeno com dados/demo (2.891 pecas reais, 826 looks), sem baixar nada.
# as configs sao as escolhidas na validacao da base completa
demo: recomendador
	mkdir -p resultados/demo
	cp resultados/config_f.csv resultados/config_d.csv resultados/vetor_d.csv resultados/demo/
	./recomendador --dados dados/demo parecidas 5 3
	./recomendador --dados dados/demo completar 3 shoes
	./recomendador --dados dados/demo avaliar_justo
	./recomendador --dados dados/demo pontos
	./recomendador --dados dados/demo grupos_formas
	./recomendador --dados dados/demo articulacoes
	@echo
	@echo "Tabelas em resultados/demo/. Resumo das versoes:"
	@grep -E "^\| (Aleatória|Popularidade|C|F|C \+ popularidade) \|" resultados/demo/tabela_final.md

.PHONY: all test clean demo tempos_f

-include $(wildcard $(BUILD)/*.d)
