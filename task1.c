#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

enum {N = 5};

long
file_size(FILE *fp)
{
    long finish, cur;
    if ((cur = ftell(fp)) == -1) {
        goto Err_size;
    }
    if (fseek(fp, 0, SEEK_END) == -1) {
        goto Err_size;
    }
    if ((finish = ftell(fp)) == -1) {
        goto Err_size;
    }
    if (fseek(fp, cur, SEEK_SET) == -1) {
        goto Err_size;
    }
    return finish;
    Err_size:
        perror("FILE_SIZE");
        fclose(fp);
        exit(1);
}

/// Add '\n' to the last line if there isn't one
void
add_eol(FILE *fp)
{
    long c, start_pos = ftell(fp);
    if (fseek(fp, -1, SEEK_END) == -1) {
        goto Err_eol;
    }
    if ((c = fgetc(fp)) != '\n') {
        fputc('\n', fp);
    }
    if (fseek(fp, start_pos, SEEK_SET) == -1) {
        goto Err_eol;
    }
    return;
    Err_eol:
        perror("ADD EOL");
        fclose(fp);
        exit(1);
}

int
check(FILE *fp, long *pos_start, long *len)
{
    long fin;
    int flag_nspace = 1;
    char str[N];
    if (!fp) {
        goto Err1;
    }
    if ((*pos_start = ftell(fp)) < 0) {
        goto Err1;
    }
    fin = file_size(fp);
    while (1) {
        if (!fgets(str, N, fp)) {
            goto Err1;
        }
        if ((strlen(str) < N - 1) || ((strlen(str) == N -1) && (str[N - 2] == '\n')) ||
                ((strlen(str) == N - 1) && (ftell(fp) == fin))) {                  /* End of line */
            *len = ftell(fp) - *pos_start;      //check
            if (strchr(str, '\n')) {
                *len -= 1;
            }
            if (strchr(str, ' ')) {
                return 0;
            }
            return flag_nspace;
        }
        if (strchr(str, ' ')) {
            flag_nspace = 0;
        }
    }
    Err1:
        perror("Error in check");
        fclose(fp);
        exit(1);
}

void
change(FILE *fp, long from, long to, long len, char *buf_ch, char *buf_last)
{
    if (fseek(fp, from, SEEK_SET) == -1) {
        goto Err_change;
    }
    fread(buf_ch, sizeof(char), len, fp);
    if (fseek(fp, to, SEEK_SET) == -1) {
        goto Err_change;
    }
    fread(buf_last, sizeof(char), len, fp);
    if (fseek(fp, from, SEEK_SET) == -1) {
        goto Err_change;
    }
    fwrite(buf_last, sizeof(char), len, fp);
    if (fseek(fp, to, SEEK_SET) == -1) {
        goto Err_change;
    }
    fwrite(buf_ch, sizeof(char), len, fp);
    return;
    Err_change:
        perror("CHANGE");
        fclose(fp);
        exit(1);
}

void
swap_equal(FILE *fp, long from, long to, long len)
{
    char buf_ch[N], buf_last[N];
//    long fsize = file_size(fp);
    while (len > N) {
        change(fp, from, to, N, buf_ch, buf_last);
        len -= N;
        to += N;
        from +=N;
    }
    change(fp, from, to, len, buf_ch, buf_last);
}

void
sub_roll_left(FILE *fp, long ch_start, long leng)
{
    long cur, nxt, len;
    char buf[N], tmp[N];
    if (fseek(fp, ch_start, SEEK_SET) == -1) {
        goto Err_sub_left;
    }
    if ((cur = ftell(fp)) == -1) {
        goto Err_sub_left;
    }
    fread(buf, sizeof(char), leng, fp);
    while ((len = fread(tmp, sizeof(char), leng, fp)) != 0) {
        if ((nxt = ftell(fp)) == -1) {
            goto Err_sub_left;
        }
        if (fseek(fp, cur, SEEK_SET) == -1) {
            goto Err_sub_left;
        }
        fwrite(tmp, sizeof(char), len, fp);
        if ((cur = ftell(fp)) == -1) {
            goto Err_sub_left;
        }
        if (fseek(fp, nxt, SEEK_SET) == -1) {
            goto Err_sub_left;
        }
    }
    if (fseek(fp, cur, SEEK_SET) == -1) {
        goto Err_sub_left;
    }
    fwrite(buf, sizeof(char), leng, fp);
    return;
    Err_sub_left:
        perror("SUB_ROLL_LEFT");
        fclose(fp);
        exit(1);
}

void
roll_left(FILE *fp, long ch_start, long dif_len)
{
    while (dif_len > N) {
        sub_roll_left(fp, ch_start, N);
        dif_len -= N;
        if (fseek(fp, ch_start, SEEK_SET) == -1) {
            goto Err_roll_left;
        }
    }
    if (dif_len != 0) {
        sub_roll_left(fp, ch_start, dif_len);
    }
    return;
    Err_roll_left:
        perror("ROLL_LEFT");
        fclose(fp);
        exit(1);
}

