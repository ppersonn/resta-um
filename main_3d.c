#include <math.h>
#include <time.h>
#include "raylib.h"
#include "func_resta_um.h"

#define JANELA_LARGURA 1000
#define JANELA_ALTURA 760
#define DURACAO_ANIMACAO 0.65f
#define PI_F 3.14159265f
#define TAXA_AUDIO 44100
#define PISO_Y (-6.0f)
#define SALA 20.0f

#if defined(RAYLIB_VERSION_MAJOR) && RAYLIB_VERSION_MAJOR >= 5
    #define RAIO_DO_MOUSE(pos, cam) GetScreenToWorldRay(pos, cam)
#else
    #define RAIO_DO_MOUSE(pos, cam) GetMouseRay(pos, cam)
#endif

#define COR_FUNDO        (Color){30, 28, 29, 255}
#define COR_MOLDURA      (Color){52, 36, 28, 255}
#define COR_CASA_CLARA   (Color){192, 172, 142, 255}
#define COR_CASA_ESCURA  (Color){121, 90, 66, 255}
#define COR_BURACO       (Color){34, 26, 21, 255}
#define COR_PECA         (Color){226, 218, 200, 255}
#define COR_SELECAO      (Color){214, 186, 112, 255}
#define COR_DESTINO      (Color){120, 160, 132, 255}
#define COR_ERRO         (Color){196, 122, 112, 255}
#define COR_TEXTO        (Color){226, 220, 208, 255}
#define COR_TEXTO_SEC    (Color){158, 150, 140, 255}

#define COR_PAREDE_ALTA  (Color){196, 186, 160, 255}
#define COR_PAREDE_BAIXA (Color){110, 128, 112, 255}
#define COR_PISO_A       (Color){168, 156, 138, 255}
#define COR_PISO_B       (Color){148, 138, 122, 255}
#define COR_MADEIRA      (Color){158, 124, 90, 255}
#define COR_MADEIRA_ESC  (Color){110, 88, 66, 255}
#define COR_METAL        (Color){92, 100, 108, 255}
#define COR_CADEIRA      (Color){96, 120, 128, 255}
#define COR_JANELA       (Color){196, 214, 224, 255}
#define COR_ESQUADRIA    (Color){226, 222, 212, 255}

typedef struct {
    int ativa;
    int bateu;
    float t;
    int jog[5];
} Animacao;

typedef struct {
    int ativo;
    int lin, col;
    float u;
} Pouso;

static Font fonte;
static Shader luz;
static Model modCaixa, modCilindro, modPeca, modQuadro;
static Sound somSelecionar, somPouso, somCaptura, somErro, somVitoria, somDerrota;
static int audioOk = 0;
static float elev[TAM][TAM];
static float marca[TAM][TAM];

static const char *VERTEX_SHADER =
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec3 vertexNormal;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 matModel;\n"
    "out vec3 fragPos;\n"
    "out vec3 fragNormal;\n"
    "void main(){\n"
    "    fragPos = vec3(matModel*vec4(vertexPosition, 1.0));\n"
    "    fragNormal = normalize(vec3(transpose(inverse(matModel))*vec4(vertexNormal, 0.0)));\n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0);\n"
    "}\n";

static const char *FRAGMENT_SHADER =
    "#version 330\n"
    "in vec3 fragPos;\n"
    "in vec3 fragNormal;\n"
    "uniform vec4 colDiffuse;\n"
    "uniform vec3 viewPos;\n"
    "out vec4 finalColor;\n"
    "void main(){\n"
    "    vec3 n = normalize(fragNormal);\n"
    "    vec3 l = normalize(vec3(-0.5, 1.0, 0.6));\n"
    "    vec3 v = normalize(viewPos - fragPos);\n"
    "    vec3 h = normalize(l + v);\n"
    "    float diff = max(dot(n, l), 0.0);\n"
    "    float fill = max(dot(n, normalize(vec3(0.6, 0.4, -0.7))), 0.0)*0.2;\n"
    "    float spec = pow(max(dot(n, h), 0.0), 48.0)*0.30;\n"
    "    vec3 cor = colDiffuse.rgb*(vec3(0.42) + diff*0.68 + fill) + vec3(spec);\n"
    "    finalColor = vec4(cor, colDiffuse.a);\n"
    "}\n";

