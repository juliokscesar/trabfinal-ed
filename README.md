# Trabalho Final - Estrutura de Dados 202402
Trabalho final realizado para a disciplina de Estrutura de Dados.

## Como compilar e rodar
Este projeto utiliza CMake na configuração da compilação, e recomenda-se que se use Linux.

Basta clonar este projeto e rodar o script `build.sh`, que irá criar uma pasta chamada 'build' com um executável 'proj'. Comandos completos:
````bash
git clone --recursive https://github.com/juliokscesar/trabfinal-ed.git
cd trabfinal-ed
chmod +X ./build.sh
./build.sh
./build/proj
````

## Descrição
Este projeto tem como objetivo gerar um modelo simples e eficiente na análise e previsão de séries binárias temporais, aproveitando-se do desempenho da linguagem C para garantir uma implementação otimizada.
O Grafo é a estrutura principal deste projeto, pois proporciona uma implementação intuitiva e prática para manusear conexões, pesos e as relações entre diferentes *neurônios* (nós).

## Expectativas e Funcionamento
O programa deve ser capaz de aplicar o modelo implementado para prever séries temporais binárias.

Uma série temporal binária é uma série de dados binários (0 ou 1) cujos valores futuros dependem dos valores anteriores. Todo sistema dinâmico cujo estado pode ser classificado binariamente é uma série temporal binária.
Exemplos incluem a marcação de pontos por cada time em uma partida de um esporte (como vôlei, basquete), previsão climática pela classificação de "choverá hoje" (1) ou "não choverá hoje" (0), monitoramento de tráfego ao longo de um dia (0 = "fluxo normal", 1 = "congestionamento"), entre muitas outras alpicações no mundo real.

## Métodos
### 1. Ajuste de geradores binários com Cadeias de Markov
Cadeias de Markov são modelos probabilísticos que descrevem sistemas onde o próximo estado depende exclusivamente do estado atual, representando processos estocásticos de memória curta. No contexto deste projeto, utilizamos Cadeias de Markov para construir geradores binários que preveem o próximo valor em uma série temporal.

Inicialmente, geramos $N$ matrizes de transição de estado, onde cada matriz representa um gerador e possui probabilidades aleatórias para transitar entre os estados $0$ e $1$. Durante a previsão, para cada valor na série de dados, todos os geradores são testados para predizer o próximo número da sequência.

O desempenho de cada gerador é ajustado dinamicamente: se a previsão estiver correta, o peso associado a ele é incrementado por uma taxa de aprendizado $lr$ (*learning-rate*); caso contrário, o peso é reduzido pelo mesmo valor. Esse mecanismo de reforço permite que os geradores mais precisos ao longo do tempo se destaquem, contribuindo para um modelo eficiente e adaptável.

### 2. Estrutura análoga a uma Rede Neural com Grafos

