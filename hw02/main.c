#include "ctype.h"
#include <stdio.h>

const int CARDS_HAND = 2;
const int CARDS = 7;

int loadCards(int val[CARDS_HAND][CARDS], char col[CARDS_HAND][CARDS]);

char match_result(int val1[], char col1[], int val2[], char col2[]);

int duplicates(int val[CARDS], char col[CARDS]);

void sort(int val[], char col[]);

char highCard(const int val1[], const int val2[], int n, int ignore, int ignore2);

char highCard_color(const int val1[], const int val2[], char col1[], char col2[], char c);

int four_of_kind(int val[]);

int three_of_kind(int val[]);

int pair(int val[], int ignore);

int straight(int val[]);

char flush(char col[]);

char most_color(const char colors[]);

int straightflush(int val[], char col[]);

int main(void)
{
    int values[CARDS_HAND][CARDS];
    char colors[CARDS_HAND][CARDS];
    while (loadCards(values, colors) != 0) {
        sort(values[0], colors[0]);
        sort(values[1], colors[1]);
        printf("%c\n", match_result(values[0], colors[0], values[1], colors[1]));
    }
}

int loadCards(int val[CARDS_HAND][CARDS], char col[CARDS_HAND][CARDS])
{
    char d;
    char c;
    char n;
    int card = 0;

    for (int i = 0; i < 9; ++i) {
        if (scanf(" %1c%1c%c", &d, &c, &n) == EOF)
            return 0;
        if (c != 'h' && c != 'd' && c != 's' && c != 'c') {
            fprintf(stderr, "%c: invalid color\n", c);
            return 0;
        }
        if (i == 1 || i == 3 || i == 8) {
            if (n != '\n') {
                fprintf(stderr, "Missing new line\n");
                return 0;
            }
        } else if (!isspace(n)) {
            fprintf(stderr, "Missing white space\n");
            return 0;
        }
        if (d >= '2' && d <= '9')
            card = d - '0';
        else if (d == 'T')
            card = 10;
        else if (d == 'J')
            card = 11;
        else if (d == 'Q')
            card = 12;
        else if (d == 'K')
            card = 13;
        else if (d == 'A')
            card = 14;
        else {
            fprintf(stderr, "%c: Invalid value\n", d);
            return 0;
        }
        if (i < 2) {
            val[0][i] = card;
            col[0][i] = c;
        } else if (i >= 2 && i < 4) {
            val[1][i - 2] = card;
            col[1][i - 2] = c;
        } else {
            for (int j = 0; j < 2; ++j) {
                val[j][i - 2] = card;
                col[j][i - 2] = c;
            }
        }
    }
    if (duplicates(val[0], col[0]) == 1 || duplicates(val[1], col[1])) {
        fprintf(stderr, "duplicates\n");
        return 0;
    }
    return 1;
}

char match_result(int val1[], char col1[], int val2[], char col2[])
{
    int p1 = straightflush(val1, col1);
    int p2 = straightflush(val2, col2);

    if (p1 != 0 || p2 != 0) {
        if (p1 == p2)
            return 'D';
        if (p1 > p2)
            return 'W';
        return 'L';
    }

    p1 = four_of_kind(val1);
    p2 = four_of_kind(val2);

    if (p1 != 0 || p2 != 0) {
        if (p1 == p2) {
            return highCard(val1, val2, 1, p1, 0);
        }
        if (p1 > p2)
            return 'W';
        return 'L';
    }

    int tok1 = three_of_kind(val1);
    int tok2 = three_of_kind(val2);
    int pair1 = pair(val1, tok1);
    int pair2 = pair(val2, tok2);

    if ((tok1 != 0 && pair1 != 0) || (tok2 != 0 && pair2 != 0)) {
        if (tok1 == tok2) {
            if (pair1 == pair2)
                return 'D';
            if (pair1 > pair2)
                return 'W';
            return 'L';

        }
        if (tok1 > tok2)
            return 'W';
        return 'L';
    }

    char flush1 = flush(col1);
    char flush2 = flush(col2);

    if (flush1 != 'n' && flush2 != 'n') {
        return highCard_color(val1, val2, col1, col2, flush1);
    }
    if (flush1 != 'n')
        return 'W';
    if (flush2 != 'n')
        return 'L';

    p1 = straight(val1);
    p2 = straight(val2);

    if (p1 != 0 || p2 != 0) {
        if (p1 == p2)
            return 'D';
        if (p1 > p2)
            return 'W';
        return 'L';
    }

    if (tok1 != 0 && tok2 != 0) {
        if (tok1 == tok2) {
            return highCard(val1, val2, 2, tok1, 0);
        }
        if (tok1 > tok2)
            return 'W';
        return 'L';
    }
    if (tok1 != 0)
        return 'W';
    if (tok2 != 0)
        return 'L';


    if (pair1 != 0 && pair2 != 0) {
        int spair1 = pair(val1, pair1);
        int spair2 = pair(val2, pair2);
        if (spair1 == 0 && spair2 == 0) {
            if (pair1 == pair2)
                return highCard(val1, val2, 3, pair1, spair1);
            if (pair1 > pair2)
                return 'W';
            return 'L';
        }
        if (spair1 != 0 && spair2 != 0) {

            if (pair1 == pair2 && spair1 == spair2)
                return highCard(val1, val2, 1, pair1, spair1);
            if (pair1 == pair2 && spair1 > spair2)
                return 'W';
            if (pair1 == pair2 && spair1 < spair2)
                return 'L';
            if (pair1 > pair2)
                return 'W';
            return 'L';
        }
        if (spair1 != 0)
            return 'W';
        return 'L';
    }
    if (pair1 != 0)
        return 'W';
    if (pair2 != 0)
        return 'L';

    return highCard(val1, val2, 5, 0, 0);


}


