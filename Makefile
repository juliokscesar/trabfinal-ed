all:
		mkdir -p build
		gcc -O2 -o build/proj src/main.c src/config.c src/markov.c src/utils.c src/logging.c src/markovgraph.c src/markovnetwork.c ext/inih/ini.c -Isrc/ -Iext/inih -lm