Font carregarFonte(void);
Mesh gerarMeshPeca(void);
Texture2D criarQuadroNegro(void);
void carregarRecursos(void);
void descarregarRecursos(void);
float ruido(float t);
float ondaSelecionar(float t);
float ondaPouso(float t);
float ondaCaptura(float t);
float ondaErro(float t);
float ondaVitoria(float t);
float ondaDerrota(float t);
Sound gerarSom(float duracao, float (*onda)(float t));
void carregarSons(void);
void tocar(Sound som);
Camera3D criarCamera(float giro, float inclinacao, float distancia);
Vector3 posicaoCelula(int lin, int col);
int celulaApontada(Camera3D cam, char mat[TAM][TAM], int *lin, int *col);
void atualizarVisuais(char mat[TAM][TAM], int selecionado, int selLin, int selCol, int hovLin, int hovCol, float dt);
void alternarTelaCheia(void);
float limitar(float v, float min, float max);
float suavizar(float t);
Color misturar(Color a, Color b, float t);
void escrever(const char *texto, float x, float y, float tam, Color cor);
void escreverCentro(const char *texto, float cx, float y, float tam, Color cor);
void escreverDireita(const char *texto, float xDir, float y, float tam, Color cor);
void desenharPeca(Vector3 base, float sxz, float sy, Color cor, Vector3 eixo, float angulo);
void desenharCaixa(Vector3 centro, Vector3 tam, Color cor);
void desenharCilindro(Vector3 base, float raio, float altura, Color cor);
void desenharSombra(float x, float z, float raio, float alfa);
void desenharMesa(float x, float z, float larg, float prof, float topoY, Color corTopo, Color corPerna);
void desenharCarteira(float x, float z);
void desenharRelogio(Vector3 c);
void desenharSala(void);
void desenharCena(char mat[TAM][TAM], int selecionado, int selLin, int selCol, int hovLin, int hovCol, Animacao *anim, Pouso *pouso);
void desenharMensagens(char mat[TAM][TAM], int erro, int fimDeJogo);

int main(){
    char mat[TAM][TAM];
    int selecionado = 0, selLin = 0, selCol = 0;
    int erro = JOGADA_VALIDA, fimDeJogo = 0, sair = 0, mudo = 0;
    float giro = 0.7f, inclinacao = 0.55f, distancia = 22.0f;
    float alvoGiro = 0.0f, alvoInclinacao = 0.95f, alvoDistancia = 12.5f;
    Animacao anim = {0};
    Pouso pouso = {0};

    inicializar(mat);

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(JANELA_LARGURA, JANELA_ALTURA, "Resta Um 3D");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    fonte = carregarFonte();
    carregarRecursos();
    carregarSons();

    while(!WindowShouldClose() && !sair){
        float dt = GetFrameTime();

        if(IsKeyPressed(KEY_F11)) alternarTelaCheia();

        if(IsKeyPressed(KEY_ESCAPE)){
            if(IsWindowFullscreen()) alternarTelaCheia();
            else sair = 1;
        }

        if(IsKeyPressed(KEY_M)){
            mudo = !mudo;
            if(audioOk) SetMasterVolume(mudo ? 0.0f : 1.0f);
        }

        if(IsKeyPressed(KEY_R)){
            inicializar(mat);
            selecionado = 0;
            erro = JOGADA_VALIDA;
            fimDeJogo = 0;
            anim.ativa = 0;
            pouso.ativo = 0;
            tocar(somSelecionar);
        }

        if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            Vector2 d = GetMouseDelta();
            alvoGiro -= d.x * 0.006f;
            alvoInclinacao = limitar(alvoInclinacao + d.y * 0.006f, 0.35f, 1.45f);
        }
        if(IsKeyDown(KEY_LEFT)) alvoGiro += 1.6f * dt;
        if(IsKeyDown(KEY_RIGHT)) alvoGiro -= 1.6f * dt;
        alvoDistancia = limitar(alvoDistancia - GetMouseWheelMove() * 0.8f, 7.0f, 16.0f);

        float k = 1.0f - expf(-10.0f * dt);
        giro += (alvoGiro - giro) * k;
        inclinacao += (alvoInclinacao - inclinacao) * k;
        distancia += (alvoDistancia - distancia) * k;

        Camera3D cam = criarCamera(giro, inclinacao, distancia);
        SetShaderValue(luz, luz.locs[SHADER_LOC_VECTOR_VIEW], &cam.position, SHADER_UNIFORM_VEC3);

        if(anim.ativa){
            anim.t += dt / DURACAO_ANIMACAO;

            if(!anim.bateu && anim.t >= 0.38f){
                anim.bateu = 1;
                tocar(somCaptura);
            }

            if(anim.t >= 1.0f){
                anim.ativa = 0;
                pouso.ativo = 1;
                pouso.lin = anim.jog[2];
                pouso.col = anim.jog[3];
                pouso.u = 0.0f;
                tocar(somPouso);
                fimDeJogo = !verificaSeHaJogadas(mat);
                if(fimDeJogo) tocar(contar(mat) == 1 ? somVitoria : somDerrota);
            }
        }

        if(pouso.ativo){
            pouso.u += dt;
            if(pouso.u > 0.45f) pouso.ativo = 0;
        }

        int lin = -1, col = -1;
        int apontou = !anim.ativa && celulaApontada(cam, mat, &lin, &col);
        int hovLin = apontou ? lin : -1;
        int hovCol = apontou ? col : -1;

        if(apontou && !fimDeJogo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){
            erro = JOGADA_VALIDA;

            if(mat[lin][col] == PECA){
                if(selecionado && lin == selLin && col == selCol){
                    selecionado = 0;
                }else{
                    selecionado = 1;
                    selLin = lin;
                    selCol = col;
                }
                tocar(somSelecionar);
            }else if(selecionado){
                int jog[5] = {selLin, selCol, lin, col, 4};
                erro = validarJogada(mat, jog);

                if(erro == JOGADA_VALIDA){
                    executarJogada(mat, jog);
                    selecionado = 0;
                    anim.ativa = 1;
                    anim.bateu = 0;
                    anim.t = 0.0f;
                    for(int n = 0; n < 5; n++) anim.jog[n] = jog[n];
                }else{
                    tocar(somErro);
                }
            }
        }

        atualizarVisuais(mat, selecionado, selLin, selCol, hovLin, hovCol, dt);

        BeginDrawing();
        ClearBackground(COR_FUNDO);
        BeginMode3D(cam);
        desenharCena(mat, selecionado, selLin, selCol, hovLin, hovCol, &anim, &pouso);
        EndMode3D();
        desenharMensagens(mat, erro, fimDeJogo);
        EndDrawing();
    }

    descarregarRecursos();
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

