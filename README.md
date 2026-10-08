# 🎯 Resta Um — Jogo de Tabuleiro em C com raylib

> Implementação do clássico **Resta Um** (Peg Solitaire) em linguagem C, com duas interfaces gráficas feitas em **raylib**: uma versão **2D** e uma versão **3D**

---

## 📋 Sobre o Projeto

O **Resta Um** é um quebra-cabeça de tabuleiro para um jogador. O objetivo é eliminar peças pulando umas sobre as outras até restar **apenas uma** no tabuleiro.

O projeto separa de forma clara a **lógica do jogo** (regras, validação, fim de jogo) da **interface** (como o jogo é desenhado e como o jogador interage). Por isso o mesmo código de regras alimenta tanto a versão 2D quanto a 3D: o que muda entre elas é apenas o arquivo `main`.

---

## 📜 Regras

1. Uma jogada move uma peça **duas casas** em linha reta (horizontal ou vertical), pulando sobre **uma peça vizinha**.
2. A casa de **destino** precisa estar vazia.
3. A peça que foi pulada é **removida** do tabuleiro.
4. O jogo termina quando não existe mais nenhuma jogada possível:
   - ✅ **Vitória:** sobrou apenas 1 peça.
   - ❌ **Derrota:** sobraram 2 ou mais peças.

---

## 🆚 Versão 2D × Versão 3D

As duas versões usam exatamente as mesmas regras. A diferença está na apresentação.

| | **2D** (`main.c`) | **3D** (`main_3d.c`) |
|---|---|---|
| **Visual** | Tabuleiro plano visto de cima, peças como círculos | Tabuleiro em perspectiva, peças torneadas em formato de peão de xadrez |
| **Cenário** | Fundo liso | Sala de aula completa: lousa, janelas, relógio, carteiras, mesa do professor |
| **Cores** | Paleta suave e escura | Tabuleiro claro/escuro estilo xadrez, peças cor de marfim |
| **Iluminação** | Nenhuma | Shader próprio (luz difusa, brilho e sombras sob as peças) |
| **Animações** | Troca direta de estado | Salto em arco, pouso com quique, captura voando para o lado, câmera suave |
| **Câmera** | Fixa | Livre: gira em volta do tabuleiro e dá zoom |
| **Sons** | Não tem | Sons sintetizados pelo próprio programa (sem arquivos de áudio) |
| **Requisitos** | Qualquer GPU com OpenGL compatível com a raylib | OpenGL 3.3 ou superior (usa shader GLSL `#version 330`) |
| **Peso** | Muito leve | Mais pesado (mais objetos, MSAA 4x e iluminação por pixel) |

### Controles

**Versão 2D**

| Ação | Controle |
|---|---|
| Selecionar peça / escolher destino | Clique esquerdo |
| Cancelar seleção | Clicar de novo na peça selecionada |
| Reiniciar | `R` |
| Tela cheia | `F11` (`ESC` sai da tela cheia) |
| Fechar o jogo | `ESC` (em janela) |

**Versão 3D**

| Ação | Controle |
|---|---|
| Selecionar peça / escolher destino | Clique esquerdo |
| Girar a câmera | Botão direito + arrastar, ou setas ← → |
| Zoom | Roda do mouse |
| Reiniciar | `R` |
| Ligar/desligar o som | `M` |
| Tela cheia | `F11` (`ESC` sai da tela cheia) |
| Fechar o jogo | `ESC` (em janela) |

Nas duas versões, ao selecionar uma peça, as casas de destino válidas ficam marcadas em verde.

---

## 🔀 Como Alternar Entre as Versões

Cada versão tem o seu próprio `main`, e **apenas um** deles pode ser compilado por vez (os dois definem a função `main`). Os arquivos `func_resta_um.h` e `resta_um.c` são sempre compilados junto.

### No VS Code (arquivo `tasks.json`)

Na tarefa **Executar Resta Um**, o comando do `gcc` indica qual `main` será usado. Basta trocar o nome do arquivo:

| Versão | Trecho do comando |
|---|---|
| 2D | `gcc.exe' main.c resta_um.c -o main.exe ...` |
| 3D | `gcc.exe' main_3d.c resta_um.c -o main.exe ...` |

Depois, execute a tarefa normalmente (`Ctrl+Shift+B`).

### Pelo terminal (Windows, w64devkit)

```bash
# Versão 2D
gcc main.c resta_um.c -o resta_um -IC:\raylib\raylib\src -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm

# Versão 3D
gcc main_3d.c resta_um.c -o resta_um_3d -IC:\raylib\raylib\src -LC:\raylib\raylib\src -lraylib -lopengl32 -lgdi32 -lwinmm
```

### Linux

```bash
gcc main.c    resta_um.c -o resta_um    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
gcc main_3d.c resta_um.c -o resta_um_3d -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
```

### macOS

```bash
gcc main.c    resta_um.c -o resta_um    -lraylib -framework OpenGL -framework Cocoa -framework IOKit
gcc main_3d.c resta_um.c -o resta_um_3d -lraylib -framework OpenGL -framework Cocoa -framework IOKit
```

---

## ⚙️ Pré-requisitos

