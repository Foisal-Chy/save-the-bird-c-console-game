#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define PIPE_COUNT 3
#define PIPE_SPACING 35
#define PIPE_WIDTH 4
#define GAP_HEIGHT 8
#define PIPE_CHAR 219
#define FLOOR_CHAR 223
#define MAX_SCORE_ENTRIES 5
#define NAME_LENGTH 30
#define SCORE_FILE "scores.dat"

typedef struct{
    char name[NAME_LENGTH];
    int score;
    int time_seconds;
} HighScoreEntry;

/// Global Variables
HANDLE hConsole;
int WIDTH, HEIGHT;
int BIRD_X;
int birdY, velocity, score, gameOver, speedDelay;
int pipeX[PIPE_COUNT],gapY[PIPE_COUNT];
time_t startTime;
HighScoreEntry highScores[MAX_SCORE_ENTRIES];

/// Terminal size measure kora
void getTerminalSize(){
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    WIDTH = csbi.srWindow.Right - csbi.srWindow.Left;///AI
    HEIGHT = csbi.srWindow.Bottom - csbi.srWindow.Top;
    BIRD_X = WIDTH / 5;
}

///Console window maximize kora
void maximizeConsole(){
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_MAXIMIZE);
    Sleep(200);
    getTerminalSize();
}

/// Cursor ke specific position e neya
void gotoxy(int x, int y){
    COORD c ={(SHORT)x, (SHORT)y};
    SetConsoleCursorPosition(hConsole, c);
}

/// Text color change kora
void setColor(int attr){
    SetConsoleTextAttribute(hConsole, attr);
}

/// Color reset kore white e fire jaoa
void resetColor(){
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/// Blinking cursor hide kora
void hideCursor(){ ///AI
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &info);
}

/// Score compare function (sorting er jonno)
int compare_scores(const void *a, const void *b){
    HighScoreEntry* entryA = (HighScoreEntry* )a;
    HighScoreEntry* entryB = (HighScoreEntry* )b;
    return (entryB->score - entryA->score);
}

/// File theke scores load kora
void load_scores(){
    FILE *file = fopen(SCORE_FILE, "rb");
    if (file == NULL){
        for (int i = 0; i < MAX_SCORE_ENTRIES; i++){
            strcpy(highScores[i].name, "Player");
            highScores[i].score = 0;
            highScores[i].time_seconds = 0;
        }
        return;
    }
    fread(highScores, sizeof(HighScoreEntry), MAX_SCORE_ENTRIES, file);
    fclose(file);
    qsort(highScores, MAX_SCORE_ENTRIES, sizeof(HighScoreEntry), compare_scores);
}

/// Scores file e save kora
void save_scores(){
    FILE *file = fopen(SCORE_FILE, "wb");
    if (file != NULL){
        qsort(highScores, MAX_SCORE_ENTRIES, sizeof(HighScoreEntry), compare_scores);
        fwrite(highScores, sizeof(HighScoreEntry), MAX_SCORE_ENTRIES, file);
        fclose(file);
    }
}

/// Notun high score insert kora
void insert_new_score(int final_score, int time_elapsed){
    if (final_score > highScores[MAX_SCORE_ENTRIES - 1].score){
        system("cls");
        setColor(14);
        gotoxy(WIDTH/2 - 15, 8);
        printf("CONGRATULATIONS! NEW HIGH SCORE!");
        setColor(15);
        gotoxy(WIDTH/2 - 15, 10);
        printf("Enter your name (max %d chars): ", NAME_LENGTH - 1);

        char tempName[NAME_LENGTH];

        if (scanf("%29s", tempName) == 1){
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }
        else{
            strcpy(tempName, "Player");
        }

        highScores[MAX_SCORE_ENTRIES - 1].score = final_score;
        highScores[MAX_SCORE_ENTRIES - 1].time_seconds = time_elapsed;
        strcpy(highScores[MAX_SCORE_ENTRIES - 1].name, tempName);

        save_scores();
    }
}

