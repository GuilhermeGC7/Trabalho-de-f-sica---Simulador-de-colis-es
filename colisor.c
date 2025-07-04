#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#define MAX_BOLAS 100

// Variáveis configuráveis
int NUM_BOLAS = 50;
float VELOCIDADE_MAX = 20.0f;
float COEF_RESTITUICAO = 1.0f;
float TAMANHO_CUBO = 40.0f;
float LIMITE = 20.0f; // Será atualizado com base no TAMANHO_CUBO

typedef struct Bola {
    Vector3 posicao;
    Vector3 velocidade;
    Color   cor;
    float   raio;
} Bola;

static Bola bolas[MAX_BOLAS];
bool mostrarGrade = false;
int modoCamera = 1; // 0: fixa, 1: orbital, 2: livre

// Menu Inicial
static void MenuInicial(void) {
    printf("=== SIMULADOR DE COLISÕES 3D ===\n");

    // Número de bolas
    do {
        printf("Digite o número de bolas (1 a %d): ", MAX_BOLAS);
        scanf("%d", &NUM_BOLAS);
    } while (NUM_BOLAS < 1 || NUM_BOLAS > MAX_BOLAS);

    // Velocidade máxima
    do {
        printf("Digite a velocidade máxima inicial (ex: 10.0): ");
        scanf("%f", &VELOCIDADE_MAX);
    } while (VELOCIDADE_MAX <= 0.0f);

    // Coeficiente de restituição
    do {
        printf("Digite o coeficiente de restituição (Cr) [ex: 0.9 ou 1.0 ou 1.2]: ");
        scanf("%f", &COEF_RESTITUICAO);
    } while (COEF_RESTITUICAO <= 0.0f);

    // Tamanho do cubo
    do {
        printf("Digite o tamanho do cubo (ex: 40.0): ");
        scanf("%f", &TAMANHO_CUBO);
    } while (TAMANHO_CUBO <= 1.0f);

    LIMITE = TAMANHO_CUBO / 2.0f;

    printf("Configurando simulador...\n");
}

// Inicializa bolas
static void InicializarBolas(void) {
    srand((unsigned)time(NULL));
    for (int i = 0; i < NUM_BOLAS; i++) {
        bolas[i].posicao = (Vector3){
            GetRandomValue(-(int)LIMITE + 2, (int)LIMITE - 2),
            GetRandomValue(-(int)LIMITE + 2, (int)LIMITE - 2),
            GetRandomValue(-(int)LIMITE + 2, (int)LIMITE - 2)};

        bolas[i].velocidade = (Vector3){
            ((float)GetRandomValue(-100, 100) / 100) * VELOCIDADE_MAX,
            ((float)GetRandomValue(-100, 100) / 100) * VELOCIDADE_MAX,
            ((float)GetRandomValue(-100, 100) / 100) * VELOCIDADE_MAX};

        bolas[i].cor = (Color){rand() % 256, rand() % 256, rand() % 256, 255};
        bolas[i].raio = 0.7f + (rand() % 60) / 100.0f;
    }
}

static void ColisaoParede(Bola *b) {
    if (b->posicao.x > LIMITE - b->raio) { b->posicao.x = LIMITE - b->raio; b->velocidade.x *= -COEF_RESTITUICAO; }
    else if (b->posicao.x < -LIMITE + b->raio) { b->posicao.x = -LIMITE + b->raio; b->velocidade.x *= -COEF_RESTITUICAO; }

    if (b->posicao.y > LIMITE - b->raio) { b->posicao.y = LIMITE - b->raio; b->velocidade.y *= -COEF_RESTITUICAO; }
    else if (b->posicao.y < -LIMITE + b->raio) { b->posicao.y = -LIMITE + b->raio; b->velocidade.y *= -COEF_RESTITUICAO; }

    if (b->posicao.z > LIMITE - b->raio) { b->posicao.z = LIMITE - b->raio; b->velocidade.z *= -COEF_RESTITUICAO; }
    else if (b->posicao.z < -LIMITE + b->raio) { b->posicao.z = -LIMITE + b->raio; b->velocidade.z *= -COEF_RESTITUICAO; }
}

