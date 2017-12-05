#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#define FORCOMP(x) (void)(x)
//ПОЛНОСТЬЮ РАБОТАЮЩИЙ ВАРИАНТ
enum {
    ANSW_SIZE = 3,
    N_SIZE = 2,
    PAC_SIZE = 5,
    VERY_BIG_SIZE = 1000,
};

static void
sig_c(int s)
{
    FORCOMP(s);
    exit(1);
}

static void
make_exit(int b)
{
    if (b == -1) {
        abort();
    }
}

static void
end_with_err(int r, int err)
{
    if (r == -1) {
        char *msg = strerror(err);
        int w = write(2, msg, strlen(msg));
        make_exit(w);
        exit(1);
    }
}

static int
communication(const char *str)
{
    int i = 0;
    int w = 0;
    int pacs_quan = 0;
    int len = 0;
    char num_b[N_SIZE];
    char str_pacs[PAC_SIZE];
    memset(num_b, 0, N_SIZE);
    memset(str_pacs, 0, PAC_SIZE);
    len = strlen(str);
    pacs_quan = ceil(((double)len)/PAC_SIZE);
    sprintf(num_b, "%d", pacs_quan);
    w = write(1, num_b, N_SIZE);
    end_with_err(w, errno);
    for (i = 0; i < pacs_quan; i++) {
        if (i == pacs_quan - 1) {
            memset(str_pacs, 0, PAC_SIZE);
            memcpy(str_pacs, str + i * PAC_SIZE, strlen(str) - i * PAC_SIZE);
            w = write(1, str_pacs, PAC_SIZE);
            end_with_err(w, errno);
        } else {
            memset(str_pacs, 0, PAC_SIZE);
            memcpy(str_pacs, str + i * PAC_SIZE, PAC_SIZE);
            w = write(1, str_pacs, PAC_SIZE);
            end_with_err(w, errno);
        }
    }
    return 0;
}

static int
choose_mode(const char *your_command, const char *mode[])
{
//прогоняем нашу команду и смотрим, есть ли такая. если есть - возвращаем ее номер в массиве команд
    int i = 0;
    //int cur_mode = -1;
    for (i = 0; i < PAC_SIZE; i++) {
        if (memcmp(your_command, mode[i], PAC_SIZE) == 0) {
            return i;
        }
    }
    const char *wrong_command = "There is no such a command\n";
    int w = write(2, wrong_command, strlen(wrong_command));
    end_with_err(w, errno);
    return -1;
}

int
main(void)
{
    const char *theme = "This is Math test\n";
    const char *mas_of_questions[] = {"How much is 1 + 1?\n", "What is the theme of this test?\n", "What has 3 sides?\n"};
    const char *mas_of_answers[] = {"2\n", "the theme of this test is math\n", "triangle\n"};
    const char *mode[] = {"gett\n", "getq\n", "cmpa\n", "getn\n", "stop\n"};
    const char *right = "Right\n", *wrong = "Wrong\n";
    int r = 0;
    int cur_mode = 0;
    char your_command[PAC_SIZE + 1];
    char number[PAC_SIZE + 1];
    char your_answer[VERY_BIG_SIZE + 1];
    signal(SIGINT, sig_c);

    while (1) {
        memset(your_command, 0, PAC_SIZE + 1);
        fgets(your_command, PAC_SIZE + 1, stdin);
        if ((cur_mode = choose_mode(your_command, mode)) == 0) {
            communication(theme);
        }
        //дальше идет выдача вопроса №...
        else if (cur_mode == 1) {
            memset(number, 0, PAC_SIZE + 1);
            fgets(number, PAC_SIZE, stdin);
            int quest_num = strtol(number, NULL, 10) - 1;
                if (quest_num < 0 || quest_num > 2) {
                return 1;
            }
            communication(mas_of_questions[quest_num]);
        }
        //дальше идет сравнение ответа с вопросом №...
        else if (cur_mode == 2) {
            int flag_ans = 0;
        	char buf_n[ANSW_SIZE] = {};
            fgets(buf_n, PAC_SIZE, stdin);
           	int ans_num = strtol(buf_n, NULL, 10) - 1;   //нужна проверка
            if (ans_num < 0 || ans_num > 2) {
                return 1;
            }
            memset(your_answer, 0, VERY_BIG_SIZE + 1);
            if (fgets(your_answer, VERY_BIG_SIZE, stdin) != NULL) {
                if (strlen(your_answer) == strlen(mas_of_answers[ans_num])) {
                    if (memcmp(your_answer, mas_of_answers[ans_num], strlen(mas_of_answers[ans_num])) == 0) {
                        flag_ans = 1;
                    }
                }
            }
            if (flag_ans) {
                communication(right);
            } else {
                communication(wrong);
            }
        }
        //дальше идет выдача количества вопросов
        else if (cur_mode == 3) {
            char str[N_SIZE];
            memset(str, 0, N_SIZE);
            sprintf(str, "%d", ANSW_SIZE);
            communication(str);
        }
        //дальше идет завершение теста
        else if (cur_mode == 4) {
            close(0);
            close(1);
            break;
        }
    }
    return 0;
}