/// Upor-niche border draw kora
void drawBorders(){
    setColor(7);
    for (int x = 0; x <= WIDTH; x++){
        gotoxy(x, 0);
        printf("%c", FLOOR_CHAR);
        gotoxy(x, HEIGHT);
        printf("%c", FLOOR_CHAR);
    }
}

/// Pakhi draw kora
void drawBird(){
    // Lej - holud color
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    gotoxy(BIRD_X, birdY);
    printf("%c", 220);

    // Body - holud color
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    gotoxy(BIRD_X + 1, birdY);
    printf("%c", 219);

    // Thot - lal color
    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    gotoxy(BIRD_X + 2, birdY);
    printf(">");
}

/// Pakhi erase kora
void eraseBird(int y){
    gotoxy(BIRD_X, y);
    printf("   ");
}

/// Pipe draw kora
void drawPipeAt(int px, int gy){
    setColor(2 | FOREGROUND_INTENSITY);
    for (int x = px; x < px + PIPE_WIDTH; x++){
        if (x < 1 || x > WIDTH - 1)
            continue;
        for (int y = 1; y < HEIGHT; y++){
            gotoxy(x, y);
            if (y < gy || y >= gy + GAP_HEIGHT){
                printf("%c", PIPE_CHAR);
            }
            else{
                printf(" ");
            }
        }
    }
}

/// Purano pipe position erase kora
void erasePipeAt(int px){
    if (px >= -4 && px < WIDTH){
        for (int y = 1; y < HEIGHT; y++){
            gotoxy(px + PIPE_WIDTH, y);
            printf(" ");
        }
    }
}

/// Score ar time display kora
void drawStats(){
    setColor(11);
    gotoxy(2, 0);
    printf("%c Score: %03d %c", FLOOR_CHAR, score, FLOOR_CHAR);

    time_t now = time(NULL);
    int elapsed = (int)difftime(now, startTime);
    int min = elapsed / 60;
    int sec = elapsed % 60;

    gotoxy(WIDTH - 15, 0);
    printf("%c Time: %02d:%02d %c", FLOOR_CHAR, min, sec, FLOOR_CHAR);
}

/// Pipes initialize kora
void initPipes()
{
    for (int i = 0; i < PIPE_COUNT; i++)
    {
        pipeX[i] = (WIDTH / 2) + (i * PIPE_SPACING);
        gapY[i] = (rand() % (HEIGHT - GAP_HEIGHT - 3)) + 2;
    }
}

/// Collision check kora
void checkCollision(){
    /// Ceiling ba floor e hit korlo kina
    if (birdY <= 0 || birdY >= HEIGHT){
        gameOver = 1;
        return;
    }
    // Pipe e hit korlo ki na
    for (int i = 0; i < PIPE_COUNT; i++){
        if (BIRD_X + 2 >= pipeX[i] && BIRD_X <= pipeX[i] + PIPE_WIDTH - 1){
            if (birdY < gapY[i] || birdY >= gapY[i] + GAP_HEIGHT){
                gameOver = 1;
                return;
            }
        }
    }
}

/// Game reset kore notun game shuru kora
void resetGame(){
    birdY = HEIGHT / 2;
    velocity = 0;
    score = 0;
    speedDelay = 90;
    gameOver = 0;
    initPipes();
    startTime = time(NULL);

    system("cls");
    drawBorders();
    for (int i = 0; i < PIPE_COUNT; i++) drawPipeAt(pipeX[i], gapY[i]);
    drawBird();
    drawStats();
}

/// Animated text display kora
void draw_animated_text(const char *text, int start_x, int y, int color){
    int len = strlen(text);
    for (int i = 0; i < len; i++){
        setColor(color);
        gotoxy(start_x + i, y);
        printf("%c", text[i]);
        Sleep(50);
    }
}