static void ColisoesEntreBolas(void) {
    for (int i = 0; i < NUM_BOLAS; i++) {
        for (int j = i + 1; j < NUM_BOLAS; j++) {
            Vector3 delta = Vector3Subtract(bolas[i].posicao, bolas[j].posicao);
            float dist = Vector3Length(delta);
            float somaR = bolas[i].raio + bolas[j].raio;

            if (dist < somaR && dist > 0.0f) {
                Vector3 normal = Vector3Scale(delta, 1.0f / dist);

                float vi1 = Vector3DotProduct(bolas[i].velocidade, normal);
                float vi2 = Vector3DotProduct(bolas[j].velocidade, normal);
                float vcm = (vi1 + vi2) * 0.5f;

                float vf1 = (1.0f + COEF_RESTITUICAO) * vcm - COEF_RESTITUICAO * vi1;
                float vf2 = (1.0f + COEF_RESTITUICAO) * vcm - COEF_RESTITUICAO * vi2;

                bolas[i].velocidade = Vector3Add(bolas[i].velocidade, Vector3Scale(normal, vf1 - vi1));
                bolas[j].velocidade = Vector3Add(bolas[j].velocidade, Vector3Scale(normal, vf2 - vi2));

                float overlap = somaR - dist;
                Vector3 correcao = Vector3Scale(normal, overlap * 0.5f);
                bolas[i].posicao = Vector3Add(bolas[i].posicao, correcao);
                bolas[j].posicao = Vector3Subtract(bolas[j].posicao, correcao);
            }
        }
    }
}

static void AtualizarSimulacao(float dt) {
    for (int i = 0; i < NUM_BOLAS; i++) {
        bolas[i].posicao = Vector3Add(bolas[i].posicao, Vector3Scale(bolas[i].velocidade, dt));
        ColisaoParede(&bolas[i]);
    }
    ColisoesEntreBolas();
}

int main(void) {
    MenuInicial();

    const int screenWidth = 1000;
    const int screenHeight = 700;
    InitWindow(screenWidth, screenHeight, "Simulação de Colisões");
    SetTargetFPS(60);

    Camera3D camera = {
        .position = (Vector3){25.0f, 25.0f, 25.0f},
        .target = (Vector3){0.0f, 0.0f, 0.0f},
        .up = (Vector3){0.0f, 1.0f, 0.0f},
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    InicializarBolas();

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        if (IsKeyDown(KEY_UP)) {
            camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.target, camera.position)), 10.0f * dt));
        }
        if (IsKeyDown(KEY_DOWN)) {
            camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Normalize(Vector3Subtract(camera.position, camera.target)), 10.0f * dt));
        }

        if (IsKeyPressed(KEY_G)) mostrarGrade = !mostrarGrade;
        if (IsKeyPressed(KEY_C)) modoCamera = (modoCamera + 1) % 3;

        if (modoCamera == 1) UpdateCamera(&camera, CAMERA_ORBITAL);
        else if (modoCamera == 2) {
            UpdateCamera(&camera, CAMERA_FREE);
            if (IsKeyDown(KEY_Q)) camera.position.y += 10.0f * dt;
            if (IsKeyDown(KEY_E)) camera.position.y -= 10.0f * dt;
        }

        AtualizarSimulacao(dt);

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
            DrawCubeWires((Vector3){0, 0, 0}, TAMANHO_CUBO, TAMANHO_CUBO, TAMANHO_CUBO, GRAY);
            if (mostrarGrade) DrawGrid(20, 1.0f);
            for (int i = 0; i < NUM_BOLAS; i++) {
                DrawSphere(bolas[i].posicao, bolas[i].raio, bolas[i].cor);
            }
        EndMode3D();

        DrawText(TextFormat("Cr: %.2f | Vel: %.1f | Bolas: %d | Cubo: %.1f", COEF_RESTITUICAO, VELOCIDADE_MAX, NUM_BOLAS, TAMANHO_CUBO), 10, 10, 20, DARKGRAY);
        const char *nomeModos[] = { "Estática", "Orbital", "Livre" };
        DrawText(TextFormat("Câmera: %s (C) | Zoom: ↑↓", nomeModos[modoCamera]), 10, 35, 18, DARKGRAY);
        DrawText("W/S: frente/trás | A/D: esquerda/direita | Q/E: subir/descer | G: grade", 10, 60, 18, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