Mesh gerarMeshPeca(void){
    const float perfil[][2] = {
        {0.00f, 0.00f}, {0.34f, 0.00f}, {0.37f, 0.03f}, {0.37f, 0.08f}, {0.33f, 0.12f},
        {0.27f, 0.15f}, {0.20f, 0.22f}, {0.15f, 0.34f}, {0.12f, 0.48f}, {0.12f, 0.54f},
        {0.21f, 0.56f}, {0.22f, 0.59f}, {0.14f, 0.62f}, {0.14f, 0.64f}, {0.20f, 0.68f},
        {0.24f, 0.74f}, {0.25f, 0.81f}, {0.22f, 0.88f}, {0.16f, 0.93f}, {0.08f, 0.96f},
        {0.00f, 0.97f}
    };
    int np = sizeof(perfil) / sizeof(perfil[0]);
    int fatias = 48;

    Mesh m = {0};
    m.vertexCount = (fatias + 1) * np;
    m.triangleCount = fatias * (np - 1) * 2;
    m.vertices = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
    m.normals = (float *)MemAlloc(m.vertexCount * 3 * sizeof(float));
    m.texcoords = (float *)MemAlloc(m.vertexCount * 2 * sizeof(float));
    m.indices = (unsigned short *)MemAlloc(m.triangleCount * 3 * sizeof(unsigned short));

    for(int s = 0; s <= fatias; s++){
        float ang = 2.0f * PI_F * s / fatias;
        float sa = sinf(ang), ca = cosf(ang);

        for(int i = 0; i < np; i++){
            int a = i > 0 ? i - 1 : 0;
            int b = i < np - 1 ? i + 1 : np - 1;
            float nr = perfil[b][1] - perfil[a][1];
            float ny = -(perfil[b][0] - perfil[a][0]);
            float len = sqrtf(nr * nr + ny * ny);
            nr /= len;
            ny /= len;

            int v = s * np + i;
            m.vertices[3 * v] = perfil[i][0] * sa;
            m.vertices[3 * v + 1] = perfil[i][1];
            m.vertices[3 * v + 2] = perfil[i][0] * ca;
            m.normals[3 * v] = nr * sa;
            m.normals[3 * v + 1] = ny;
            m.normals[3 * v + 2] = nr * ca;
            m.texcoords[2 * v] = (float)s / fatias;
            m.texcoords[2 * v + 1] = (float)i / (np - 1);
        }
    }

    int k = 0;
    for(int s = 0; s < fatias; s++){
        for(int i = 0; i < np - 1; i++){
            unsigned short A = s * np + i;
            unsigned short B = (s + 1) * np + i;
            unsigned short C = A + 1;
            unsigned short D = B + 1;
            m.indices[k++] = A;
            m.indices[k++] = B;
            m.indices[k++] = D;
            m.indices[k++] = A;
            m.indices[k++] = D;
            m.indices[k++] = C;
        }
    }

    UploadMesh(&m, false);
    return m;
}