/// Main menu display kora
void display_main_menu(int *choice){
    int selected = 1;
    char key;

    while (1){
        system("cls");
        setColor(14);
        gotoxy(WIDTH/2 - 14 , 3);
        printf("==============================");
        setColor(11);
        gotoxy(WIDTH/2 - 14, 4);
        printf("      SAVE THE BIRD GAME      ");
        setColor(14);
        gotoxy(WIDTH/2 - 14, 5);
        printf("==============================");

        char *options[] ={"  START  ", " SCOREBOARD ", "   ABOUT   ", "   EXIT    "};
        for (int i = 0; i < 4; i++){
            gotoxy(WIDTH/2 -5 , 8 + i * 2);
            if (i + 1 == selected){
                setColor(BACKGROUND_BLUE | 15);
                printf(">>> %s <<<", options[i]);
            }
            else{
                setColor(15);
                printf("    %s    ", options[i]);
            }
        }

        resetColor();

        key = _getch();
    if (key == 72){

        if (selected > 1)
        selected = selected - 1;

        else
        selected = 4;
        }
    else if (key == 80){

        if (selected < 4)
        selected = selected + 1;

        else
        selected = 1;
        }

    else if (key == 13 || key == 's' || key == 'S'){
            *choice = selected;
            break;
        }
    else if (key == 'e' || key == 'E'){
            *choice = 4;
            break;
        }
    }
}

/// Scoreboard display kora
void display_scoreboard(){
    system("cls");
    setColor(10);
    gotoxy(WIDTH/2 - 10, 3);
    printf("======= SCOREBOARD =======");
    setColor(15);

    load_scores();

    gotoxy(WIDTH/2 - 25, 6);
    printf("Rank | Name                          | Score | Time");
    gotoxy(WIDTH/2 - 25, 7);
    printf("-----|-------------------------------|-------|---------");

    for (int i = 0; i < MAX_SCORE_ENTRIES; i++){
        gotoxy(WIDTH/2 - 25, 8 + i);
        int min = highScores[i].time_seconds / 60;
        int sec = highScores[i].time_seconds % 60;

        printf(" %d   | %-30s| %-5d | %02d:%02d", i + 1, highScores[i].name, highScores[i].score, min, sec);
    }

    setColor(13);
    gotoxy(WIDTH/2 - 20, 15);
    printf("Press any key to return to Main Menu...");
    _getch();
    resetColor();
}

/// About screen display kora
void display_about(){
    system("cls");
    setColor(14);
    gotoxy(WIDTH/2 - 10, 3);
    printf("======= ABOUT THE GAME =======");

    setColor(15);
    gotoxy(10, 6);
    printf("Game: Save The Bird (Flappy Bird Clone)");
    gotoxy(10, 7);
    printf("Developed in: C Programming Language");
    gotoxy(10, 8);
    printf("Version: 1.0");

    setColor(11);
    gotoxy(10, 10);
    printf("HOW TO PLAY:");
    gotoxy(10, 12);
    printf("- Press SPACE to make the bird flap.");
    gotoxy(10, 13);
    printf("- Avoid hitting the green pipes.");
    gotoxy(10, 14);
    printf("- Score points by passing through pipes.");
    gotoxy(10, 15);
    printf("- Game speeds up as you score more!");
    gotoxy(10, 18);
    printf("DEVELOPED BY:");
    gotoxy(10, 20);
    printf("-Naza, ID: C253089");
    gotoxy(10, 21);
    printf("-Towhid, ID: C253094");
    gotoxy(10, 22);
    printf("-Foisal, ID: C253077");

    setColor(13);
    gotoxy(WIDTH/2 - 20, 24);
    printf("Press any key to return to Main Menu...");
    _getch();
    resetColor();
}

