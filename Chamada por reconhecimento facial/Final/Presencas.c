#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <curl/curl.h>

#define MAX_ALUNOS 1000
#define MAX_NOME 256
#define MAX_DATES 512
#define MAX_LINE 4096

static void trim(char *s) {
    if (!s) return;
    char *p = s;
    while (*p && (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    int len = (int)strlen(s);
    while (len > 0 && (s[len-1] == ' ' || s[len-1] == '\t' || s[len-1] == '\r' || s[len-1] == '\n')) {
        s[len-1] = '\0'; len--;
    }
}

static void remove_bom(char *s) {
    unsigned char *u = (unsigned char*)s;
    if (u[0] == 0xEF && u[1] == 0xBB && u[2] == 0xBF) {
        memmove(s, s+3, strlen(s+3)+1);
    }
}

static int split_csv_simple(char *line, char tokens[][MAX_NOME], int max_tokens) {
    int idx = 0;
    char *p = line;
    while (idx < max_tokens && p && *p) {
        char *sep = strchr(p, ',');
        if (sep) {
            int len = (int)(sep - p);
            if (len >= MAX_NOME) len = MAX_NOME-1;
            strncpy(tokens[idx], p, len);
            tokens[idx][len] = '\0';
            trim(tokens[idx]);
            idx++;
            p = sep + 1;
        } else {
            strncpy(tokens[idx], p, MAX_NOME-1);
            tokens[idx][MAX_NOME-1] = '\0';
            trim(tokens[idx]);
            idx++;
            break;
        }
    }
    return idx;
}

static void obterDataHoje(char *out) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(out, "%02d/%02d", tm.tm_mday, tm.tm_mon + 1);

 
}

static char alunos[MAX_ALUNOS][MAX_NOME];
static int nAlunos = 0;

static char presentes[MAX_ALUNOS][MAX_NOME];
static int nPresentes = 0;

void carregarAlunos(const char *arquivo) {
    FILE *f = fopen(arquivo, "r");
    if (!f) {
        fprintf(stderr, "Erro: nao foi possivel abrir %s\n", arquivo);
        exit(1);
    }
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    remove_bom(line); trim(line);
    char low[MAX_LINE]; strncpy(low, line, sizeof(low)-1); low[sizeof(low)-1]=0;
    for (int i=0; low[i]; i++) low[i] = (char)tolower((unsigned char)low[i]);
    int header = (strstr(low, "id") || strstr(low, "nome") || strstr(low, "name")) ? 1 : 0;
    if (!header) {
        char toks[8][MAX_NOME];
        int tc = split_csv_simple(line, toks, 8);
        if (tc >= 2) {
            strncpy(alunos[nAlunos], toks[1], MAX_NOME-1);
            alunos[nAlunos][MAX_NOME-1] = '\0';
            nAlunos++;
        } else if (tc == 1) {
            strncpy(alunos[nAlunos], toks[0], MAX_NOME-1);
            alunos[nAlunos][MAX_NOME-1] = '\0';
            nAlunos++;
        }
    }
    while (fgets(line, sizeof(line), f)) {
        remove_bom(line); trim(line);
        if (strlen(line) == 0) continue;
        char toks[8][MAX_NOME];
        int tc = split_csv_simple(line, toks, 8);
        if (tc >= 2) {
            strncpy(alunos[nAlunos], toks[1], MAX_NOME-1);
            alunos[nAlunos][MAX_NOME-1] = '\0';
            nAlunos++;
        } else if (tc == 1) {
            strncpy(alunos[nAlunos], toks[0], MAX_NOME-1);
            alunos[nAlunos][MAX_NOME-1] = '\0';
            nAlunos++;
        }
        if (nAlunos >= MAX_ALUNOS) break;
    }
    fclose(f);
}

void carregarPresentes(const char *arquivo) {
    FILE *f = fopen(arquivo, "r");
    if (!f) {
        nPresentes = 0;
        return;
    }
    char line[MAX_LINE];
    if (!fgets(line, sizeof(line), f)) { fclose(f); nPresentes = 0; return; }
    remove_bom(line); trim(line);
    char low[MAX_LINE]; strncpy(low, line, sizeof(low)-1); low[sizeof(low)-1]=0;
    for (int i=0; low[i]; i++) low[i] = (char)tolower((unsigned char)low[i]);
    int header = (strstr(low, "nome") || strstr(low, "name")) ? 1 : 0;
    if (!header) {
        strncpy(presentes[nPresentes], line, MAX_NOME-1);
        presentes[nPresentes][MAX_NOME-1] = '\0';
        trim(presentes[nPresentes]);
        nPresentes++;
    }
    while (fgets(line, sizeof(line), f)) {
        remove_bom(line); trim(line);
        if (strlen(line) == 0) continue;
        char toks[8][MAX_NOME];
        int tc = split_csv_simple(line, toks, 8);
        if (tc >= 1) {
            strncpy(presentes[nPresentes], toks[0], MAX_NOME-1);
            presentes[nPresentes][MAX_NOME-1] = '\0';
            trim(presentes[nPresentes]);
            nPresentes++;
        }
        if (nPresentes >= MAX_ALUNOS) break;
    }
    fclose(f);
}

void atualizarPresencasSemestral(const char *arquivoSemestral) {
    FILE *f = fopen(arquivoSemestral, "r");
    char header[MAX_LINE] = "";
    char dates[MAX_DATES][32]; int nDates = 0;
    char map_names[MAX_ALUNOS][MAX_NOME]; int map_n = 0;
    int *map_n_dates = NULL;
    int **map_vals = NULL; 

    if (f) {
        if (fgets(header, sizeof(header), f)) {
            remove_bom(header); trim(header);
            char toks[ MAX_DATES ][ MAX_NOME ];
            int tc = split_csv_simple(header, toks, MAX_DATES);
            nDates = (tc >= 1) ? (tc - 1) : 0;
            for (int i=0; i<nDates; i++) {
                strncpy(dates[i], toks[i+1], 31); dates[i][31] = '\0'; trim(dates[i]);
            }
        }
        char line[MAX_LINE];
        map_n = 0;
        map_n_dates = malloc(sizeof(int) * MAX_ALUNOS);
        map_vals = malloc(sizeof(int*) * MAX_ALUNOS);
        while (fgets(line, sizeof(line), f)) {
            char copy[MAX_LINE]; strncpy(copy, line, MAX_LINE-1); copy[MAX_LINE-1] = '\0';
            char toks[ MAX_DATES ][ MAX_NOME ];
            int tc = split_csv_simple(copy, toks, MAX_DATES);
            if (tc >= 1) {
                strncpy(map_names[map_n], toks[0], MAX_NOME-1); map_names[map_n][MAX_NOME-1]=0; trim(map_names[map_n]);
                int existing_dates = tc - 1;
                map_n_dates[map_n] = existing_dates;
                map_vals[map_n] = malloc(sizeof(int) * existing_dates);
                for (int d=0; d<existing_dates; d++) {
                    map_vals[map_n][d] = atoi(toks[1 + d]);
                }
                map_n++;
                if (map_n >= MAX_ALUNOS) break;
            }
        }
        fclose(f);
    }
    char dataHoje[16]; obterDataHoje(dataHoje);
    int hoje_index = -1;
    for (int i=0;i<nDates;i++) if (strcmp(dates[i], dataHoje) == 0) { hoje_index = i; break; }
    if (hoje_index == -1) {
        if (nDates < MAX_DATES-1) {
            strncpy(dates[nDates], dataHoje, 31); dates[nDates][31] = '\0';
            hoje_index = nDates;
            nDates++;
        } else {
            fprintf(stderr, "Numero maximo de datas atingido\n");
            if (map_vals) {
                for (int i=0;i<map_n;i++) free(map_vals[i]);
                free(map_vals); free(map_n_dates);
            }
            return;
        }
    }
    FILE *tmp = fopen("temp_presencas.csv", "w");
    if (!tmp) {
        fprintf(stderr, "Erro ao criar arquivo temporario\n");
        if (map_vals) { for (int i=0;i<map_n;i++) free(map_vals[i]); free(map_vals); free(map_n_dates); }
        return;
    }
    fprintf(tmp, "Nome");
    for (int i=0;i<nDates;i++) fprintf(tmp, ",%s", dates[i]);
    fprintf(tmp, "\n");

    for (int i=0;i<nAlunos;i++) {
        char *nome = alunos[i];
        fprintf(tmp, "%s", nome);
        int found = -1;
        for (int m=0; m<map_n; m++) if (strcmp(map_names[m], nome) == 0) { found = m; break; }
        for (int d=0; d<nDates; d++) {
            int val = 0;
            if (found != -1 && d < map_n_dates[found]) {
                val = map_vals[found][d];
            } else {
                val = 0;
            }
            if (d == hoje_index) {
                int pres = 0;
                for (int p=0;p<nPresentes;p++) if (strcmp(presentes[p], nome) == 0) { pres = 1; break; }
                val = pres;
            }
            fprintf(tmp, ",%d", val);
        }
        fprintf(tmp, "\n");
    }

    fclose(tmp);
    if (map_vals) {
        for (int i=0;i<map_n;i++) free(map_vals[i]);
        free(map_vals); free(map_n_dates);
    }

    remove(arquivoSemestral);
    if (rename("temp_presencas.csv", arquivoSemestral) != 0) {
        fprintf(stderr, "Erro ao mover arquivo temporario para %s\n", arquivoSemestral);
    } else {
        printf("Arquivo semestral atualizado: %s\n", arquivoSemestral);
    }
}

int main(void) {
    carregarAlunos("nomes.csv");//cadastro
    carregarPresentes("presentes.csv");//chamada

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int ano = tm.tm_year + 1900;
    int semestre = (tm.tm_mon < 6) ? 1 : 2;
    char arquivoSemestral[256];
    snprintf(arquivoSemestral, sizeof(arquivoSemestral), "presencas_%d_S%d.csv", ano, semestre);
    
    atualizarPresencasSemestral(arquivoSemestral);



    const char *token = "";
    char caminhoLocal[300];
    snprintf(caminhoLocal, sizeof(caminhoLocal),
             "%s", arquivoSemestral);
    char caminhoDropbox[300];
    snprintf(caminhoDropbox, sizeof(caminhoDropbox),
             "/%s", arquivoSemestral);
    char comando[3000];
    snprintf(comando, sizeof(comando),
        "curl -X POST https://content.dropboxapi.com/2/files/upload "
        "--header \"Authorization: Bearer %s\" "
        "--header \"Dropbox-API-Arg: {\\\"path\\\": \\\"%s\\\", \\\"mode\\\": \\\"overwrite\\\"}\" "
        "--header \"Content-Type: application/octet-stream\" "
        "--data-binary @\"%s\"",
        token, caminhoDropbox, caminhoLocal);

    printf("Executando:\n%s\n\n", comando);
    int r = system(comando);
    if (r == 0)
        printf("Upload concluído! Arquivo enviado: %s\n", arquivoSemestral);
    else
        printf("Erro ao executar o comando curl.\n");

    return 0;
}
