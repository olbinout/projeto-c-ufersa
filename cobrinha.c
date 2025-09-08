#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

// --- Constantes do Jogo ---
const int LARGURA_TELA = 800;
const int ALTURA_TELA = 600;
const int TAMANHO_BLOCO = 20; // Tamanho de cada segmento da cobra e da comida
const float FPS = 10.0;     // Velocidade do jogo (10 "passos" por segundo)

// --- Estruturas de Dados ---

// Estrutura para uma posição (x, y) na grade
typedef struct {
    int x;
    int y;
} Ponto;

// Estrutura para a cobra
typedef struct {
    Ponto corpo[300]; // Um array para guardar os segmentos do corpo
    int tamanho;
    int direcao; // 0=Cima, 1=Baixo, 2=Esquerda, 3=Direita
} Cobra;

// Estrutura para a comida
typedef struct {
    Ponto pos;
    bool ativa;
} Comida;


// --- Funções Auxiliares ---

// Gera uma nova posição para a comida
void gerar_comida(Comida *comida) {
    comida->pos.x = (rand() % (LARGURA_TELA / TAMANHO_BLOCO)) * TAMANHO_BLOCO;
    comida->pos.y = (rand() % (ALTURA_TELA / TAMANHO_BLOCO)) * TAMANHO_BLOCO;
    comida->ativa = true;
}


int main() {
    // --- Variáveis do Allegro ---
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *fila_eventos = NULL;
    ALLEGRO_TIMER *timer = NULL;

    // --- Inicialização do Allegro ---
    if (!al_init()) {
        fprintf(stderr, "Falha ao inicializar o Allegro!\n");
        return -1;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Falha ao inicializar o teclado!\n");
        return -1;
    }

    if (!al_init_primitives_addon()) {
        fprintf(stderr, "Falha ao inicializar o add-on de primitivas!\n");
        return -1;
    }

    // Criar o timer do jogo
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        fprintf(stderr, "Falha ao criar o timer!\n");
        return -1;
    }

    // Criar a janela
    display = al_create_display(LARGURA_TELA, ALTURA_TELA);
    if (!display) {
        fprintf(stderr, "Falha ao criar a janela!\n");
        al_destroy_timer(timer);
        return -1;
    }
    al_set_window_title(display, "Jogo da Cobrinha com Allegro 5");

    // Criar a fila de eventos
    fila_eventos = al_create_event_queue();
    if (!fila_eventos) {
        fprintf(stderr, "Falha ao criar a fila de eventos!\n");
        al_destroy_display(display);
        al_destroy_timer(timer);
        return -1;
    }

    // Registrar as fontes de eventos
    al_register_event_source(fila_eventos, al_get_display_event_source(display));
    al_register_event_source(fila_eventos, al_get_keyboard_event_source());
    al_register_event_source(fila_eventos, al_get_timer_event_source(timer));

    // --- Inicialização do Jogo ---
    srand(time(NULL)); // Semente para números aleatórios

    Cobra cobra;
    cobra.tamanho = 3;
    cobra.direcao = 3; // Começa indo para a direita
    cobra.corpo[0] = (Ponto){100, 100}; // Cabeça
    cobra.corpo[1] = (Ponto){80, 100};
    cobra.corpo[2] = (Ponto){60, 100};

    Comida comida;
    gerar_comida(&comida);

    bool rodando = true;
    bool redesenhar = true;

    // Iniciar o timer
    al_start_timer(timer);

    // --- Loop Principal do Jogo ---
    while (rodando) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(fila_eventos, &ev);

        // --- Lógica de Eventos ---

        // Se o evento for do timer, atualizamos a lógica do jogo
        if (ev.type == ALLEGRO_EVENT_TIMER) {
            // Mover o corpo da cobra
            for (int i = cobra.tamanho - 1; i > 0; i--) {
                cobra.corpo[i] = cobra.corpo[i - 1];
            }

            // Mover a cabeça da cobra de acordo com a direção
            switch(cobra.direcao) {
                case 0: cobra.corpo[0].y -= TAMANHO_BLOCO; break; // Cima
                case 1: cobra.corpo[0].y += TAMANHO_BLOCO; break; // Baixo
                case 2: cobra.corpo[0].x -= TAMANHO_BLOCO; break; // Esquerda
                case 3: cobra.corpo[0].x += TAMANHO_BLOCO; break; // Direita
            }

            // --- Verificação de Colisões ---

            // Colisão com as paredes
            if (cobra.corpo[0].x < 0 || cobra.corpo[0].x >= LARGURA_TELA ||
                cobra.corpo[0].y < 0 || cobra.corpo[0].y >= ALTURA_TELA) {
                rodando = false;
            }

            // Colisão com o próprio corpo
            for (int i = 1; i < cobra.tamanho; i++) {
                if (cobra.corpo[0].x == cobra.corpo[i].x && cobra.corpo[0].y == cobra.corpo[i].y) {
                    rodando = false;
                }
            }

            // Colisão com a comida
            if (cobra.corpo[0].x == comida.pos.x && cobra.corpo[0].y == comida.pos.y) {
                cobra.tamanho++;
                gerar_comida(&comida);
            }

            redesenhar = true;
        }
        // Se o evento for o fechamento da janela
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            rodando = false;
        }
        // Se o evento for uma tecla pressionada
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP:
                    if (cobra.direcao != 1) cobra.direcao = 0;
                    break;
                case ALLEGRO_KEY_DOWN:
                    if (cobra.direcao != 0) cobra.direcao = 1;
                    break;
                case ALLEGRO_KEY_LEFT:
                    if (cobra.direcao != 3) cobra.direcao = 2;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    if (cobra.direcao != 2) cobra.direcao = 3;
                    break;
                case ALLEGRO_KEY_ESCAPE:
                    rodando = false;
                    break;
            }
        }

        // --- Lógica de Desenho ---
        if (redesenhar && al_is_event_queue_empty(fila_eventos)) {
            redesenhar = false;
            al_clear_to_color(al_map_rgb(0, 0, 0)); // Limpa a tela com preto

            // Desenhar a cobra
            for (int i = 0; i < cobra.tamanho; i++) {
                al_draw_filled_rectangle(
                    cobra.corpo[i].x, cobra.corpo[i].y,
                    cobra.corpo[i].x + TAMANHO_BLOCO, cobra.corpo[i].y + TAMANHO_BLOCO,
                    al_map_rgb(0, 255, 0) // Verde
                );
            }
            // Desenhar a cabeça com uma cor diferente para destaque
             al_draw_filled_rectangle(
                    cobra.corpo[0].x, cobra.corpo[0].y,
                    cobra.corpo[0].x + TAMANHO_BLOCO, cobra.corpo[0].y + TAMANHO_BLOCO,
                    al_map_rgb(100, 255, 100) // Verde claro
                );

            // Desenhar a comida
            al_draw_filled_rectangle(
                comida.pos.x, comida.pos.y,
                comida.pos.x + TAMANHO_BLOCO, comida.pos.y + TAMANHO_BLOCO,
                al_map_rgb(255, 0, 0) // Vermelho
            );

            al_flip_display(); // Atualiza a tela
        }
    }

    // --- Finalização ---
    printf("Fim de Jogo! Pontuação: %d\n", cobra.tamanho - 3);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(fila_eventos);

    return 0;
}
