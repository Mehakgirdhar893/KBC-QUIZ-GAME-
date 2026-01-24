#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>

#define MAX_QUES_LEN 300
#define MAX_OPTION_LEN 300
#define TIME_LIMIT 15

typedef struct
{
    char text[MAX_QUES_LEN];
    char option[4][MAX_OPTION_LEN];
    char correct_option;
    int prize_money;
} Question;

int money = 0;
int correct_count = 0;
int wrong_count = 0;

/* Function declarations */
int read_question(const char *file_name, Question **question);
void ask_question(Question question);
void shuffle_questions(Question *questions, int n);

int main()
{
    Question *questions = NULL;
    int no_of_questions;
    int i;
    char nav;
    int total_asked = 0;

    srand(time(NULL));

    no_of_questions = read_question("ques.txt", &questions);
    shuffle_questions(questions, no_of_questions);

    for (i = 0; i < no_of_questions; i++)
    {
        system("cls");
        printf("\nQuestion %d of %d\n", i + 1, no_of_questions);
        ask_question(questions[i]);
        total_asked++;
    

        /* VALID INPUT LOOP */
        do
        {
            printf("\n\nPress N for Next | Q to Quit : ");
            scanf(" %c", &nav);

            if (nav == 'Q' || nav == 'q')
                goto end_quiz;

            if (nav != 'N' && nav != 'n')
                printf("Invalid choice! Please press N or Q.\n");

        } while (nav != 'N' && nav != 'n');
    }

end_quiz:
    system("cls");
    printf("\n===== QUIZ RESULT =====\n");
    printf("Total Questions Asked : %d\n", total_asked);
    printf("Correct Answers       : %d\n", correct_count);
    printf("Wrong Answers         : %d\n", wrong_count);
    printf("Total Prize Won       : %d\n", money);

    free(questions);
    return 0;
}

void shuffle_questions(Question *questions, int n)
{
    int i, j;
    Question temp;

    for (i = n - 1; i > 0; i--)
    {
        j = rand() % (i + 1);
        temp = questions[i];
        questions[i] = questions[j];
        questions[j] = temp;
    }
}

void ask_question(Question question)
{
    char answer = '\0';
    int time_left;

    printf("\n%s", question.text);
    printf("%s", question.option[0]);
    printf("%s", question.option[1]);
    printf("%s", question.option[2]);
    printf("%s", question.option[3]);

    printf("\nYou have %d seconds to answer...\n", TIME_LIMIT);
         
    for (time_left = TIME_LIMIT; time_left > 0; time_left--)
    {
        printf("\rTime Left: %2d seconds ", time_left);
        if (time_left <= 5)
            Beep(1000, 200);

        Sleep(1000);

        if (kbhit())
        {
            answer = getch();
            break;
            
        }
    }

    printf("\n");
   printf("ENTER YOUR CHOICE: ");
    if (answer == '\0')
    {
        wrong_count++;
        printf("Time Over!\n");
        printf("Correct Answer was: %c", question.correct_option);
        return;
    }

    printf("Your Answer: %c\n", answer);

    if (answer == question.correct_option)
    {
        correct_count++;
        money += question.prize_money;
        printf("Correct Answer!\n");
        printf("Prize Won = %d", question.prize_money);
    }
    else
    {
        wrong_count++;
        printf("Wrong Answer!\n");
        printf("Correct Answer was: %c", question.correct_option);
    }
}

int read_question(const char *file_name, Question **question)
{
    FILE *ptr;
    char line[MAX_QUES_LEN];
    int count = 0;
    int i, j;

    ptr = fopen(file_name, "r");
    if (ptr == NULL)
    {
        printf("Unable to open question file!\n");
        exit(1);
    }

    while (fgets(line, sizeof(line), ptr))
        count++;

    int no_of_questions = count / 8;

    *question = (Question *)malloc(no_of_questions * sizeof(Question));
    rewind(ptr);

    for (i = 0; i < no_of_questions; i++)
    {
        fgets((*question)[i].text, MAX_QUES_LEN, ptr);

        for (j = 0; j < 4; j++)
            fgets((*question)[i].option[j], MAX_OPTION_LEN, ptr);

        fgets(line, MAX_QUES_LEN, ptr);
        (*question)[i].correct_option = line[0];

        fgets(line, MAX_QUES_LEN, ptr); // skip unused line

        fgets(line, MAX_QUES_LEN, ptr);
        (*question)[i].prize_money = atoi(line);
    }

    fclose(ptr);
    return no_of_questions;
}
