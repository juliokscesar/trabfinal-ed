# Trabalho Final - Estrutura de Dados 202402
Trabalho final realizado para a disciplina de Estrutura de Dados.

## Como compilar e rodar
Este projeto utiliza CMake na configuração da compilação, e recomenda-se que se use Linux.

Basta clonar este projeto e rodar o script `build.sh`, que irá criar uma pasta chamada 'build' com um executável 'proj'. 
Comandos completos:
````shell
git clone --recursive https://github.com/juliokscesar/trabfinal-ed.git
cd trabfinal-ed
chmod +X ./build.sh
./build.sh
./build/proj -h
````

## Guia de Uso
O programa possui uma interface para ser utilizado pelo terminal, e o ajuste de configurações pode ser feito através do
arquivo `config.ini`.

### Exemplo
A pasta `data` contém alguns arquivos com dados para demonstração. O arquivo `p1_07.dat`, por exemplo, contém 2000 valores
gerados com uma probabilidade de 0,7 para 1 e 0,3 para 0. Para executar o programa com esse arquivo:
```shell
./build/proj -d data/p1_07.dat
```
A saída do programa depende das configurações no arquivo `config.ini`. Lá é possível especificar a razão para divisão dos
dados de treino, validação e teste, se deve usar grafos e redes, se deve mostrar as matrizes de transição obtidas, as matrizes
de confusão, etc.

### Configuração
Toda a configuração do programa é realizada a partir das variáveis no arquivo `config.ini`. No arquivo, cada item está
devidamente descrito com comentários para fácil compreensão. Todos os caminhos de arquivo devem ser relativos ao diretório atual
na execução.

***ATENÇÃO***: qualquer inserção, remoção ou alteração nos nomes das variáveis compromete o funcionamento do programa. Atente-se
a alterar apenas os *valores* das variáveis, e não seus nomes.

### Comandos
Após a compilação, a flag `-h` na execução do programa exibe uma mensagem de explicação de cada flag disponível.
```
-------------------------------------------------------------------------------------------------
---------------------------- TIME SERIES FORECAST WITH MARKOV CHAINS ----------------------------
-------------------------------------------------------------------------------------------------

Usage: ./proj [-h] [-d data_file] [-m] [-c config_file] [-w] [-s steps] [-p] [-o order]
=> [-h]: show this message and exit.
=> [-d data_file]: use data file in path data_file.
=> [-m]: insert data manually value by value. If this flag and '-d data_file' is provided, ignore the data file.
=> [-c config]: use config file in path confg_file.
=> [-w]: wait for user input before advancing to next sections.
=> [-s steps]: predict next 'steps' instead of what's in the configuration file.
=> [-p]: print details from loaded data. Useful for making sure the program has loaded things correctly.
=> [-o order]: use 'order' for the system, instead of what's set in the configuration file.

!! All file paths must be relative to the program's executable file.
!! You can change the default data file path in the config file. If no '-c config_file' is provided, it uses 'config.ini' as default.
```
- `-h`: exibe esta mensaem de ajuda e termina o programa, ignorando qualquer outra flag passada;
- `-d data_file`: utiliza o caminho do arquivo `data_file` explicitamente para carregar os dados, ignorando o caminho padrão no arquivo `config.ini`.
- `-m`: modo de inserção manual dos dados. Se esta flag é passada, o programa aguarda a entrada do usuário de quaisquer
números inteiros até a entrada de um caractere não numérico. Se `-m` e `-d` são passados juntos, apenas `-m` é usado.
- `-c config_file`: especifica o caminho do arquivo INI de configuração.
- `-w`: o programa aguarda a confirmação do usuário antes de prosseguir para as próximas etapas, ou seja, entre a utilização
da Cadeia de Markov Padrão, do Grafo de Markov (opcional) e da Rede de Markov (opcional). É útil para ler separadamente cada informação.
- `-s steps`: específica o número de passos `steps` para prever após as etapas de treinamento e teste.
- `-p`: exibe detalhes sobre os dados lidos, incluindo os próprios dados, valores únicos extraídos, divisão de treino,
validação e teste.
- `-o order`: especifica a ordem que deve ser utilizada para as cadeias de Markov. Se esta flag for passada, ignora o valor que está setado no arquvio `config.ini`.

## Descrição
Este projeto tem como objetivo gerar um modelo simples e eficiente na análise e previsão de séries binárias temporais, 
aproveitando-se do desempenho da linguagem C para garantir uma implementação otimizada. O Grafo é a estrutura principal 
deste projeto, pois proporciona uma implementação intuitiva e prática para manusear conexões, pesos e as relações entre 
diferentes *neurônios* (nós).

## Expectativas e Funcionamento
O programa deve ser capaz de aplicar o modelo implementado para prever séries temporais binárias.

Uma série temporal binária é uma série de dados binários (0 ou 1) cujos valores futuros dependem dos valores anteriores. 
Todo sistema dinâmico cujo estado pode ser classificado binariamente é uma série temporal binária. Exemplos incluem a 
marcação de pontos por cada time em uma partida de um esporte (como vôlei, basquete), previsão climática pela classificação 
de "choverá hoje" (1) ou "não choverá hoje" (0), monitoramento de tráfego ao longo de um dia (0 = "fluxo normal", 
1 = "congestionamento"), entre muitas outras alpicações no mundo real.

