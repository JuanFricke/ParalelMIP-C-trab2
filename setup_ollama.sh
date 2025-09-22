#!/bin/bash

# Setup script for Ollama and advanced sentiment analysis
# This script installs Ollama and sets up the environment for LLM-based sentiment analysis

set -e

echo "=== Setting up Ollama for Advanced Sentiment Analysis ==="

# Check if Ollama is already installed
if command -v ollama &> /dev/null; then
    echo "Ollama is already installed."
    ollama --version
else
    echo "Installing Ollama..."
    
    # Install Ollama
    curl -fsSL https://ollama.com/install.sh | sh
    
    # Add Ollama to PATH for current session
    export PATH="$PATH:$HOME/.local/bin"
    
    echo "Ollama installed successfully!"
fi

# Start Ollama service
echo "Starting Ollama service..."
ollama serve &
OLLAMA_PID=$!

# Wait for Ollama to start
echo "Waiting for Ollama to start..."
sleep 5

# Check if Ollama is running
if ! curl -s http://localhost:11434/api/tags > /dev/null; then
    echo "Error: Ollama service failed to start"
    exit 1
fi

echo "Ollama service is running on http://localhost:11434"

# Pull a lightweight model for sentiment analysis
echo "Pulling Llama 2 model for sentiment analysis..."
ollama pull llama2:7b

# Alternative: Pull a smaller model if the above is too large
# echo "Pulling TinyLlama model (smaller alternative)..."
# ollama pull tinyllama

echo "Model pulled successfully!"

# Test the sentiment analyzer
echo "Testing sentiment analyzer..."
python3 -c "
import sys
sys.path.append('.')
from sentiment_analyzer import SentimentAnalyzer

analyzer = SentimentAnalyzer()
if analyzer.check_ollama_connection():
    print('✓ Ollama connection successful')
    models = analyzer.get_available_models()
    print(f'Available models: {models}')
    
    # Test sentiment analysis
    test_text = 'I love this beautiful day and feel so happy!'
    sentiment = analyzer.analyze_sentiment(test_text)
    print(f'Test sentiment analysis: \"{test_text}\" -> {sentiment}')
else:
    print('✗ Ollama connection failed')
"

echo ""
echo "=== Setup Complete ==="
echo "Ollama is now running with PID: $OLLAMA_PID"
echo "You can now use the advanced sentiment analysis features."
echo ""
echo "To stop Ollama: kill $OLLAMA_PID"
echo "To restart Ollama: ollama serve"
echo ""
echo "Usage examples:"
echo "  python3 sentiment_analyzer.py 'I love this song!'"
echo "  python3 sentiment_analyzer.py --batch lyrics.txt results.txt"
