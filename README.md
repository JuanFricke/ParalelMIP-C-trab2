# 🎵 Análise de Música com MPI

Análise paralela de música usando MPI para processar 57.650 músicas com:
- **Contagem de palavras** (40% da nota)
- **Análise de artistas** (40% da nota)  
- **Classificação de sentimento** (20% da nota)

## 🚀 Execução

```bash
# Executar análise completa com 14 processos MPI
./run_analysis.sh

# Executar benchmark (paralelo vs single thread)
./benchmark_simple.sh
```

## 📊 Configurações

- **Músicas**: 57.650
- **Processos**: 14 MPI (forçado com --oversubscribe)
- **Chunks**: 100 músicas cada
- **LLM**: 100 músicas para classificação
- **Progresso**: Visível a cada 10 chunks

## 📁 Arquivos Essenciais

- `music_analysis.c` - Programa principal
- `ollama_client.c/h` - Cliente para LLM
- `helper.c/h` - Funções auxiliares
- `golden_music.csv` - Dados das músicas
- `Makefile` - Compilação
- `run_analysis.sh` - Script de execução

## 🔧 Requisitos

- OpenMPI
- libcurl
- json-c
- Ollama (para classificação de sentimento)

## 📈 Progresso

O programa mostra progresso em tempo real:
```
Word counting: 1000/57650 songs processed (1.7%) - Chunk 10/577
Artist analysis: 1000/57650 songs processed (1.7%) - Chunk 10/577
```