Texture2D criarQuadroNegro(void){
    int W = 1024, H = 512;
    Color giz = {232, 232, 220, 255};
    Color gizAmarelo = {236, 218, 150, 255};
    Image img = GenImageColor(W, H, (Color){49, 68, 58, 255});

    for(int k = 0; k < 12; k++)
        ImageDrawRectangle(&img, (k * 173) % 760, (k * 91) % 380, 240 + (k % 3) * 40, 80 + (k % 4) * 20, (Color){53, 72, 62, 255});

    Color borda = {74, 94, 82, 255};
    ImageDrawRectangle(&img, 10, 10, W - 20, 3, borda);
    ImageDrawRectangle(&img, 10, H - 13, W - 20, 3, borda);
    ImageDrawRectangle(&img, 10, 10, 3, H - 20, borda);
    ImageDrawRectangle(&img, W - 13, 10, 3, H - 20, borda);

    ImageDrawTextEx(&img, fonte, "Lógica de Programação", (Vector2){60, 40}, 64, 4, giz);
    ImageDrawTextEx(&img, fonte, "Jogo Resta Um", (Vector2){60, 125}, 42, 3, gizAmarelo);
    ImageDrawLine(&img, 60, 185, 520, 185, giz);

    ImageDrawTextEx(&img, fonte, "for(i = 0; i < 7; i++)", (Vector2){60, 215}, 34, 2, giz);
    ImageDrawTextEx(&img, fonte, "    mat[i][j] = PECA;", (Vector2){60, 265}, 34, 2, giz);
    ImageDrawTextEx(&img, fonte, "validarJogada(mat, jog);", (Vector2){60, 330}, 34, 2, gizAmarelo);
    ImageDrawTextEx(&img, fonte, "32 peças  →  1", (Vector2){60, 420}, 38, 3, giz);

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            if(!((i > 1 && i < 5) || (j > 1 && j < 5))) continue;

            int cx = 780 + (j - 3) * 46;
            int cy = 270 + (i - 3) * 46;

            if(i == 3 && j == 3) ImageDrawCircleLines(&img, cx, cy, 15, giz);
            else ImageDrawCircle(&img, cx, cy, 15, giz);
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    GenTextureMipmaps(&tex);
    SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    return tex;
}

void carregarRecursos(void){
    luz = LoadShaderFromMemory(VERTEX_SHADER, FRAGMENT_SHADER);

    modCaixa = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    modCilindro = LoadModelFromMesh(GenMeshCylinder(1.0f, 1.0f, 40));
    modPeca = LoadModelFromMesh(gerarMeshPeca());

    modCaixa.materials[0].shader = luz;
    modCilindro.materials[0].shader = luz;
    modPeca.materials[0].shader = luz;

    modQuadro = LoadModelFromMesh(GenMeshPlane(1.0f, 1.0f, 1, 1));
    SetMaterialTexture(&modQuadro.materials[0], MATERIAL_MAP_DIFFUSE, criarQuadroNegro());
}

void descarregarRecursos(void){
    if(audioOk){
        UnloadSound(somSelecionar);
        UnloadSound(somPouso);
        UnloadSound(somCaptura);
        UnloadSound(somErro);
        UnloadSound(somVitoria);
        UnloadSound(somDerrota);
        CloseAudioDevice();
    }
}

float ruido(float t){
    unsigned int x = (unsigned int)(t * TAXA_AUDIO) * 2654435761u;
    x ^= x >> 15;
    x *= 2246822519u;
    x ^= x >> 13;
    return (float)(x & 0xFFFF) / 32768.0f - 1.0f;
}

float ondaSelecionar(float t){
    float env = expf(-t * 45.0f);
    return (sinf(2.0f * PI_F * 740.0f * t) * 0.5f + sinf(2.0f * PI_F * 1480.0f * t) * 0.12f) * env;
}

float ondaPouso(float t){
    return sinf(2.0f * PI_F * 150.0f * t) * expf(-t * 30.0f) * 0.8f
         + sinf(2.0f * PI_F * 310.0f * t) * expf(-t * 45.0f) * 0.4f
         + ruido(t) * expf(-t * 180.0f) * 0.3f;
}

float ondaCaptura(float t){
    return sinf(2.0f * PI_F * 880.0f * t) * expf(-t * 60.0f) * 0.35f
         + sinf(2.0f * PI_F * 1320.0f * t) * expf(-t * 80.0f) * 0.15f
         + ruido(t) * expf(-t * 220.0f) * 0.3f;
}

float ondaErro(float t){
    float env = expf(-t * 9.0f);
    return (sinf(2.0f * PI_F * 150.0f * t) + sinf(2.0f * PI_F * 159.0f * t)) * 0.3f * env;
}

float ondaVitoria(float t){
    const float notas[4] = {523.25f, 659.25f, 783.99f, 1046.50f};
    float v = 0.0f;

    for(int n = 0; n < 4; n++){
        float dt = t - n * 0.16f;
        if(dt < 0.0f) continue;
        float env = expf(-dt * (n == 3 ? 3.0f : 6.0f));
        v += (sinf(2.0f * PI_F * notas[n] * dt) + sinf(2.0f * PI_F * notas[n] * 2.0f * dt) * 0.25f) * env * 0.3f;
    }
    return v;
}

float ondaDerrota(float t){
    const float notas[2] = {392.00f, 311.13f};
    float v = 0.0f;

    for(int n = 0; n < 2; n++){
        float dt = t - n * 0.24f;
        if(dt < 0.0f) continue;
        v += sinf(2.0f * PI_F * notas[n] * dt) * expf(-dt * 4.0f) * 0.4f;
    }
    return v;
}

