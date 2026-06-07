
LIB_E 		= $(CCODES)/utilitarios-em-c/bin/static
LIB_C 		= $(CCODES)/utilitarios-em-c/bin/shared
HEADERS 		= $(CCODES)/utilitarios-em-c/include
DESTINO 		= lib/
COMPILER		= g++

codigos 		= progresso led monitor
objetos 		= auxiliar progresso led monitor
bibliotecas = libteste libtempo liblegivel libterminal libconversao
cabecalhos	= teste tempo legivel terminal conversao


# Compila tudo que existe(processo demorado).
all: importa-bibliotecas compila-objetos \
	test-progresso test-led test-monitor

limpa-testes:
	rm -fv bin/ut*
	rm -fv bin/tests/ut*

clean: limpa-testes
	rm -frv build/ bin/

cria-dirs-do-projeto:
	@echo "Template de projeto mais comun em projetos:"
	@echo "Talvez alguns sejam desnecessários neste."
	@mkdir -p lib/include
	@mkdir -p bin/tests
	@mkdir -p src
	@mkdir -p include
	@mkdir -p build
	@mkdir -p data
	@mkdir -p bin/demos

# Iteração que copia/ou compila, um processo mais automatizado.
$(bibliotecas):
	@cp -vu $(LIB_E)/$@.a $(DESTINO)

$(cabecalhos):
	@cp --verbose --update $(HEADERS)/$@.h  lib/include

$(objetos):
	@mkdir -vp build/
	@echo "Compilação do objeto de '$@' em 'build'..."
	@clang++ -Ilib/include -std=gnu++17 -c -o build/$@.o src/$@.cpp

importa-bibliotecas-i: $(bibliotecas) $(cabecalhos)
	@echo "Importado bibliotecas estáticas e seus respectivos cabeçalhos."

compila-objetos: $(objetos)
	@echo "Todos objetos foram compilados em 'build'."

compila-testes-unitarios: compila-objetos test-progresso teste-led test-monitor
	@echo "\nCompilando todos testes unitários ...\n"

	clang++ -Wall -O0 -std=gnu++17 -I$(HEADERS) -D__unit_tests__ \
		-c -o build/test-monitor.o src/monitor.cpp

# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
obj-entrada:
	$(COMPILER) -O3 -Oz -g3 -Wall -pedantic -std=gnu++23 \
		-c -o build/entrada.o src/entrada.cpp
entrada-test:
	clang++ -std=c++23 -D__unit_tests__ -g3 -O0 -Wall \
		-c -o build/entrada-test.o src/entrada.cpp
	clang++ -o bin/ut_entrada build/entrada-test.o -lcurses -lc

# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
obj-comunicacao:
	$(COMPILER) -Wall -pedantic -std=gnu++23 \
		-c -o build/comunicacao.o src/comunicacao.cpp

comunicacao-test: entrada-obj
	clang++ -O0 -g3 -Wall -pedantic -std=gnu++23 -D__unit_tests__ \
		-c -o build/comunicacao-test.o src/comunicacao.cpp
	clang++ -o bin/ut_comunicacao build/comunicacao-test.o build/entrada.o \
		-lcurses

obj-painel-debug:
	@$(COMPILER) -g3 -O0 -Wall -c -o build/painel-debug.o src/painel.cpp
	@echo "Objeto do módulo painel gerado em 'build'."
# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
debug: obj-painel-debug
	@$(COMPILER) -D__unit_tests__ -g3 -O0 -Wall \
		-c -o build/painel-debug.o src/painel.cpp
	@echo "Objeto do módulo 'painel' compilado."
	@$(COMPILER) -g3 -O0 -Wall -D__unit_tests__ -D__debug__ \
		-c -o build/main-debug.o src/main.cpp
	@echo "Objeto do 'main' compilado em 'build'."
	$(COMPILER) -O0 -o bin/debug	\
		build/main-debug.o			\
		build/painel-debug.o			\
		build/entrada.o				\
		build/comunicacao.o			\
		-lcurses

release: cria-dirs-do-projeto obj-entrada obj-comunicacao
	@$(COMPILER) -Ilib/ -O3 -Oz -Wall							\
		-c -o build/painel-release.o src/painel.cpp
	@echo "Objeto painel-release criado."
	@$(COMPILER) -Ilib/ -O3 -Oz -Wall -D__unit_tests__	\
		-c -o build/main-release.o src/main.cpp
	@echo "Objeto main-release gerado."
	@$(COMPILER) -Ilib/ -o bin/release	\
		build/main-release.o				\
		build/painel-release.o			\
		build/entrada.o					\
		build/comunicacao.o				\
			-lcurses -L ./lib -llegivel
	@echo "Todos objetos lincados em 'release' e 'painel-de-progressos'."

# Funciona apenas para um sistema Linux, com um sistema gráfico, que tenha 
# o 'mate-terminal' versão 1.26.1.
run-release:
	@mate-terminal --zoom=1.0 --window --command 'bin/release'

# --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --
demonstracoes = verificador_de_injecao injetor_de_entradas

servidor_injetor:
	$(COMPILER) -o bin/demos/$@ src/demo/$@.cpp \
		build/comunicacao.o build/entrada.o -lcurses
cliente-receptor:
	$(COMPILER) -o bin/demos/$@ src/demo/$@.cpp \
		build/comunicacao.o build/entrada.o -lcurses

$(demonstracoes):
	clang++ -o bin/$@ src/demo/$@.cpp src/entrada.cpp -lcurses

demos: $(demonstracoes)
