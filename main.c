/* kbcsim.c
   Compile in Dev-C++ (MinGW) on Windows.
   File format expected (each question = 8 lines):
   1) question text
   2) option A text
   3) option B text
   4) option C text
   5) option D text
   6) correct option (single char like A or B)
   7) timeout in seconds (integer)
   8) prize money (integer)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>   // for Sleep, console color, Clear screen
#include <conio.h>     // for getch()
#include <ctype.h>

#define MAX_QUES_LEN 300
#define MAX_OPTION_LEN 300
#define FONT_H 7
#define FONT_W 5
#define LETTER_COUNT 6


typedef struct {
    char text[MAX_QUES_LEN];
    char option[4][MAX_OPTION_LEN];
    char correct_option;    // 'A','B','C','D'
    int timeout;            // seconds
    int prize_money;
} Question;

void clear_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

void set_color(WORD attr) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, attr);
}

void cls() {
    // portable-ish clear for Windows console
    system("cls");
}

/* Trim trailing newline and carriage return */
void trim_newline(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

/* Read questions from file. Returns number of questions, and sets *questions to allocated array.
   Caller must free(*questions) when done. */
int read_questions(const char *file_name, Question **questions_out) {
    FILE *ptr = fopen(file_name, "r");
    if (!ptr) {
        fprintf(stderr, "Unable to open question bank file '%s'\n", file_name);
        return 0;
    }

    char buf[MAX_QUES_LEN];
    int lines = 0;
    while (fgets(buf, sizeof(buf), ptr)) lines++;
    if (lines == 0) {
        fclose(ptr);
        return 0;
    }

    if (lines % 8 != 0) {
        // not exact multiple of 8 lines -> warn but attempt best-effort
        // we'll compute floor(lines/8)
    }
    int no_of_questions = lines / 8;
    rewind(ptr);

    Question *questions = (Question *) malloc(no_of_questions * sizeof(Question));
    if (!questions) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(ptr);
        return 0;
    }
     int i;
    for ( i = 0; i < no_of_questions; ++i) {
        if (!fgets(questions[i].text, MAX_QUES_LEN, ptr)) { questions[i].text[0] = '\0'; }
        trim_newline(questions[i].text); 
        int j;
        for (j = 0; j < 4; ++j) {
            if (!fgets(questions[i].option[j], MAX_OPTION_LEN, ptr)) questions[i].option[j][0] = '\0';
            trim_newline(questions[i].option[j]);
        }
        char option_line[32] = {0};
        if (!fgets(option_line, sizeof(option_line), ptr)) option_line[0] = '\0';
        trim_newline(option_line);
        questions[i].correct_option = (option_line[0] == '\0') ? '\0' : option_line[0];

        char number[32] = {0};
        if (!fgets(number, sizeof(number), ptr)) number[0] = '\0';
        trim_newline(number);
        questions[i].timeout = atoi(number);

        char prize[32] = {0};
        if (!fgets(prize, sizeof(prize), ptr)) prize[0] = '\0';
        trim_newline(prize);
        questions[i].prize_money = atoi(prize);
    }

    fclose(ptr);
    *questions_out = questions;
    return no_of_questions;
}

/* Print one question with some coloring. user_choice: '\0' if not answered, else 'A'..'D' */
void print_question_colored(const Question *q, int index, int total, char user_choice) {
    // header
    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("\nQuestion %d of %d\n", index + 1, total);
    set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    printf("%s\n\n", q->text);

    // options
    const char opts[4] = {'A','B','C','D'};
    int i;
	for (i = 0; i < 4; ++i) {
        // if user selected, highlight
        if (user_choice == opts[i]) {
            set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
            printf("-> %c) %s  [your choice]\n", opts[i], q->option[i]);
        } else {
            set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // yellow-ish
            printf("   %c) %s\n", opts[i], q->option[i]);
        }
    }

    set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    printf("\nCorrect option: (hidden until submit)\n");
    set_color(FOREGROUND_INTENSITY);
    printf("Timeout: %d seconds   Prize: %d\n", q->timeout, q->prize_money);
    // reset to normal
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/* -- LED scrolling WELCOME board: draws 7x5 font and scrolls right->left in red -- */

/* 5x7 font for characters we need: W,E,L,C,O,M,E (we'll map only these letters uppercase) */
const unsigned char font5x7[LETTER_COUNT][FONT_H] = {
    // W (index 0)
    {0x11,0x11,0x11,0x15,0x15,0x0A,0x0A},
    // E (1)
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},
    // L (2)
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},
    // C (3)
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},
    // O (4)
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},
    // M (5)
    {0x11,0x1B,0x15,0x15,0x11,0x11,0x11}
    // We'll reuse E for last E
};

int char_index_for(char ch) {
    if (ch == 'W') return 0;
    if (ch == 'E') return 1;
    if (ch == 'L') return 2;
    if (ch == 'C') return 3;
    if (ch == 'O') return 4;
    if (ch == 'M') return 5;
    return -1;
}

