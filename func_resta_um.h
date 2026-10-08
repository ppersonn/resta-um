#ifndef RESTA_UM_H
#define RESTA_UM_H

#define TAM 7
#define PECA '@'
#define VAZIO 'O'
#define FORA ' '

enum {
    JOGADA_VALIDA,
    ERRO_ENTRADA,
    ERRO_LIMITES,
    ERRO_SEM_PECA,
    ERRO_DESTINO_OCUPADO,
    ERRO_MOVIMENTO,
    ERRO_SEM_SALTO
};

void inicializar(char mat[TAM][TAM]);
int validarJogada(char mat[TAM][TAM], int jog[5]);
const char *mensagemErro(int erro);
void executarJogada(char mat[TAM][TAM], int jog[5]);
int contar(char mat[TAM][TAM]);
int verificaSeHaJogadas(char mat[TAM][TAM]);

#endif