int duplicates(int val[CARDS], char col[CARDS])
{
    for (int i = 0; i < CARDS; ++i) {
        for (int j = i + 1; j < CARDS; ++j) {
            if (val[i] == val[j] && col[i] == col[j])
                return 1;
        }
    }
    return 0;
}

void sort(int val[], char col[])
{
    for (int i = 0; i < CARDS - 1; ++i) {
        for (int j = 0; j < CARDS - 1 - i; ++j) {
            if (val[j] > val[j + 1]) {
                int tempv = val[j];
                char tempc = col[j];
                val[j] = val[j + 1];
                val[j + 1] = tempv;
                col[j] = col[j + 1];
                col[j + 1] = tempc;
            }
        }
    }
}

char highCard(const int val1[], const int val2[], int n, int ignore, int ignore2)
{
    int i1 = CARDS - 1;
    int i2 = CARDS - 1;
    int counter = 0;
    while (counter < n && i1 >= 0 && i2 >= 0) {
        if (val1[i1] == ignore || val2[i2] == ignore || val1[i1] == ignore2 || val2[i2] == ignore2) {
            if (val1[i1] == ignore || val1[i1] == ignore2)
                i1--;
            if (val2[i2] == ignore || val2[i2] == ignore2)
                i2--;
            continue;
        }
        if (val1[i1] > val2[i2])
            return 'W';
        if (val1[i1] < val2[i2])
            return 'L';
        counter++;
        i1--;
        i2--;
    }
    return 'D';
}

char highCard_color(const int val1[], const int val2[], char col1[], char col2[], char c)
{
    int i1 = CARDS - 1;
    int i2 = CARDS - 1;
    int counter = 0;
    while (counter < 5 && i1 >= 0 && i2 >= 0) {
        if (col1[i1] == c && col2[i2] == c) {
            if (val1[i1] > val2[i2])
                return 'W';
            if (val1[i1] < val2[i2])
                return 'L';
            counter++;
            i1--;
            i2--;
        }
        if (i1 >= 0 && col1[i1] != c)
            i1--;
        if (i2 >= 0 && col2[i2] != c)
            i2--;
    }
    return 'D';
}

int four_of_kind(int val[])
{
    for (int i = CARDS - 1; i > 2; --i) {
        if (val[i] == val[i - 1] && val[i] == val[i - 2] && val[i] == val[i - 3])
            return val[i];
    }
    return 0;
}

int three_of_kind(int val[])
{
    for (int i = CARDS - 1; i > 1; --i) {
        if (val[i] == val[i - 1] && val[i] == val[i - 2])
            return val[i];
    }
    return 0;
}

int pair(int val[], int ignore)
{
    for (int i = CARDS - 1; i > 0; --i) {
        if (val[i] != ignore && val[i] == val[i - 1])
            return val[i];
    }
    return 0;
}

int straightflush(int val[], char col[])
{
    char color = most_color(col);
    int counter = 0;
    if (val[6] == 14 && val[0] == 2 && col[6] == col[0])
        counter++;
    for (int i = 0; i < CARDS - 1; ++i) {
        if (val[i] == val[i + 1] && col[i] != color)
            continue;
        if (val[i] == val[i + 1] - 1 && col[i] == color && col[i + 1] == color)
            counter++;
        else {
            if (counter >= 4)
                return val[i];
            counter = 0;
        }
    }
    if (counter >= 4)
        return val[CARDS];
    return 0;
}

int straight(int val[])
{
    int counter = 0;
    if (val[6] == 14 && val[0] == 2)
        counter++;

    for (int i = 0; i < CARDS - 1; ++i) {
        if (val[i] == val[i + 1])
            continue;
        if (val[i] == val[i + 1] - 1)
            counter++;
        else {
            if (counter >= 4)
                return val[i];
            counter = 0;
        }
    }
    if (counter >= 4)
        return val[CARDS - 1];
    return 0;
}

char most_color(const char colors[])
{
    int counter[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < CARDS; ++i) {
        if (colors[i] == 'h')
            counter[0]++;
        if (colors[i] == 'd')
            counter[1]++;
        if (colors[i] == 's')
            counter[2]++;
        if (colors[i] == 'c')
            counter[3]++;
    }
    for (int j = 0; j < 4; ++j) {
        if (counter[j] >= 5)
            switch (j) {
            case 0:
                return 'h';
            case 1:
                return 'd';
            case 2:
                return 's';
            case 3:
                return 'c';
            }
    }
    return 'n';
}

char flush(char col[])
{
    char color = most_color(col);
    if (color != 'n') {
        int counter = 0;
        for (int i = 0; i < CARDS; ++i) {
            if (col[i] == color)
                counter++;
            if (counter >= 5)
                return color;
        }
    }
    return 'n';
}

