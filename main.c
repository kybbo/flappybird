#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define DELAY 50000
#define GRAVITY 1
#define JUMP_STRENGTH -3

#define PIPE_GAP 6
#define PIPE_WIDTH 1
#define PIPE_SPACING 30
#define MAX_PIPES 5

#define HIGHSCORE_FILE ".highscore"

typedef struct {
    int y, x;
    int velocity;
} Bird;

typedef struct {
    int x;
    int gap_y;
    int passed;
} Pipe;

Bird bird;
Pipe pipes[MAX_PIPES];
int num_pipes = 0;
int pipe_spawn_timer = 0;

int running = 1;
int paused = 0;
int game_over = 0;
int menu_selection = 0; // 0 = Restart, 1 = Quit
int score = 0;
int highscore = 0;

int max_y, max_x;

// =========================
// Utils
// =========================

void load_highscore() {
    FILE *f = fopen(HIGHSCORE_FILE, "r");
    if (f) {
        fscanf(f, "%d", &highscore);
        fclose(f);
    }
}

void save_highscore() {
    if (score > highscore) {
        FILE *f = fopen(HIGHSCORE_FILE, "w");
        if (f) {
            fprintf(f, "%d", score);
            fclose(f);
        }
    }
}

// =========================
// Init Functions
// =========================

void init_pipes() {
    num_pipes = 0;
    pipe_spawn_timer = 0;
}

void init_game() {
    getmaxyx(stdscr, max_y, max_x);
    bird.y = max_y / 2;
    bird.x = max_x / 4;
    bird.velocity = 0;
    score = 0;
    init_pipes();
}

// =========================
// Countdown
// =========================

void countdown() {
    for (int i = 3; i > 0; --i) {
        clear();
        mvprintw(max_y/2, max_x/2, "%d", i);
        refresh();
        sleep(1);
    }
}

// =========================
// Input Handling
// =========================

void handle_input() {
    int ch = getch();
    if (game_over) {
        if (ch == KEY_UP || ch == KEY_DOWN) {
            menu_selection = !menu_selection;
        } else if (ch == '\n') {
            if (menu_selection == 0) {
                game_over = 0;
                menu_selection = 0;
                init_game();
                countdown();
            } else {
                running = 0;
            }
        }
        return;
    }

    if (ch == ' ' || ch == KEY_UP) {
        bird.velocity = JUMP_STRENGTH;
    } else if (ch == 'p') {
        paused = !paused;
    } else if (ch == 'q') {
        running = 0;
    }
}

// =========================
// Pipe Logic
// =========================

void update_pipes() {
    pipe_spawn_timer++;
    if (pipe_spawn_timer >= PIPE_SPACING) {
        if (num_pipes < MAX_PIPES) {
            pipes[num_pipes].x = max_x;
            pipes[num_pipes].gap_y = rand() % (max_y - PIPE_GAP - 2) + 1;
            pipes[num_pipes].passed = 0;
            num_pipes++;
        }
        pipe_spawn_timer = 0;
    }

    for (int i = 0; i < num_pipes; i++) {
        pipes[i].x--;
    }

    if (num_pipes > 0 && pipes[0].x < -PIPE_WIDTH) {
        for (int i = 1; i < num_pipes; i++) {
            pipes[i - 1] = pipes[i];
        }
        num_pipes--;
    }
}

// =========================
// Update
// =========================

void update() {
    bird.velocity += GRAVITY;
    bird.y += bird.velocity;

    update_pipes();

    for (int i = 0; i < num_pipes; i++) {
        if (!pipes[i].passed && pipes[i].x + PIPE_WIDTH < bird.x) {
            score++;
            pipes[i].passed = 1;
        }
        if (bird.x == pipes[i].x) {
            if (bird.y < pipes[i].gap_y || bird.y > pipes[i].gap_y + PIPE_GAP) {
                save_highscore();
                game_over = 1;
                return;
            }
        }
    }

    if (bird.y >= max_y) {
        bird.y = max_y - 1;
        save_highscore();
        game_over = 1;
    } else if (bird.y < 0) {
        bird.y = 0;
        bird.velocity = 0;
    }
}

// =========================
// Draw
// =========================

void draw() {
    clear();
    if (game_over) {
        mvprintw(max_y/2 - 2, (max_x - 12)/2, "GAME OVER!");
        mvprintw(max_y/2 - 1, (max_x - 20)/2, "Your Score: %d", score);
        mvprintw(max_y/2,     (max_x - 20)/2, "Highscore: %d", highscore);
        if (menu_selection == 0) {
            mvprintw(max_y/2 + 2, (max_x - 10)/2, "> Restart");
            mvprintw(max_y/2 + 3, (max_x - 10)/2, "  Quit");
        } else {
            mvprintw(max_y/2 + 2, (max_x - 10)/2, "  Restart");
            mvprintw(max_y/2 + 3, (max_x - 10)/2, "> Quit");
        }
    } else {
        for (int i = 0; i < num_pipes; i++) {
            for (int y = 0; y < max_y; y++) {
                if (y < pipes[i].gap_y || y > pipes[i].gap_y + PIPE_GAP) {
                    mvprintw(y, pipes[i].x, "|");
                }
            }
        }
        mvprintw(bird.y, bird.x, "O");
        mvprintw(0, max_x - 15, "Score: %d", score);
        mvprintw(0, 0, "SPACE/UP=flap | P=pause | Q=quit");
    }
    refresh();
}

// =========================
// Main
// =========================

int main() {
    initscr();
    noecho();
    cbreak();
    curs_set(FALSE);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    load_highscore();
    init_game();
    countdown();

    while (running) {
        handle_input();
        if (!paused && !game_over) {
            update();
        }
        draw();
        usleep(DELAY);
    }

    endwin();
    printf("Thanks for playing!\n");
    return 0;
}