/// 3 second countdown dekhano
void start_countdown(){
    for (int i = 3; i > 0; i--){
        system("cls");
        setColor(12);
        gotoxy(WIDTH / 2 - 7, HEIGHT / 2);
        printf("STARTING IN...");
        gotoxy(WIDTH / 2 - 1, HEIGHT / 2 + 1);
        setColor(14);
        printf(" %d ", i);
        Sleep(1000);
    }
    system("cls");
}

/// Game over screen dekhano
void game_over_screen(int final_score, int time_elapsed){
    // Game over animation
    for(int i=0; i<3; i++){
        setColor(12 | FOREGROUND_INTENSITY);
        gotoxy(WIDTH / 2 - 8, HEIGHT / 2);
        printf(">>> GAME OVER <<<");
        Sleep(200);
        gotoxy(WIDTH / 2 - 8, HEIGHT / 2);
        printf("                 ");
        Sleep(200);
    }

    // Result display kora
    setColor(15);
    gotoxy(WIDTH / 2 - 11, HEIGHT / 2 + 2);
    printf("Time Played: %02d:%02d", time_elapsed / 60, time_elapsed % 60);
    gotoxy(WIDTH / 2 - 9, HEIGHT / 2 + 3);
    printf("Final Score: %d", final_score);

    insert_new_score(final_score, time_elapsed);

    // Play again option
    int selected = 1;
    char key;

    while(1){
        Sleep(100);

        gotoxy(WIDTH / 2 - 10, HEIGHT / 2 + 5);
        setColor(14);
        printf("Do you want to play again?");

        gotoxy(WIDTH / 2 - 6, HEIGHT / 2 + 7);
        if (selected == 1) setColor(BACKGROUND_GREEN | 15);
        else setColor(15);
        printf("  YES  ");

        gotoxy(WIDTH / 2 + 2, HEIGHT / 2 + 7);
        if (selected == 2) setColor(BACKGROUND_RED | 15);
        else setColor(15);
        printf("   NO  ");

        key = _getch();
    if (key == 75 || key == 77){
                if (selected == 1)
                selected = 2;
                else
                selected = 1;
        }

    else if (key == 13 || key == 'y' || key == 'Y' || key == 'n' || key == 'N')break;
    }

        resetColor();

    if (selected == 1 || key == 'y' || key == 'Y'){
        // Yes - game restart
        gameOver = 0;
        start_countdown();
        resetGame();

        // Gameplay loop
        while (!gameOver){
            int prevBirdY = birdY;

            // Input handle kora
            if (_kbhit()){
                int ch = _getch();
                if (ch == 32) velocity = -4;
                else if (ch == 'q' || ch == 'Q')
                {
                    gameOver = 1;
                    break;
                }
            }

            /// Physics update kora
            velocity += 1;
            if (velocity > 2) velocity = 2;
            eraseBird(prevBirdY);
            birdY += velocity;

            // Pipe movement ar scoring
            for (int i = 0; i < PIPE_COUNT; i++){
                int oldX = pipeX[i];
                pipeX[i] -= 1;

                // Pipe reset ar score increase
                if (pipeX[i] + PIPE_WIDTH < 1){
                    // Shobcheye dane pipe khuje ber kora
                    int maxX = pipeX[0];
                    for (int j = 1; j < PIPE_COUNT; j++){
                        if (pipeX[j] > maxX)
                            maxX = pipeX[j];
                    }
                    pipeX[i] = maxX + PIPE_SPACING;
                    gapY[i] = (rand() % (HEIGHT - GAP_HEIGHT - 3)) + 2;
                    score++;

                    // Score onujai speed barhano
                    if (score % 5 == 0 && speedDelay > 40) speedDelay -= 5;
                }

                // Purano position mুche fela
                if (oldX != pipeX[i]){
                    erasePipeAt(oldX);
                }
            }

            checkCollision();

            // Screen e draw kora
            for (int i = 0; i < PIPE_COUNT; i++) drawPipeAt(pipeX[i], gapY[i]);
            drawBird();
            drawStats();

            Sleep(speedDelay);
        }

        // Abar game over hole recursive call
        if (gameOver){
            int new_time_elapsed = (int)difftime(time(NULL), startTime);
            game_over_screen(score, new_time_elapsed);
        }
    }
    else{
        //final score dekhiye main menu te fire jaoa
        system("cls");
        setColor(14);
        gotoxy(WIDTH / 2 - 10, HEIGHT / 2 - 2);
        printf("======================");
        gotoxy(WIDTH / 2 - 10, HEIGHT / 2 - 1);
        printf("   GAME ENDED");
        gotoxy(WIDTH / 2 - 10, HEIGHT / 2);
        printf("======================");

        setColor(15);
        gotoxy(WIDTH / 2 - 11, HEIGHT / 2 + 2);
        printf("Time Played: %02d:%02d", time_elapsed / 60, time_elapsed % 60);
        gotoxy(WIDTH / 2 - 9, HEIGHT / 2 + 3);
        printf("Final Score: %d", final_score);

        setColor(13);
        gotoxy(WIDTH / 2 - 15, HEIGHT / 2 + 5);
        printf("Press any key to return to Main Menu...");

        _getch();
        resetColor();
        gameOver = 0;
    }
}