Sound gerarSom(float duracao, float (*onda)(float t)){
    int n = (int)(duracao * TAXA_AUDIO);
    short *dados = (short *)MemAlloc(n * sizeof(short));

    for(int i = 0; i < n; i++){
        float v = limitar(onda((float)i / TAXA_AUDIO), -1.0f, 1.0f);
        dados[i] = (short)(v * 30000.0f);
    }

    Wave w = {0};
    w.frameCount = n;
    w.sampleRate = TAXA_AUDIO;
    w.sampleSize = 16;
    w.channels = 1;
    w.data = dados;

    Sound s = LoadSoundFromWave(w);
    UnloadWave(w);
    return s;
}

void carregarSons(void){
    InitAudioDevice();
    audioOk = IsAudioDeviceReady();
    if(!audioOk) return;

    somSelecionar = gerarSom(0.12f, ondaSelecionar);
    somPouso = gerarSom(0.20f, ondaPouso);
    somCaptura = gerarSom(0.14f, ondaCaptura);
    somErro = gerarSom(0.30f, ondaErro);
    somVitoria = gerarSom(1.40f, ondaVitoria);
    somDerrota = gerarSom(0.90f, ondaDerrota);
}

void tocar(Sound som){
    if(audioOk) PlaySound(som);
}

Camera3D criarCamera(float giro, float inclinacao, float distancia){
    Camera3D cam = {0};
    cam.target = (Vector3){0.0f, 0.0f, 0.0f};
    cam.up = (Vector3){0.0f, 1.0f, 0.0f};
    cam.fovy = 40.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    cam.position = (Vector3){
        distancia * cosf(inclinacao) * sinf(giro),
        distancia * sinf(inclinacao),
        distancia * cosf(inclinacao) * cosf(giro)
    };
    return cam;
}

Vector3 posicaoCelula(int lin, int col){
    return (Vector3){(float)(col - TAM / 2), 0.0f, (float)(lin - TAM / 2)};
}

int celulaApontada(Camera3D cam, char mat[TAM][TAM], int *lin, int *col){
    Ray raio = RAIO_DO_MOUSE(GetMousePosition(), cam);
    float melhor = 1e9f;
    int achou = 0;

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            if(mat[i][j] == FORA) continue;

            Vector3 c = posicaoCelula(i, j);
            float altura = (mat[i][j] == PECA) ? 0.95f : 0.05f;
            BoundingBox caixa = {{c.x - 0.5f, -0.3f, c.z - 0.5f}, {c.x + 0.5f, altura, c.z + 0.5f}};
            RayCollision hit = GetRayCollisionBox(raio, caixa);

            if(hit.hit && hit.distance < melhor){
                melhor = hit.distance;
                *lin = i;
                *col = j;
                achou = 1;
            }
        }
    }
    return achou;
}

void atualizarVisuais(char mat[TAM][TAM], int selecionado, int selLin, int selCol, int hovLin, int hovCol, float dt){
    float k = 1.0f - expf(-14.0f * dt);

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            float alvoElev = 0.0f, alvoMarca = 0.0f;

            if(mat[i][j] == PECA){
                if(selecionado && i == selLin && j == selCol) alvoElev = 1.0f;
                else if(i == hovLin && j == hovCol) alvoElev = 0.22f;
            }else if(mat[i][j] == VAZIO && selecionado){
                int jog[5] = {selLin, selCol, i, j, 4};
                if(validarJogada(mat, jog) == JOGADA_VALIDA) alvoMarca = 1.0f;
            }

            elev[i][j] += (alvoElev - elev[i][j]) * k;
            marca[i][j] += (alvoMarca - marca[i][j]) * k;
        }
    }
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

float limitar(float v, float min, float max){
    return v < min ? min : (v > max ? max : v);
}

