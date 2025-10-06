#!/bin/bash

echo "🎨 Gerando Apresentação com Marp"
echo "================================="

# Verificar se Marp está instalado
if ! command -v marp &> /dev/null; then
    echo "❌ Marp não está instalado!"
    echo "📦 Instalando Marp..."
    npm install -g @marp-team/marp-cli
fi

# Gerar PDF
echo "📄 Gerando PDF..."
marp apresentacao.md --pdf --output apresentacao.pdf

if [ $? -eq 0 ]; then
    echo "✅ Apresentação gerada com sucesso!"
    echo "📁 Arquivo: apresentacao.pdf"
    echo ""
    echo "🎯 Para visualizar:"
    echo "   - PDF: evince apresentacao.pdf"
    echo "   - HTML: marp apresentacao.md --html"
    echo "   - Slides: marp apresentacao.md --server"
else
    echo "❌ Erro ao gerar apresentação!"
    exit 1
fi