void scroll_led_welcome(int width) {
    // Build message columns for " WELCOME " (spaces around)
    const char *msg = " WELCOME ";
    int msg_len = (int)strlen(msg);

    // Create a dynamic columns array: for each letter produce 6 columns (5 font + 1 gap)
    int total_cols = 0;
    int *cols = NULL;
    for (int i = 0; i < msg_len; ++i) {
        int idx = char_index_for(msg[i]);
        if (idx == -1) {
            // space -> 1 blank column
            cols = (int*)realloc(cols, sizeof(int) * (total_cols + 1));
            cols[total_cols++] = 0;
        } else {
            // 5 columns from font bits
            for (int c = 0; c < FONT_W; ++c) {
                unsigned char col_bits = 0;
                for (int r = 0; r < FONT_H; ++r) {
                    // font5x7[idx][r] is a pattern but constructed as bytes where bits 0..4 represent the row?
                    // In our stored small font we used values for ease: treat each row byte as bitmask; extract bit c
                    if ( (font5x7[idx][r] >> (FONT_W - 1 - c)) & 1 ) {
                        col_bits |= (1 << r);
                    }
                }
                cols = (int*)realloc(cols, sizeof(int) * (total_cols + 1));
                cols[total_cols++] = col_bits;
            }
            // gap column
            cols = (int*)realloc(cols, sizeof(int) * (total_cols + 1));
            cols[total_cols++] = 0;
        }
    }

    // Add extra blank columns at end for smooth scroll
    int k;
	for ( k = 0; k < width; ++k) {
        cols = (int*)realloc(cols, sizeof(int) * (total_cols + 1));
        cols[total_cols++] = 0;
    }

    // Scroll from col 0 to total_cols - width
    int offset;
	for ( offset = 0; offset <= total_cols - width; ++offset) {
        cls();
        // print the LED board of size width x FONT_H
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        // draw top margin
        printf("\n\n");
        int row;
	for ( row = 0; row < FONT_H; ++row) {
            int col;
	for ( col = 0; col < width; ++col) {
                int cidx = offset + col;
                int bits = cols[cidx];
                int on = (bits >> row) & 1;
                if (on) {
                    // red LED dot (bright)
                    SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
                    printf("O");
                } else {
                    // dark dot (low intensity)
                    SetConsoleTextAttribute(h, 0); // black
                    printf(".");
                }
                // small space for separation
                SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                printf(" ");
            }
            printf("\n");
        }
        SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        Sleep(80);
    }

    free(cols);
    cls();
}

/* Main interactive quiz loop:
   - allows prev/next/jump/answer/submit
   - user choices kept until submit
*/
int main() {
    Question *questions = NULL;
    int no_of_questions = read_questions("ques.txt", &questions);
    if (no_of_questions <= 0) {
        printf("No questions loaded. Ensure ques.txt exists with correct format.\n");
        return 1;
    }

    char *user_choice = (char*) malloc(no_of_questions);
    if (!user_choice) {
        fprintf(stderr, "Memory error\n");
        free(questions);
        return 1;
    }
    for (int i = 0; i < no_of_questions; ++i) user_choice[i] = '\0';

    int current = 0;
    int exit_flag = 0;
    while (!exit_flag) {
        cls();
        print_question_colored(&questions[current], current, no_of_questions, user_choice[current]);

        // menu
        set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("\nMenu:\n");
        set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
        printf("1) Previous    2) Next    3) Answer/Change    4) Jump to Q    5) Show LED Welcome    6) Submit and Finish\n");
        set_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        printf("Enter choice (1-6): ");
        int m = 0;
        if (scanf("%d", &m) != 1) { clear_stdin_line(); continue; }

        switch (m) {
            case 1:
                if (current > 0) current--;
                else {
                    set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    printf("\nAlready at first question. Press any key...");
                    getch();
                }
                break;
            case 2:
                if (current < no_of_questions - 1) current++;
                else {
                    set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    printf("\nAlready at last question. Press any key...");
                    getch();
                }
                break;
            case 3: {
                // answer
                clear_stdin_line();
                set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                printf("\nEnter option (A-D) or X to clear: ");
                char c = (char) toupper(getchar());
                clear_stdin_line();
                if (c >= 'A' && c <= 'D') {
                    user_choice[current] = c;
                } else if (c == 'X') {
                    user_choice[current] = '\0';
                } else {
                    set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    printf("Invalid option. Press any key...");
                    getch();
                }
                break;
            }
            case 4: {
                set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                printf("\nEnter question number to jump to (1 - %d): ", no_of_questions);
                int qn;
                if (scanf("%d", &qn) == 1) {
                    if (qn >= 1 && qn <= no_of_questions) current = qn - 1;
                    else { set_color(FOREGROUND_RED | FOREGROUND_INTENSITY); printf("Invalid number. Press any key..."); getch(); }
                }
                clear_stdin_line();
                break;
            }
            case 5:
                // show LED scrolling board (width 40 columns)
                scroll_led_welcome(40);
                break;
            case 6:
                // submit and finish
                exit_flag = 1;
                break;
            default:
                set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
                printf("Invalid menu. Press any key...");
                getch();
        }
    }

    // compute results
    int correct = 0;
    int wrong = 0;
    int total_prize = 0;
    set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("\n---- Results ----\n\n");
    for (int i = 0; i < no_of_questions; ++i) {
        char uc = user_choice[i];
        char corr = questions[i].correct_option;
        printf("Q%d: ", i+1);
        if (uc == '\0') {
            set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
            printf("Not answered. ");
            printf("Correct: %c\n", corr);
            wrong++;
        } else if (uc == corr) {
            set_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            printf("Correct (You: %c). Prize +%d\n", uc, questions[i].prize_money);
            correct++;
            total_prize += questions[i].prize_money;
        } else {
            set_color(FOREGROUND_RED | FOREGROUND_INTENSITY);
            printf("Wrong (You: %c). Correct: %c\n", uc, corr);
            wrong++;
        }
        set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    printf("\nTotal Correct: %d\nTotal Wrong/Unanswered: %d\nTotal Prize Money Won: %d\n", correct, wrong, total_prize);

    free(questions);
    free(user_choice);
    set_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return 0;
} 
