#!/bin/bash

echo "🚀 Music Analysis Program - I/O Optimized MPI Version"
echo "====================================================="

export PATH=/usr/lib64/openmpi/bin:$PATH

echo "Compilando programa..."
make clean
make main

if [ $? -eq 0 ]; then
    echo "✅ Compilação bem-sucedida!"
    echo ""
    
    echo "🚀 Executando com 14 processos MPI..."
    echo "Configurações:"
    echo "- Processos: 14 MPI"
    echo "- Músicas: Todas do CSV (golden_music.csv)"
    echo "- Otimizações: I/O otimizado, buffer 1MB, chunks de 100 linhas"
    echo ""
    
    echo "Iniciando execução..."
    echo "Tempo de início: $(date '+%H:%M:%S')"
    echo ""
    
    # Executar versão otimizada
    mpirun -np 14 --oversubscribe ./main
    
    echo ""
    echo "Tempo de fim: $(date '+%H:%M:%S')"
    echo "✅ Execução concluída!"
    
else
    echo "❌ Erro na compilação!"
    exit 1
fi