#include <stdlib.h>
#include "func_resta_um.h"

void inicializar(char mat[TAM][TAM]){
    for(int i = 0; i < TAM; i++)
        for(int j = 0; j < TAM; j++)
            mat[i][j] = ((i > 1 && i < 5) || (j > 1 && j < 5)) ? PECA : FORA;

    mat[3][3] = VAZIO;
}

int validarJogada(char mat[TAM][TAM], int jog[5]){
    int linO = jog[0], colO = jog[1], linD = jog[2], colD = jog[3];

    if(jog[4] != 4) return ERRO_ENTRADA;

    if(linO < 0 || linO >= TAM || colO < 0 || colO >= TAM ||
       linD < 0 || linD >= TAM || colD < 0 || colD >= TAM)
        return ERRO_LIMITES;

    if(mat[linO][colO] != PECA) return ERRO_SEM_PECA;
    if(mat[linD][colD] != VAZIO) return ERRO_DESTINO_OCUPADO;

    int dLin = abs(linD - linO), dCol = abs(colD - colO);
    if(!((dLin == 2 && dCol == 0) || (dLin == 0 && dCol == 2)))
        return ERRO_MOVIMENTO;

    if(mat[(linO + linD) / 2][(colO + colD) / 2] != PECA)
        return ERRO_SEM_SALTO;

    return JOGADA_VALIDA;
}

const char *mensagemErro(int erro){
    switch(erro){
        case ERRO_ENTRADA:         return "Entrada invalida! Digite dois numeros por linha.";
        case ERRO_LIMITES:         return "Posicao fora dos limites do tabuleiro.";
        case ERRO_SEM_PECA:        return "A posicao de origem nao possui peca.";
        case ERRO_DESTINO_OCUPADO: return "A posicao de destino nao esta vazia.";
        case ERRO_MOVIMENTO:       return "O movimento deve pular uma peca em linha reta.";
        case ERRO_SEM_SALTO:       return "Nao ha peca para ser saltada no meio do trajeto.";
        default:                   return "";
    }
}

void executarJogada(char mat[TAM][TAM], int jog[5]){
    mat[jog[0]][jog[1]] = VAZIO;
    mat[jog[2]][jog[3]] = PECA;
    mat[(jog[0] + jog[2]) / 2][(jog[1] + jog[3]) / 2] = VAZIO;
}

int contar(char mat[TAM][TAM]){
    int q = 0;

    for(int i = 0; i < TAM; i++)
        for(int j = 0; j < TAM; j++)
            if(mat[i][j] == PECA) q++;

    return q;
}

int verificaSeHaJogadas(char mat[TAM][TAM]){
    int dl[4] = {-1, 1, 0, 0};
    int dc[4] = {0, 0, -1, 1};

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            if(mat[i][j] != PECA) continue;

            for(int d = 0; d < 4; d++){
                int li = i + dl[d], ci = j + dc[d];
                int lf = i + 2 * dl[d], cf = j + 2 * dc[d];

                if(lf >= 0 && lf < TAM && cf >= 0 && cf < TAM &&
                   mat[li][ci] == PECA && mat[lf][cf] == VAZIO)
                    return 1;
            }
        }
    }
    return 0;
}
