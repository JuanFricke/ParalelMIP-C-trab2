# 🎵 Apresentação - Análise de Música Paralela com MPI

## 📋 Sobre a Apresentação

Esta apresentação documenta o projeto **ParalelMIP-C-trab2**, que implementa análise de música usando processamento paralelo com MPI.

## 📁 Arquivos da Apresentação

- **`apresentacao.md`** - Apresentação em formato Markdown (Marp)
- **`apresentacao.html`** - Apresentação em HTML (gerada automaticamente)
- **`gerar_apresentacao.sh`** - Script para gerar PDF com Marp
- **`gerar_apresentacao_simples.sh`** - Script para gerar HTML simples

## 🎯 Conteúdo da Apresentação

### 1. **Objetivos do Projeto**
- Contagem de palavras (40% da nota)
- Análise de artistas (40% da nota)  
- Classificação de sentimento (20% da nota)

### 2. **Arquitetura Técnica**
- Stack tecnológico (C + MPI + Ollama)
- Estrutura de dados
- Otimizações implementadas

### 3. **Estratégia de Paralelização**
- Distribuição round-robin
- Chunks de 100 linhas
- Processamento otimizado

### 4. **Resultados de Performance**
- Benchmark paralelo vs single thread
- Speedup: 0.84x
- Eficiência: 6.0%

### 5. **Resultados da Análise**
- 49,999 palavras únicas encontradas
- 371 artistas únicos identificados
- Classificação de sentimento: 35% Positive, 45% Neutral, 20% Negative

### 6. **Desafios e Soluções**
- Memory overflow → Chunks pequenos
- I/O bottleneck → Buffer de 1MB
- Overhead MPI → Chunks maiores
- LLM sequencial → Apenas processo 0

## 🚀 Como Usar

### Opção 1: HTML Simples (Recomendado)
```bash
./gerar_apresentacao_simples.sh
firefox apresentacao.html
```

### Opção 2: Marp (se instalado)
```bash
# Instalar Marp
npm install -g @marp-team/marp-cli

# Gerar apresentação
./gerar_apresentacao.sh
```

### Opção 3: Visualizar Markdown
```bash
# Visualizar o arquivo .md diretamente
cat apresentacao.md
```

## 📊 Dados da Apresentação

### Performance
- **Tempo Paralelo:** 8.10 segundos
- **Tempo Single:** 6.84 segundos
- **Speedup:** 0.84x
- **Eficiência:** 6.0%

### Dataset
- **Total de músicas:** 57,650
- **Palavras únicas:** 49,999
- **Artistas únicos:** 371
- **Músicas para LLM:** 2,000

### Top 5 Palavras
1. **the:** 497,448 ocorrências
2. **you:** 495,482 ocorrências
3. **to:** 296,742 ocorrências
4. **and:** 294,467 ocorrências
5. **it:** 219,469 ocorrências

## 🎨 Características da Apresentação

### Design
- **Tema:** Moderno e profissional
- **Cores:** Azul e branco
- **Layout:** Responsivo
- **Tipografia:** Segoe UI

### Funcionalidades
- **Navegação:** Scroll suave
- **Responsivo:** Adapta a diferentes telas
- **Print-friendly:** Pode ser impresso como PDF
- **Acessível:** Cores contrastantes

## 📝 Estrutura dos Slides

1. **Título** - Nome do projeto
2. **Objetivos** - 3 desafios principais
3. **Arquitetura** - Stack tecnológico
4. **Paralelização** - Estratégia round-robin
5. **Otimizações** - I/O e memory management
6. **Performance** - Benchmark results
7. **Resultados** - Análise de dados
8. **Sentimento** - Classificação LLM
9. **Desafios** - Problemas e soluções
10. **Código** - Estrutura do projeto
11. **Conclusões** - Objetivos alcançados
12. **Próximos Passos** - Melhorias futuras
13. **Obrigado** - Perguntas e discussão

## 🔧 Personalização

### Modificar Conteúdo
Edite o arquivo `apresentacao.md` para alterar:
- Texto dos slides
- Dados de performance
- Resultados da análise
- Estrutura da apresentação

### Modificar Visual
Edite o arquivo `gerar_apresentacao_simples.sh` para alterar:
- Cores do tema
- Tipografia
- Layout
- Estilos CSS

## 📱 Compatibilidade

### Navegadores Suportados
- ✅ Firefox
- ✅ Chrome/Chromium
- ✅ Safari
- ✅ Edge

### Dispositivos
- ✅ Desktop
- ✅ Tablet
- ✅ Mobile (responsivo)

## 🎯 Dicas de Apresentação

### Para Apresentar
1. **Abra o HTML** no navegador
2. **Use F11** para tela cheia
3. **Navegue com scroll** ou setas
4. **Tenha backup** em PDF

### Para Imprimir
1. **Ctrl+P** no navegador
2. **Selecione "Salvar como PDF"**
3. **Ajuste margens** se necessário
4. **Imprima** ou salve

## 📞 Suporte

Se tiver problemas com a apresentação:

1. **Verifique** se os arquivos existem
2. **Execute** os scripts com permissão
3. **Teste** em diferentes navegadores
4. **Consulte** este README

---

**🎉 Boa apresentação!** 🎵
