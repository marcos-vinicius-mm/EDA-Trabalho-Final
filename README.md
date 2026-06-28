# EDA — Trabalho Final

Contador de frequência de palavras usando quatro implementações de dicionário:
AVL iterativa, Rubro-Negra iterativa, Hash com encadeamento exterior e Hash com endereçamento aberto.

## Compilação

```bash
cd frequencia
make
```

## Uso

```bash
./freq dictionary <estrutura> <arquivo.txt> [saida.csv]
```

### Estruturas

| Parâmetro    | Estrutura                          |
|--------------|------------------------------------|
| `avl`        | Árvore AVL iterativa               |
| `rb`         | Árvore Rubro-Negra iterativa       |
| `hash-chain` | Hash com encadeamento exterior     |
| `hash-open`  | Hash com endereçamento aberto      |
| `all`        | Executa as quatro e exibe comparativo |

### Exemplos

```bash
./freq dictionary avl dom-casmurro
./freq dictionary hash-chain sherlock_holmes
./freq dictionary all dom-casmurro
./freq --help
```

Se informar apenas o nome do livro (sem caminho), o programa busca em `include/test_books/`.

Os CSVs são gravados em `sheet_results/<nome_do_livro>/`.

## Estrutura do projeto

```
frequencia/
  include/
    Dictionary.h
    AVLTree.h
    RBTree.h
    ChainedHashTable.h
    OpenAddressingHashTable.h
    test_books/          # Livros de teste (.txt)
  src/
    Main.cpp
  sheet_results/         # CSVs gerados (criado em runtime)
  Makefile
apresentacao/
  Relatorio/
    relatorio.pdf        # Relatório 
```

## Autores

- João Pedro Teófilo — `joaopedroteofilo@alu.ufc.br`
- Marcos V. de Morais Maniçoba — `marcosufcvinicius@alu.ufc.br`