float suavizar(float t){
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

Color misturar(Color a, Color b, float t){
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
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

void desenharPeca(Vector3 base, float sxz, float sy, Color cor, Vector3 eixo, float angulo){
    DrawModelEx(modPeca, base, eixo, angulo, (Vector3){sxz, sy, sxz}, cor);
}

void desenharCaixa(Vector3 centro, Vector3 tam, Color cor){
    DrawModelEx(modCaixa, centro, (Vector3){0, 1, 0}, 0, tam, cor);
}

void desenharCilindro(Vector3 base, float raio, float altura, Color cor){
    DrawModelEx(modCilindro, base, (Vector3){0, 1, 0}, 0, (Vector3){raio, altura, raio}, cor);
}

void desenharSombra(float x, float z, float raio, float alfa){
    if(alfa < 2.0f) return;
    desenharCilindro((Vector3){x, 0.004f, z}, raio, 0.004f, (Color){0, 0, 0, (unsigned char)alfa});
}

void desenharMesa(float x, float z, float larg, float prof, float topoY, Color corTopo, Color corPerna){
    float alturaPerna = topoY - 0.15f - PISO_Y;
    float yPerna = PISO_Y + alturaPerna / 2;
    float dx = larg / 2 - 0.4f, dz = prof / 2 - 0.4f;

    desenharCaixa((Vector3){x, topoY - 0.15f, z}, (Vector3){larg, 0.3f, prof}, corTopo);

    for(int sx = -1; sx <= 1; sx += 2)
        for(int sz = -1; sz <= 1; sz += 2)
            desenharCaixa((Vector3){x + sx * dx, yPerna, z + sz * dz}, (Vector3){0.3f, alturaPerna, 0.3f}, corPerna);
}

void desenharCarteira(float x, float z){
    desenharMesa(x, z, 4.6f, 3.2f, -2.0f, COR_MADEIRA, COR_METAL);

    desenharCaixa((Vector3){x, -3.4f, z + 3.0f}, (Vector3){2.4f, 0.2f, 2.2f}, COR_CADEIRA);
    desenharCaixa((Vector3){x, -2.3f, z + 4.0f}, (Vector3){2.4f, 2.0f, 0.2f}, COR_CADEIRA);

    for(int sx = -1; sx <= 1; sx += 2)
        for(int sz = -1; sz <= 1; sz += 2)
            desenharCaixa((Vector3){x + sx * 1.0f, -4.75f, z + 3.0f + sz * 0.9f}, (Vector3){0.2f, 2.5f, 0.2f}, COR_METAL);
}

void desenharRelogio(Vector3 c){
    time_t agora = time(NULL);
    struct tm *hora = localtime(&agora);
    float minutos = hora->tm_min + hora->tm_sec / 60.0f;
    float horas = (hora->tm_hour % 12) + minutos / 60.0f;
    float angulos[2] = {horas * 30.0f, minutos * 6.0f};
    float compr[2] = {1.0f, 1.5f};

    DrawModelEx(modCilindro, c, (Vector3){1, 0, 0}, 90, (Vector3){1.9f, 0.25f, 1.9f}, COR_MADEIRA_ESC);
    DrawModelEx(modCilindro, (Vector3){c.x, c.y, c.z + 0.2f}, (Vector3){1, 0, 0}, 90, (Vector3){1.7f, 0.1f, 1.7f}, (Color){232, 228, 216, 255});

    for(int n = 0; n < 2; n++){
        float rad = angulos[n] * PI_F / 180.0f;
        Vector3 centro = {c.x + sinf(rad) * compr[n] / 2, c.y + cosf(rad) * compr[n] / 2, c.z + 0.4f};
        DrawModelEx(modCaixa, centro, (Vector3){0, 0, 1}, -angulos[n], (Vector3){0.12f, compr[n], 0.05f}, (Color){40, 38, 38, 255});
    }
}

void desenharSala(void){
    float e = 0.5f;
    float alturaAlta = 13.0f, alturaBaixa = 5.0f;
    float yAlta = PISO_Y + alturaBaixa + alturaAlta / 2;
    float yBaixa = PISO_Y + alturaBaixa / 2;

    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            desenharCaixa((Vector3){-17.5f + i * 5.0f, PISO_Y - 0.1f, -17.5f + j * 5.0f}, (Vector3){5.0f, 0.2f, 5.0f},
                          ((i + j) % 2 == 0) ? COR_PISO_A : COR_PISO_B);

    for(int lado = -1; lado <= 1; lado += 2){
        float p = lado * (SALA + e / 2);
        desenharCaixa((Vector3){0, yAlta, p}, (Vector3){SALA * 2 + 2 * e, alturaAlta, e}, COR_PAREDE_ALTA);
        desenharCaixa((Vector3){0, yBaixa, p}, (Vector3){SALA * 2 + 2 * e, alturaBaixa, e}, COR_PAREDE_BAIXA);
        desenharCaixa((Vector3){p, yAlta, 0}, (Vector3){e, alturaAlta, SALA * 2}, COR_PAREDE_ALTA);
        desenharCaixa((Vector3){p, yBaixa, 0}, (Vector3){e, alturaBaixa, SALA * 2}, COR_PAREDE_BAIXA);
    }

    DrawModelEx(modQuadro, (Vector3){0.0f, 3.5f, -SALA + 0.3f}, (Vector3){1, 0, 0}, 90, (Vector3){18.0f, 1.0f, 8.0f}, (Color){235, 235, 235, 255});
    desenharCaixa((Vector3){0.0f, 3.5f, -SALA + 0.15f}, (Vector3){18.8f, 8.8f, 0.2f}, COR_MADEIRA_ESC);
    desenharCaixa((Vector3){0.0f, -0.75f, -SALA + 0.55f}, (Vector3){18.0f, 0.25f, 0.6f}, COR_MADEIRA_ESC);
    desenharRelogio((Vector3){13.0f, 8.0f, -SALA + 0.1f});

    for(int n = -1; n <= 1; n++){
        float z = n * 11.0f;
        desenharCaixa((Vector3){-SALA + 0.1f, 4.0f, z}, (Vector3){0.2f, 5.8f, 4.8f}, COR_ESQUADRIA);
        desenharCaixa((Vector3){-SALA + 0.2f, 4.0f, z}, (Vector3){0.1f, 5.2f, 4.2f}, COR_JANELA);
        desenharCaixa((Vector3){-SALA + 0.26f, 4.0f, z}, (Vector3){0.1f, 5.2f, 0.18f}, COR_ESQUADRIA);
        desenharCaixa((Vector3){-SALA + 0.26f, 4.0f, z}, (Vector3){0.1f, 0.18f, 4.2f}, COR_ESQUADRIA);
    }

    desenharCaixa((Vector3){SALA - 0.15f, -1.5f, -9.0f}, (Vector3){0.3f, 9.0f, 4.0f}, COR_MADEIRA_ESC);
    desenharCaixa((Vector3){SALA - 0.35f, -2.0f, -7.4f}, (Vector3){0.2f, 0.3f, 0.3f}, COR_METAL);
    desenharCaixa((Vector3){SALA - 0.1f, 3.0f, 9.0f}, (Vector3){0.2f, 4.0f, 8.0f}, (Color){160, 126, 92, 255});
    desenharCaixa((Vector3){SALA - 0.25f, 4.2f, 7.0f}, (Vector3){0.1f, 1.6f, 1.3f}, (Color){196, 178, 130, 255});
    desenharCaixa((Vector3){SALA - 0.25f, 2.4f, 9.4f}, (Vector3){0.1f, 1.4f, 1.8f}, (Color){138, 164, 150, 255});
    desenharCaixa((Vector3){SALA - 0.25f, 4.0f, 11.0f}, (Vector3){0.1f, 1.8f, 1.4f}, (Color){184, 140, 128, 255});

    desenharCaixa((Vector3){-8.0f, 4.0f, SALA - 0.1f}, (Vector3){5.0f, 3.4f, 0.1f}, (Color){138, 164, 150, 255});
    desenharCaixa((Vector3){2.0f, 3.0f, SALA - 0.1f}, (Vector3){3.2f, 4.6f, 0.1f}, (Color){196, 178, 130, 255});
    desenharCaixa((Vector3){9.0f, 4.0f, SALA - 0.1f}, (Vector3){4.4f, 3.0f, 0.1f}, (Color){184, 140, 128, 255});

    desenharMesa(-11.0f, -15.0f, 8.0f, 3.6f, -1.6f, COR_MADEIRA, COR_MADEIRA_ESC);

    desenharCarteira(-13.0f, -6.0f);
    desenharCarteira(-13.0f, 4.0f);
    desenharCarteira(-13.0f, 13.0f);
    desenharCarteira(13.0f, -6.0f);
    desenharCarteira(13.0f, 4.0f);
    desenharCarteira(13.0f, 13.0f);
}

void desenharCena(char mat[TAM][TAM], int selecionado, int selLin, int selCol, int hovLin, int hovCol, Animacao *anim, Pouso *pouso){
    float tempo = (float)GetTime();

    desenharSala();
    desenharMesa(0.0f, 0.0f, 12.0f, 10.0f, -0.5f, COR_MADEIRA, COR_METAL);

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            if(mat[i][j] == FORA) continue;

            Vector3 c = posicaoCelula(i, j);
            Color cor = ((i + j) % 2 == 0) ? COR_CASA_CLARA : COR_CASA_ESCURA;
            if(i == hovLin && j == hovCol && !(mat[i][j] == VAZIO && !selecionado))
                cor = (Color){cor.r + 14, cor.g + 14, cor.b + 14, 255};

            desenharCaixa((Vector3){c.x, -0.4f, c.z}, (Vector3){1.02f, 0.2f, 1.02f}, COR_MOLDURA);
            desenharCaixa((Vector3){c.x, -0.15f, c.z}, (Vector3){0.98f, 0.3f, 0.98f}, cor);

            if(mat[i][j] == VAZIO){
                desenharCilindro(c, 0.17f, 0.02f, COR_BURACO);

                if(marca[i][j] > 0.02f){
                    float pulso = 1.0f + 0.10f * sinf(tempo * 5.0f);
                    desenharCilindro(c, 0.26f * marca[i][j] * pulso, 0.035f, COR_DESTINO);
                }
            }
        }
    }

    for(int i = 0; i < TAM; i++){
        for(int j = 0; j < TAM; j++){
            if(mat[i][j] != PECA) continue;
            if(anim->ativa && i == anim->jog[2] && j == anim->jog[3]) continue;

            Vector3 c = posicaoCelula(i, j);
            float e = elev[i][j];
            int ehSelecionada = selecionado && i == selLin && j == selCol;
            float altura = 0.02f + 0.18f * e + (ehSelecionada ? 0.03f * sinf(tempo * 4.0f) * e : 0.0f);
            Color cor = misturar(COR_PECA, COR_SELECAO, limitar((e - 0.25f) / 0.75f, 0.0f, 1.0f));
            float sy = 1.0f;

            if(pouso->ativo && i == pouso->lin && j == pouso->col){
                float k = expf(-9.0f * pouso->u) * cosf(22.0f * pouso->u);
                sy = 1.0f - 0.20f * k;
            }

            desenharSombra(c.x, c.z, 0.40f - 0.06f * limitar(e, 0.0f, 1.0f), 120.0f * (1.0f - 0.4f * limitar(e, 0.0f, 1.0f)));
            desenharPeca((Vector3){c.x, altura, c.z}, 1.0f / sqrtf(sy), sy, cor, (Vector3){0, 1, 0}, 0.0f);
        }
    }

    if(anim->ativa){
        float t = anim->t;
        Vector3 a = posicaoCelula(anim->jog[0], anim->jog[1]);
        Vector3 b = posicaoCelula(anim->jog[2], anim->jog[3]);
        Vector3 d = {(b.x - a.x) / 2.0f, 0.0f, (b.z - a.z) / 2.0f};
        float suave = suavizar(t);
        float altura = 0.02f + 1.25f * sinf(PI_F * t);
        float sy = 1.0f + 0.10f * sinf(PI_F * t);
        if(t < 0.12f) sy -= 0.14f * sinf(PI_F * t / 0.12f);

        Vector3 voo = {a.x + (b.x - a.x) * suave, altura, a.z + (b.z - a.z) * suave};
        float proximidade = limitar((altura - 0.02f) / 1.4f, 0.0f, 1.0f);

        desenharSombra(voo.x, voo.z, 0.40f - 0.12f * proximidade, 120.0f * (1.0f - proximidade));
        desenharPeca(voo, 1.0f / sqrtf(sy), sy, COR_PECA, (Vector3){d.z, 0.0f, -d.x}, 16.0f * sinf(2.0f * PI_F * t));

        float p = limitar((t - 0.38f) / 0.62f, 0.0f, 1.0f);
        Vector3 m = posicaoCelula((anim->jog[0] + anim->jog[2]) / 2, (anim->jog[1] + anim->jog[3]) / 2);
        Vector3 perp = {-d.z, 0.0f, d.x};
        float lado = (m.x * perp.x + m.z * perp.z >= 0.0f) ? 1.0f : -1.0f;
        float fim = limitar((p - 0.55f) / 0.45f, 0.0f, 1.0f);
        float encolher = 1.0f - fim * fim * (3.0f - 2.0f * fim);

        if(encolher > 0.01f){
            Vector3 queda = {m.x + lado * perp.x * 2.0f * p, 0.02f + 1.0f * sinf(PI_F * p), m.z + lado * perp.z * 2.0f * p};
            desenharPeca(queda, encolher, encolher, COR_PECA, d, lado * 200.0f * p);
        }
    }
}

