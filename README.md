# Parallel Spotify Data Processing with MPI

Este projeto implementa uma aplicação paralela em C usando MPI para processar dados de músicas do Spotify, realizando três tipos principais de análise:

1. **Contagem de palavras** (40% da nota) - Conta a frequência de cada palavra nas letras das músicas
2. **Artistas com mais músicas** (40% da nota) - Identifica os artistas com maior quantidade de músicas
3. **Classificação de sentimento** (20% da nota) - Classifica as letras como Positiva, Neutra ou Negativa

## Requisitos

- **MPI (Message Passing Interface)**: Para execução paralela
- **Compilador C**: Compatível com C99
- **Sistema Linux**: Testado em Fedora 42

### Instalação das Dependências

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y mpich libmpich-dev
```

#### Fedora/CentOS/RHEL:
```bash
sudo dnf install -y mpich mpich-devel
```

## Estrutura do Projeto

```
ParalelMIP-C-trab2/
├── main.c                    # Código principal da aplicação
├── spotify_millsongdata.csv  # Dataset do Spotify
├── Makefile                  # Sistema de build
└── README.md                 # Este arquivo
```

## Compilação

### Compilação Básica
```bash
make
```

### Compilação com Debug
```bash
make debug
```

### Compilação com Profiling
```bash
make profile
```

## Execução

### Verificar se MPI está instalado
```bash
make check-mpi
```

### Executar com diferentes números de processos

```bash
# 1 processo (sequencial)
make run-1

# 2 processos
make run-2

# 4 processos
make run-4

# 8 processos
make run-8

# Número customizado de processos
make run NP=6
```

### Execução manual
```bash
# Compilar
make

# Executar com mpirun
mpirun -np 4 ./spotify_processor
```

## Funcionalidades Implementadas

### 1. Contagem de Palavras (40% da nota)
- Processa todas as letras das músicas em paralelo
- Remove pontuação e converte para minúsculas
- Conta frequência de cada palavra única
- Exibe as 20 palavras mais frequentes

### 2. Artistas com Mais Músicas (40% da nota)
- Conta o número de músicas por artista
- Processa dados em paralelo
- Exibe os 20 artistas com mais músicas

### 3. Classificação de Sentimento (20% da nota)
- Implementa análise de sentimento baseada em palavras-chave
- Classifica letras como Positiva, Neutra ou Negativa
- Utiliza dicionários de palavras positivas e negativas
- Exibe estatísticas de distribuição de sentimento

## Arquitetura Paralela

### Distribuição de Dados
- O processo 0 (master) lê o arquivo CSV
- Os dados são distribuídos uniformemente entre todos os processos
- Cada processo processa sua porção dos dados localmente

### Comunicação MPI
- `MPI_Bcast`: Transmite o número total de músicas
- `MPI_Scatterv`: Distribui os dados entre processos
- `MPI_Send/MPI_Recv`: Coleta resultados parciais
- `MPI_Reduce`: Agrega contadores de sentimento

### Estruturas de Dados
```c
typedef struct {
    char artist[MAX_ARTIST_LENGTH];
    char song[200];
    char text[MAX_LINE_LENGTH];
} SongData;

typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} WordCount;

typedef struct {
    char artist[MAX_ARTIST_LENGTH];
    int song_count;
} ArtistCount;
```

## Métricas de Performance

A aplicação mede e exibe:
- Tempo total de execução
- Taxa de processamento (músicas/segundo)
- Número total de palavras únicas processadas
- Número total de artistas únicos processados
- Distribuição de sentimento das músicas

### Exemplo de Saída
```
=== PARALLEL SPOTIFY DATA PROCESSING RESULTS ===
Total execution time: 15.42 seconds

=== TOP 20 MOST FREQUENT WORDS ===
1. love: 45231 occurrences
2. you: 38947 occurrences
3. i: 35621 occurrences
...

=== TOP 20 ARTISTS WITH MOST SONGS ===
1. The Beatles: 234 songs
2. Bob Dylan: 198 songs
3. Elvis Presley: 187 songs
...

=== SENTIMENT ANALYSIS RESULTS ===
Positive songs: 456789
Neutral songs: 234567
Negative songs: 123456

Sentiment distribution:
Positive: 56.1%
Neutral: 28.8%
Negative: 15.1%

=== PERFORMANCE METRICS ===
Total words processed: 12543 unique words
Total artists processed: 15678 unique artists
Total songs processed: 814812
Processing rate: 52847.32 songs/second
```

## Análise de Performance

### Fatores que Impactam o Desempenho

1. **Número de Processos**: Mais processos podem melhorar o tempo de processamento, mas há overhead de comunicação
2. **Tamanho do Dataset**: O arquivo CSV contém mais de 2.4 milhões de linhas
3. **Comunicação MPI**: A coleta de resultados parciais pode ser um gargalo
4. **Estruturas de Dados**: Uso de arrays simples para contagem pode ser ineficiente para grandes volumes

### Otimizações Implementadas

- Distribuição uniforme de dados
- Processamento local antes da comunicação
- Uso eficiente de memória com limites predefinidos
- Algoritmos de ordenação simples para resultados

## Limitações e Melhorias Futuras

### Limitações Atuais
- Análise de sentimento baseada apenas em palavras-chave
- Estruturas de dados com tamanhos fixos
- Algoritmos de ordenação não otimizados

### Possíveis Melhorias
- Integração com modelos de linguagem (Ollama, como sugerido)
- Uso de estruturas de dados mais eficientes (hash tables)
- Implementação de algoritmos de ordenação mais rápidos
- Paralelização da fase de merge dos resultados

## Troubleshooting

### Erro: "MPI compiler not found"
```bash
make install-deps  # Ubuntu/Debian
# ou
make install-deps-fedora  # Fedora/CentOS
```

### Erro: "Could not open file spotify_millsongdata.csv"
- Verifique se o arquivo CSV está no diretório correto
- Verifique as permissões de leitura do arquivo

### Performance baixa
- Verifique se o sistema tem recursos suficientes
- Teste com diferentes números de processos
- Use `make profile` para análise de performance

## Autores

Desenvolvido para o Trabalho 2 de Processamento Paralelo com C e MPI.

## Licença

Este projeto é para fins educacionais.
