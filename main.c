#include "raylib.h"
#include "func_resta_um.h"

#define JANELA_LARGURA 640
#define JANELA_ALTURA 760

#define COR_FUNDO     (Color){45, 47, 52, 255}
#define COR_CASA      (Color){62, 65, 72, 255}
#define COR_BURACO    (Color){41, 43, 48, 255}
#define COR_PECA_BORDA (Color){206, 201, 190, 255}
#define COR_PECA      (Color){118, 148, 152, 255}
#define COR_SELECAO   (Color){196, 172, 120, 255}
#define COR_DESTINO   (Color){132, 166, 142, 255}
#define COR_ERRO      (Color){192, 124, 118, 255}
#define COR_TEXTO     (Color){224, 221, 214, 255}
#define COR_TEXTO_SEC (Color){150, 150, 148, 255}

typedef struct {
    float celula, x0, y0, esc;
} Layout;

static Font fonte;

Font carregarFonte(void);
Layout calcularLayout(void);
int celulaDoMouse(Layout lay, Vector2 mouse, int *lin, int *col);
void alternarTelaCheia(void);
void escrever(const char *texto, float x, float y, float tam, Color cor);
void escreverCentro(const char *texto, float cx, float y, float tam, Color cor);
void escreverDireita(const char *texto, float xDir, float y, float tam, Color cor);
void desenharTabuleiro(Layout lay, char mat[TAM][TAM], int selecionado, int selLin, int selCol);
void desenharMensagens(Layout lay, char mat[TAM][TAM], int erro, int fimDeJogo);

int main(){
    char mat[TAM][TAM];
    int selecionado = 0, selLin = 0, selCol = 0;
    int erro = JOGADA_VALIDA, fimDeJogo = 0, sair = 0;

    inicializar(mat);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(JANELA_LARGURA, JANELA_ALTURA, "Resta Um");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    fonte = carregarFonte();

    while(!WindowShouldClose() && !sair){
        if(IsKeyPressed(KEY_F11)) alternarTelaCheia();

        if(IsKeyPressed(KEY_ESCAPE)){
            if(IsWindowFullscreen()) alternarTelaCheia();
            else sair = 1;
        }

        if(IsKeyPressed(KEY_R)){
            inicializar(mat);
            selecionado = 0;
            erro = JOGADA_VALIDA;
            fimDeJogo = 0;
        }

        Layout lay = calcularLayout();
        int lin, col;

        if(!fimDeJogo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
           celulaDoMouse(lay, GetMousePosition(), &lin, &col) && mat[lin][col] != FORA){

            erro = JOGADA_VALIDA;

            if(mat[lin][col] == PECA){
                if(selecionado && lin == selLin && col == selCol){
                    selecionado = 0;
                }else{
                    selecionado = 1;
                    selLin = lin;
                    selCol = col;
                }
            }else if(selecionado){
                int jog[5] = {selLin, selCol, lin, col, 4};
                erro = validarJogada(mat, jog);

                if(erro == JOGADA_VALIDA){
                    executarJogada(mat, jog);
                    selecionado = 0;
                    fimDeJogo = !verificaSeHaJogadas(mat);
                }
            }
        }

        BeginDrawing();
        ClearBackground(COR_FUNDO);
        desenharTabuleiro(lay, mat, selecionado, selLin, selCol);
        desenharMensagens(lay, mat, erro, fimDeJogo);
        EndDrawing();
    }

    UnloadFont(fonte);
    CloseWindow();

    return 0;
}

Font carregarFonte(void){
    const char *caminhos[] = {"fonte.ttf", "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"};
    int codepoints[224];
    for(int i = 0; i < 224; i++) codepoints[i] = 32 + i;

    for(int i = 0; i < 3; i++){
        if(FileExists(caminhos[i])){
            Font f = LoadFontEx(caminhos[i], 64, codepoints, 224);
            GenTextureMipmaps(&f.texture);
            SetTextureFilter(f.texture, TEXTURE_FILTER_TRILINEAR);
            return f;
        }
    }
    return GetFontDefault();
}

Layout calcularLayout(void){
    float w = (float)GetScreenWidth();
    float h = (float)GetScreenHeight();
    Layout lay;

    float escH = h / JANELA_ALTURA, escW = w / JANELA_LARGURA;
    lay.esc = escH < escW ? escH : escW;

    float topo = 110 * lay.esc, base = 90 * lay.esc;
    float c1 = (h - topo - base) / TAM;
    float c2 = (w * 0.9f) / TAM;

    lay.celula = c1 < c2 ? c1 : c2;
    lay.x0 = (w - lay.celula * TAM) / 2;
    lay.y0 = topo + (h - topo - base - lay.celula * TAM) / 2;

    return lay;
}

