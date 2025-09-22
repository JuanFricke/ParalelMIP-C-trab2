#!/usr/bin/env python3
"""
Advanced Sentiment Analysis using Local LLM (Ollama)
This script provides a more sophisticated sentiment analysis compared to the keyword-based approach in C.
"""

import json
import sys
import requests
import time
from typing import Dict, List, Tuple

class SentimentAnalyzer:
    def __init__(self, ollama_url: str = "http://localhost:11434"):
        """
        Initialize the sentiment analyzer with Ollama connection.
        
        Args:
            ollama_url: URL of the Ollama server (default: http://localhost:11434)
        """
        self.ollama_url = ollama_url
        self.model = "llama2"  # Default model, can be changed
        
    def check_ollama_connection(self) -> bool:
        """Check if Ollama server is running and accessible."""
        try:
            response = requests.get(f"{self.ollama_url}/api/tags", timeout=5)
            return response.status_code == 200
        except requests.exceptions.RequestException:
            return False
    
    def get_available_models(self) -> List[str]:
        """Get list of available models in Ollama."""
        try:
            response = requests.get(f"{self.ollama_url}/api/tags")
            if response.status_code == 200:
                models = response.json().get('models', [])
                return [model['name'] for model in models]
            return []
        except requests.exceptions.RequestException:
            return []
    
    def analyze_sentiment(self, text: str) -> str:
        """
        Analyze sentiment of a given text using Ollama LLM.
        
        Args:
            text: Text to analyze
            
        Returns:
            Sentiment classification: "Positive", "Neutral", or "Negative"
        """
        if not self.check_ollama_connection():
            return self._fallback_sentiment_analysis(text)
        
        prompt = f"""
        Analyze the sentiment of the following song lyrics and classify them as either "Positive", "Neutral", or "Negative".
        
        Lyrics: "{text[:500]}"  # Limit text length for API efficiency
        
        Respond with only one word: Positive, Neutral, or Negative.
        """
        
        try:
            payload = {
                "model": self.model,
                "prompt": prompt,
                "stream": False,
                "options": {
                    "temperature": 0.1,  # Low temperature for consistent results
                    "top_p": 0.9
                }
            }
            
            response = requests.post(
                f"{self.ollama_url}/api/generate",
                json=payload,
                timeout=30
            )
            
            if response.status_code == 200:
                result = response.json()
                sentiment = result.get('response', '').strip().lower()
                
                # Parse the response
                if 'positive' in sentiment:
                    return "Positive"
                elif 'negative' in sentiment:
                    return "Negative"
                else:
                    return "Neutral"
            else:
                return self._fallback_sentiment_analysis(text)
                
        except requests.exceptions.RequestException:
            return self._fallback_sentiment_analysis(text)
    
    def _fallback_sentiment_analysis(self, text: str) -> str:
        """
        Fallback sentiment analysis using keyword-based approach.
        This is used when Ollama is not available.
        """
        positive_keywords = [
            'love', 'happy', 'joy', 'beautiful', 'wonderful', 'amazing', 'great', 
            'good', 'best', 'smile', 'laugh', 'dream', 'hope', 'peace', 'free',
            'bright', 'sunshine', 'celebrate', 'victory', 'success', 'win',
            'perfect', 'fantastic', 'excellent', 'brilliant', 'magical'
        ]
        
        negative_keywords = [
            'hate', 'sad', 'pain', 'hurt', 'cry', 'death', 'die', 'kill', 
            'bad', 'worst', 'angry', 'fear', 'scared', 'lonely', 'broken',
            'dark', 'nightmare', 'terrible', 'awful', 'horrible', 'disaster',
            'failure', 'lose', 'defeat', 'suffer', 'agony', 'despair'
        ]
        
        text_lower = text.lower()
        
        positive_count = sum(1 for word in positive_keywords if word in text_lower)
        negative_count = sum(1 for word in negative_keywords if word in text_lower)
        
        if positive_count > negative_count:
            return "Positive"
        elif negative_count > positive_count:
            return "Negative"
        else:
            return "Neutral"
    
    def batch_analyze(self, texts: List[str], batch_size: int = 10) -> List[str]:
        """
        Analyze multiple texts in batches.
        
        Args:
            texts: List of texts to analyze
            batch_size: Number of texts to process in each batch
            
        Returns:
            List of sentiment classifications
        """
        results = []
        
        for i in range(0, len(texts), batch_size):
            batch = texts[i:i + batch_size]
            batch_results = []
            
            for text in batch:
                sentiment = self.analyze_sentiment(text)
                batch_results.append(sentiment)
                time.sleep(0.1)  # Small delay to avoid overwhelming the server
            
            results.extend(batch_results)
            
            # Progress indicator
            if i % (batch_size * 10) == 0:
                print(f"Processed {min(i + batch_size, len(texts))}/{len(texts)} texts")
        
        return results

def main():
    """Main function for command-line usage."""
    if len(sys.argv) < 2:
        print("Usage: python3 sentiment_analyzer.py <text_to_analyze>")
        print("   or: python3 sentiment_analyzer.py --batch <input_file> <output_file>")
        sys.exit(1)
    
    analyzer = SentimentAnalyzer()
    
    if sys.argv[1] == "--batch":
        if len(sys.argv) != 4:
            print("Usage: python3 sentiment_analyzer.py --batch <input_file> <output_file>")
            sys.exit(1)
        
        input_file = sys.argv[2]
        output_file = sys.argv[3]
        
        try:
            with open(input_file, 'r', encoding='utf-8') as f:
                texts = [line.strip() for line in f if line.strip()]
            
            print(f"Analyzing {len(texts)} texts...")
            results = analyzer.batch_analyze(texts)
            
            with open(output_file, 'w', encoding='utf-8') as f:
                for text, sentiment in zip(texts, results):
                    f.write(f"{sentiment}\t{text}\n")
            
            print(f"Results saved to {output_file}")
            
        except FileNotFoundError:
            print(f"Error: File {input_file} not found")
            sys.exit(1)
        except Exception as e:
            print(f"Error: {e}")
            sys.exit(1)
    
    else:
        text = " ".join(sys.argv[1:])
        sentiment = analyzer.analyze_sentiment(text)
        print(f"Sentiment: {sentiment}")

if __name__ == "__main__":
    main()