/// Main function
int main(){
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    hideCursor();

    maximizeConsole();

    // Startup animation
    system("cls");
    draw_animated_text("SAVE THE BIRD", WIDTH/2 - 7, 5, 10 | FOREGROUND_INTENSITY);
    Sleep(1000);

    // Main menu loop
    int menu_choice;
    load_scores();

    while (1){
        display_main_menu(&menu_choice);
        switch (menu_choice){
        case 1:
            //game shuru kora
            start_countdown();
            resetGame();

            // Gameplay loop
            while (!gameOver){
                int prevBirdY = birdY;

                // Input
                if (_kbhit()){
                    int ch = _getch();
                    if (ch == 32)
                        velocity = -4;
                    else if (ch == 'q' || ch == 'Q'){
                        gameOver = 1;
                        break;
                    }
                }
                // Physics
                velocity += 1;
                if (velocity > 2)
                velocity = 2;
                eraseBird(prevBirdY);
                birdY += velocity;

                // Pipe movement
                for (int i = 0; i < PIPE_COUNT; i++){
                    int oldX = pipeX[i];
                    pipeX[i] -= 1;

                    if (pipeX[i] + PIPE_WIDTH < 1){
                        int maxX = pipeX[0];
                        for (int j = 1; j < PIPE_COUNT; j++){
                            if (pipeX[j] > maxX) maxX = pipeX[j];
                        }
                        pipeX[i] = maxX + PIPE_SPACING;
                        gapY[i] = (rand() % (HEIGHT - GAP_HEIGHT - 3)) + 2;
                        score++;

                        if (score % 5 == 0 && speedDelay > 40) speedDelay -= 5;
                    }

                    if (oldX != pipeX[i]){
                        erasePipeAt(oldX);
                    }
                }

                // Collision
                checkCollision();

                // Render
                for (int i = 0; i < PIPE_COUNT; i++) drawPipeAt(pipeX[i], gapY[i]);
                drawBird();
                drawStats();
                Sleep(speedDelay);
            }

            // Game over hole
            if (gameOver){
                int time_elapsed = (int)difftime(time(NULL), startTime);
                game_over_screen(score, time_elapsed);
            }
            break;

        case 2:
            // SCOREBOARD dekhano
            display_scoreboard();
            break;

        case 3:
            // ABOUT dekhano
            display_about();
            break;

        case 4:
            // EXIT
            system("cls");
            setColor(11);
            gotoxy(WIDTH / 2 - 10, HEIGHT / 2);
            printf("Thanks for playing SAVE THE BIRD!");
            setColor(15);
            gotoxy(WIDTH / 2 - 4, HEIGHT / 2 + 1);
            printf("   Goodbye!");
            Sleep(1000);
            resetColor();
            return 0;
        }
    }
}
