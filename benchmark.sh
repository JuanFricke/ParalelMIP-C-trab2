#!/bin/bash

# ========================================
# CONFIGURAÇÕES DO BENCHMARK
# ========================================
# Edite estas variáveis para personalizar o benchmark:
NUM_PROCESSES=14           # Número de processos MPI para teste paralelo
TEST_SONGS=10000000          # Número de músicas para teste (amostra)
PARALLEL_TIMEOUT=600      # Timeout para execução paralela (segundos)
SINGLE_TIMEOUT=600       # Timeout para execução single thread (segundos)
# ========================================

echo "🏁 BENCHMARK: Paralelo vs Single Thread 🏁"
echo "=========================================="
echo "Configurações:"
echo "- Processos MPI: $NUM_PROCESSES"
echo "- Músicas de teste: $TEST_SONGS"
echo "- Timeout paralelo: ${PARALLEL_TIMEOUT}s"
echo "- Timeout single: ${SINGLE_TIMEOUT}s"
echo ""

export PATH=/usr/lib64/openmpi/bin:$PATH

echo "Compilando programa principal..."
make clean
make main

if [ $? -eq 0 ]; then
    echo "✅ Compilação bem-sucedida!"
    echo ""
    
    # Criar um CSV de teste com apenas TEST_SONGS músicas
    echo "Criando CSV de teste com $TEST_SONGS músicas..."
    head -$((TEST_SONGS + 1)) golden_music.csv > test_music.csv
    
    # Modificar temporariamente o código para usar o arquivo de teste
    sed -i 's/golden_music.csv/test_music.csv/g' main.c
    make main
    
    echo ""
    echo "🚀 TESTE 1: Execução PARALELA ($NUM_PROCESSES processos MPI)"
    echo "============================================================"
    echo "Configurações:"
    echo "- Processos: $NUM_PROCESSES MPI"
    echo "- Músicas: $TEST_SONGS (amostra para teste rápido)"
    echo "- Versão: I/O OPTIMIZED"
    echo ""
    
    echo "Iniciando execução paralela..."
    echo "Tempo de início: $(date '+%H:%M:%S')"
    echo ""
    
    # Medir tempo de execução paralela
    time_start_parallel=$(date +%s.%N)
    timeout ${PARALLEL_TIMEOUT}s mpirun -np $NUM_PROCESSES --oversubscribe ./main > parallel_output.txt 2>&1
    parallel_exit_code=$?
    time_end_parallel=$(date +%s.%N)
    
    echo ""
    echo "Tempo de fim: $(date '+%H:%M:%S')"
    if [ $parallel_exit_code -eq 124 ]; then
        echo "⏰ Execução paralela interrompida por timeout (${PARALLEL_TIMEOUT}s)"
    else
        echo "✅ Execução paralela concluída!"
    fi
    echo ""
    
    echo "🐌 TESTE 2: Execução SINGLE THREAD (1 processo MPI)"
    echo "=================================================="
    echo "Configurações:"
    echo "- Processos: 1 MPI"
    echo "- Músicas: $TEST_SONGS (amostra para teste rápido)"
    echo "- Versão: I/O OPTIMIZED"
    echo ""
    
    echo "Iniciando execução single thread..."
    echo "Tempo de início: $(date '+%H:%M:%S')"
    echo ""
    
    # Medir tempo de execução single thread
    time_start_single=$(date +%s.%N)
    timeout ${SINGLE_TIMEOUT}s mpirun -np 1 ./main > single_output.txt 2>&1
    single_exit_code=$?
    time_end_single=$(date +%s.%N)
    
    echo ""
    echo "Tempo de fim: $(date '+%H:%M:%S')"
    if [ $single_exit_code -eq 124 ]; then
        echo "⏰ Execução single thread interrompida por timeout (${SINGLE_TIMEOUT}s)"
    else
        echo "✅ Execução single thread concluída!"
    fi
    echo ""
    
    # Calcular tempos
    parallel_time=$(echo "$time_end_parallel - $time_start_parallel" | bc -l)
    single_time=$(echo "$time_end_single - $time_start_single" | bc -l)
    
    if (( $(echo "$parallel_time > 0" | bc -l) )) && (( $(echo "$single_time > 0" | bc -l) )); then
        speedup=$(echo "scale=2; $single_time / $parallel_time" | bc -l)
        efficiency=$(echo "scale=1; $speedup / $NUM_PROCESSES * 100" | bc -l)
        
        # Resultados
        echo "📊 RESULTADOS DO BENCHMARK"
        echo "=========================="
        echo "⏱️  Tempo Paralelo ($NUM_PROCESSES processos): $(echo "scale=2; $parallel_time" | bc) segundos"
        echo "⏱️  Tempo Single Thread (1 processo): $(echo "scale=2; $single_time" | bc) segundos"
        echo "🚀 Speedup: $(echo "scale=2; $speedup" | bc)x mais rápido"
        echo "📈 Eficiência: $(echo "scale=1; $efficiency" | bc)%"
        echo ""
        
        # Análise de performance
        if (( $(echo "$speedup > 8" | bc -l) )); then
            echo "🎉 EXCELENTE: Speedup muito alto! Paralelização muito eficiente."
        elif (( $(echo "$speedup > 5" | bc -l) )); then
            echo "✅ BOM: Speedup significativo. Paralelização eficiente."
        elif (( $(echo "$speedup > 2" | bc -l) )); then
            echo "⚠️  MODERADO: Speedup moderado. Pode haver overhead de comunicação."
        else
            echo "❌ BAIXO: Speedup baixo. Paralelização pode não estar otimizada."
        fi
    else
        echo "❌ Não foi possível calcular speedup (tempos inválidos)"
    fi
    
    echo ""
    echo "🔧 OTIMIZAÇÕES APLICADAS:"
    echo "========================="
    echo "1. I/O Otimizado: Buffer de 1MB para leitura de arquivo"
    echo "2. Chunks de 100 linhas: Processa múltiplas linhas por vez"
    echo "3. LLM Reduzido: Apenas 20 músicas para classificação de sentimento"
    echo "4. Parsing Otimizado: Busca manual de delimitadores"
    echo "5. Memory Safe: Libera memória imediatamente após uso"
    echo ""
    
    echo "📋 LOGS DE EXECUÇÃO"
    echo "==================="
    echo "Log paralelo (últimas 5 linhas):"
    tail -5 parallel_output.txt
    echo ""
    echo "Log single thread (últimas 5 linhas):"
    tail -5 single_output.txt
    
    # Restaurar arquivo original
    sed -i 's/test_music.csv/golden_music.csv/g' main.c
    rm -f test_music.csv
    
else
    echo "❌ Erro na compilação!"
    exit 1
fi