int celulaDoMouse(Layout lay, Vector2 mouse, int *lin, int *col){
    if(mouse.x < lay.x0 || mouse.y < lay.y0) return 0;

    *col = (int)((mouse.x - lay.x0) / lay.celula);
    *lin = (int)((mouse.y - lay.y0) / lay.celula);

    return *lin < TAM && *col < TAM;
}

void alternarTelaCheia(void){
    int monitor = GetCurrentMonitor();

    if(IsWindowFullscreen()){
        ToggleFullscreen();
        SetWindowSize(JANELA_LARGURA, JANELA_ALTURA);
        SetWindowPosition((GetMonitorWidth(monitor) - JANELA_LARGURA) / 2,
                          (GetMonitorHeight(monitor) - JANELA_ALTURA) / 2);
    }else{
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    }
}

void escrever(const char *texto, float x, float y, float tam, Color cor){
    DrawTextEx(fonte, texto, (Vector2){x, y}, tam, tam / 16, cor);
}

void escreverCentro(const char *texto, float cx, float y, float tam, Color cor){
    Vector2 d = MeasureTextEx(fonte, texto, tam, tam / 16);
    escrever(texto, cx - d.x / 2, y, tam, cor);
}

void escreverDireita(const char *texto, float xDir, float y, float tam, Color cor){
    Vector2 d = MeasureTextEx(fonte, texto, tam, tam / 16);
    escrever(texto, xDir - d.x, y, tam, cor);
}

void desenharTabuleiro(Layout lay, char mat[TAM][TAM], int selecionado, int selLin, int selCol){
    float c = lay.celula;

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            if(mat[i][j] == FORA) continue;

            float x = lay.x0 + j * c;
            float y = lay.y0 + i * c;
            Vector2 centro = {x + c / 2, y + c / 2};

            DrawRectangleRounded((Rectangle){x + c * 0.04f, y + c * 0.04f, c * 0.92f, c * 0.92f}, 0.25f, 8, COR_CASA);

            if(mat[i][j] == PECA){
                DrawCircleV(centro, c * 0.375f, COR_PECA_BORDA);
                DrawCircleV(centro, c * 0.30f, COR_PECA);
                if(selecionado && i == selLin && j == selCol)
                    DrawRing(centro, c * 0.40f, c * 0.46f, 0, 360, 48, COR_SELECAO);
            }else{
                DrawCircleV(centro, c * 0.125f, COR_BURACO);
                if(selecionado){
                    int jog[5] = {selLin, selCol, i, j, 4};
                    if(validarJogada(mat, jog) == JOGADA_VALIDA)
                        DrawCircleV(centro, c * 0.20f, COR_DESTINO);
                }
            }
        }
    }
}

void desenharMensagens(Layout lay, char mat[TAM][TAM], int erro, int fimDeJogo){
    int restantes = contar(mat);
    float esc = lay.esc;
    float xEsq = lay.x0, xDir = lay.x0 + lay.celula * TAM;
    float centro = (xEsq + xDir) / 2;
    float yBase = lay.y0 + lay.celula * TAM + 18 * esc;

    escrever("RESTA UM", xEsq, 22 * esc, 42 * esc, COR_TEXTO);
    escrever(TextFormat("Peças: %d", restantes), xEsq, 72 * esc, 24 * esc, COR_TEXTO_SEC);
    escreverDireita("R: reiniciar  ·  F11: tela cheia", xDir, 78 * esc, 18 * esc, COR_TEXTO_SEC);

    if(fimDeJogo){
        const char *texto = (restantes == 1) ? "Parabéns, você venceu!" :
                            TextFormat("Fim de jogo! Restam %d peças.", restantes);
        escreverCentro(texto, centro, yBase, 30 * esc, COR_SELECAO);
        escreverCentro("Pressione R para jogar novamente", centro, yBase + 40 * esc, 20 * esc, COR_TEXTO_SEC);
    }else if(erro != JOGADA_VALIDA){
        escreverCentro(mensagemErro(erro), centro, yBase, 22 * esc, COR_ERRO);
    }else{
        escreverCentro("Clique em uma peça e depois no destino.", centro, yBase, 22 * esc, COR_TEXTO_SEC);
    }
}