- **Compilador C** (GCC recomendado; no Windows o projeto usa o `w64devkit` que acompanha a raylib)
- **[raylib](https://www.raylib.com/)** instalada. Versão 5.x recomendada.
- Placa de vídeo com **OpenGL 3.3+** (necessário para a versão 3D)

### Fonte (opcional)

Os dois jogos procuram uma fonte nesta ordem:

1. `fonte.ttf` na pasta do projeto
2. Segoe UI do Windows (`C:/Windows/Fonts/segoeui.ttf`)
3. Arial do Windows (`C:/Windows/Fonts/arial.ttf`)

Se nenhuma for encontrada, a raylib usa a fonte padrão, que **não exibe acentos**. Em Linux e macOS, coloque um arquivo `fonte.ttf` ao lado do código para ter acentuação correta.

---

## 🧱 Estrutura do Projeto

```
resta-um/
├── func_resta_um.h   # Constantes, códigos de erro e protótipos da lógica
├── resta_um.c        # Lógica do jogo (sem nenhuma entrada/saída)
├── main.c            # Interface 2D em raylib
├── main_3d.c         # Interface 3D em raylib
├── fonte.ttf         # (opcional) fonte com suporte a acentos
└── README.md
```

### Lógica do jogo (`func_resta_um.h` + `resta_um.c`)

Esta parte não usa `printf`, `scanf` nem raylib. Ela só trabalha com a matriz do tabuleiro, e é por isso que serve para qualquer interface.

| Função | O que faz |
|---|---|
| `inicializar(mat)` | Monta o tabuleiro 7×7 com a posição inicial |
| `validarJogada(mat, jog)` | Confere se a jogada segue as regras e retorna um código (`JOGADA_VALIDA` ou um código de erro) |
| `mensagemErro(erro)` | Converte um código de erro em texto para mostrar ao jogador |
| `executarJogada(mat, jog)` | Move a peça e remove a peça pulada |
| `contar(mat)` | Conta quantas peças restam |
| `verificaSeHaJogadas(mat)` | Procura se ainda existe alguma jogada possível (fim de jogo) |

A jogada é representada por um vetor `jog[5]`:

```
jog[0] = linha de origem      jog[1] = coluna de origem
jog[2] = linha de destino     jog[3] = coluna de destino
jog[4] = quantidade de valores lidos (4 = entrada completa)
```

Códigos retornados por `validarJogada`:

| Código | Motivo |
|---|---|
| `JOGADA_VALIDA` | Jogada permitida |
| `ERRO_ENTRADA` | Entrada incompleta ou inválida |
| `ERRO_LIMITES` | Posição fora do tabuleiro |
| `ERRO_SEM_PECA` | Não há peça na origem |
| `ERRO_DESTINO_OCUPADO` | O destino não está vazio |
| `ERRO_MOVIMENTO` | O movimento não pula uma casa em linha reta |
| `ERRO_SEM_SALTO` | Não há peça no meio para ser pulada |

### Interface 2D (`main.c`)

Laço principal do raylib que lê o clique, chama as funções de lógica e desenha o tabuleiro. As funções principais são `celulaDoMouse` (converte a posição do mouse em linha e coluna), `desenharTabuleiro` e `desenharMensagens`. O layout é recalculado a cada quadro, então se adapta a qualquer tamanho de janela e à tela cheia.

### Interface 3D (`main_3d.c`)

Além do laço principal, concentra o que torna o jogo 3D:

- **Peça torneada:** `gerarMeshPeca` cria a malha girando um perfil em torno do eixo, como num torno.
- **Iluminação:** um shader GLSL embutido no código dá luz, brilho e profundidade aos objetos.
- **Clique em 3D:** o mouse dispara um raio a partir da câmera e `celulaApontada` descobre qual casa foi atingida.
- **Animações:** estados suaves para elevação, marcas de destino, salto, pouso e captura.
- **Sala de aula:** `desenharSala` monta o cenário. A lousa usa uma textura gerada em tempo de execução e o relógio mostra a hora real do computador.
- **Sons:** `gerarSom` sintetiza as ondas sonoras em memória, sem precisar de arquivos de áudio.

---

## 🎨 Personalizando

| Quero mudar... | Onde mexer |
|---|---|
| Cores do jogo | Constantes `COR_*` no topo de `main.c` ou `main_3d.c` |
| Formato da peça 3D | Lista de pontos `perfil[][2]` em `gerarMeshPeca` (`main_3d.c`) |
| Texto da lousa | Função `criarQuadroNegro` (`main_3d.c`) |
| Móveis e paredes da sala | Funções `desenharSala` e `desenharCarteira` (`main_3d.c`) |
| Duração do salto | `DURACAO_ANIMACAO` (`main_3d.c`) |
| Sons | Funções `onda*` e `carregarSons` (`main_3d.c`) |
| Mensagens de erro | Função `mensagemErro` (`resta_um.c`) |
| Tamanho inicial da janela | `JANELA_LARGURA` e `JANELA_ALTURA` |

---

## 🛠️ Possíveis Melhorias

- Desfazer jogada
- Contador de jogadas e cronômetro
- Outros formatos de tabuleiro (europeu, 37 casas)
- Dica automática de jogada
- Salvar recordes