void
roll_right(FILE *fp, long start, long dif_len) {
    char buf[N], tmp[N];
    long dif;
    while (dif_len >= N) {
        fseek(fp, -N, SEEK_END);
        fread(buf, sizeof(char), N, fp);
        fseek(fp, -N, SEEK_END);
        while ((ftell(fp) - N) >= start) {
            fseek(fp, ftell(fp) -N, SEEK_SET);
            fread(tmp, sizeof(char), N, fp);
            fwrite(tmp, sizeof(char), N,  fp);
            fseek(fp, ftell(fp) - 2 * N, SEEK_SET);
        }
        // dif can become unlimited. You should divide the following copy
        // to limited ones
        dif = ftell(fp) /*+ N*/ - start; // -1:         !!!!!!!!!!!!!!!!!!
        // fp points to the first already read symbol
        if (dif) {
            fseek(fp, start, SEEK_SET);
            // FIXME: `dif` bytes may don't fit in `tmp`        !!!!!!!!!!
            // it happens with the example
            fread(tmp, sizeof(char), dif, fp);
            fseek(fp, start + N, SEEK_SET);
            fwrite(tmp, sizeof(char), dif, fp);
/*            fseek(fp, ftell(fp) + N - dif, SEEK_SET);
            fwrite(tmp, sizeof(char), dif, fp); */
        }
        fseek(fp, start, SEEK_SET);
        fwrite(buf, sizeof(char), N, fp);
        dif_len -= N;
    }
//    assert(dif_len > 0);
    if (dif_len) {
//        printf("%ld**********\n", dif_len);
        fseek(fp, -dif_len, SEEK_END);      //ТУТ Я СЧИТЫВАЮ С \n, ХОТЯ НЕ УВЕРЕН, ЧТО ОН НУЖЕН
        fread(buf, sizeof(char), dif_len, fp);
        fseek(fp, ftell(fp) - dif_len, SEEK_SET);
        while ((ftell(fp) - dif_len) >= start) {
            fseek(fp, ftell(fp) - dif_len, SEEK_SET);
            fread(tmp, sizeof(char), dif_len, fp);
            fwrite(tmp, sizeof(char), dif_len, fp);
            fseek(fp, ftell(fp) - 2 * dif_len, SEEK_SET);
        }
        dif = ftell(fp) + dif_len - start;
        if (dif) {
            fseek(fp, start, SEEK_SET);
            fread(tmp, sizeof(char), dif, fp);
            fseek(fp, start + dif_len, SEEK_SET);
            fwrite(tmp, sizeof(char), dif, fp);
/*            fseek(fp, ftell(fp) + dif_len - dif, SEEK_SET);
            fwrite(tmp, sizeof(char), dif, fp); */
        }
        fseek(fp, start, SEEK_SET);
        fwrite(buf, sizeof(char), dif_len, fp);
    }
}


void
on_screen(FILE *fp, long start, long len)
{
    char str[N];
    long pos = ftell(fp);
    if (fseek(fp, start, SEEK_SET) == -1) {
        goto Err_screen;
    }
    while (len > 0) {
        if (fgets(str, N, fp) == NULL) {
            goto Err_screen;
        }
        len -= strlen(str);
        printf("%s", str);
    }
    fseek(fp, pos, SEEK_SET);
    return;
    Err_screen:
        perror("ON SCREEN");
        fclose(fp);
        exit(1);
}

int
main(int argc, char *argv[])
{
    FILE *fp;
    int flag;
    long ch_start = -1;
    long ch_len = -1;
    long last_start = -1;
    long last_len = -1;
    long fin_pos = -1;
    long cur = -1;
    long mur = -1;
    if (argc < 2) {
        fprintf(stderr, "No arguements");
        return 1;
    }
    fp = fopen(argv[1], "r+");
    if (!fp) {
        perror("FOPEN");
        return 1;
    }
    add_eol(fp);
    if (fseek(fp, 0, SEEK_END) == -1) {
        perror("FSEEK");
        fclose(fp);
        return 1;
    }
    fin_pos = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) {
        perror("FSEEK");
        fclose(fp);
        return 1;
    }
    while (ftell(fp) < fin_pos) {
        flag = check(fp, &last_start, &last_len);
        if (flag) {
            ch_start = last_start;
            ch_len = last_len;
        }
    }
    if (ch_start == -1) {
        printf("no non-space strings");
        fclose(fp);
        return 0;
    }
    if (ch_start == last_start) {
        printf("THE LAST STRING IS THE NEEDED STRING\n");
        on_screen(fp, ch_start, ch_len);
        fclose(fp);
        return 0;
    }

    printf("THE NEEDED STRING IS\n");
    on_screen(fp, ch_start, ch_len);
    printf("WE FOUNd LAST STRING:\n");
    on_screen(fp, last_start, last_len); 

    if (ch_len == last_len) {
        printf("THEY ARE EQUAL\n");
        swap_equal(fp, ch_start, last_start, ch_len);
    } else {
        if (ch_len > last_len) {
            swap_equal(fp, ch_start, last_start, last_len + 1);
            roll_left(fp, ch_start + last_len + 1, ch_len - last_len);
        }
        else {
            swap_equal(fp, ch_start, last_start, ch_len + 1);
            char m[2];
            fread(m, sizeof(char), 1, fp);
            printf("\n\n\n%c\n\n\n", m[0]);
            roll_right(fp, ch_start + ch_len + 1, last_len - ch_len /*- 1*/);
            printf("AAAAAAAAA");
        }
    }
    fclose(fp);
    return 0;
}