void desenharMensagens(char mat[TAM][TAM], int erro, int fimDeJogo){
    int restantes = contar(mat);
    float w = (float)GetScreenWidth(), h = (float)GetScreenHeight();
    float escH = h / JANELA_ALTURA, escW = w / 900.0f;
    float esc = escH < escW ? escH : escW;
    float margem = 30 * esc;
    float yBase = h - 80 * esc;

    escrever("RESTA UM", margem, 20 * esc, 42 * esc, COR_TEXTO);
    escrever(TextFormat("Peças: %d", restantes), margem, 70 * esc, 24 * esc, COR_TEXTO_SEC);

    escreverDireita("Botão direito + arrastar: girar  ·  Setas: girar", w - margem, 28 * esc, 17 * esc, COR_TEXTO_SEC);
    escreverDireita("Roda: zoom  ·  R: reiniciar  ·  M: som  ·  F11: tela cheia", w - margem, 52 * esc, 17 * esc, COR_TEXTO_SEC);

    if(fimDeJogo){
        const char *texto = (restantes == 1) ? "Parabéns, você venceu!" :
                            TextFormat("Fim de jogo! Restam %d peças.", restantes);
        escreverCentro(texto, w / 2, yBase, 30 * esc, COR_SELECAO);
        escreverCentro("Pressione R para jogar novamente", w / 2, yBase + 40 * esc, 20 * esc, COR_TEXTO_SEC);
    }else if(erro != JOGADA_VALIDA){
        escreverCentro(mensagemErro(erro), w / 2, yBase + 10 * esc, 22 * esc, COR_ERRO);
    }else{
        escreverCentro("Clique em uma peça e depois no destino.", w / 2, yBase + 10 * esc, 22 * esc, COR_TEXTO_SEC);
    }
}
