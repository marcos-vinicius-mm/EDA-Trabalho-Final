#!/usr/bin/env bash
# Compila relatorio.tex -> relatorio.pdf
# Requer: pdflatex (pacote texlive-latex-extra recomendado)

set -euo pipefail
cd "$(dirname "$0")"

if ! command -v pdflatex >/dev/null 2>&1; then
    echo "Erro: pdflatex nao encontrado."
    echo "Instale com: sudo apt install texlive-latex-base texlive-latex-extra texlive-lang-portuguese texlive-pictures"
    exit 1
fi

pdflatex -interaction=nonstopmode relatorio.tex
pdflatex -interaction=nonstopmode relatorio.tex
echo "PDF gerado: $(pwd)/relatorio.pdf"
