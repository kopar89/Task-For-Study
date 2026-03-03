#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <errno.h>

void v_mashinnoe(void *num, int *b, size_t size) {
    unsigned char *bytes = (unsigned char *)num;
    int bit_index = 0;
    
    for (size_t byte_idx = 0; byte_idx < size; byte_idx++) {
        for (int bit_idx = 7; bit_idx >= 0; bit_idx--) {
            b[bit_index++] = (bytes[byte_idx] >> bit_idx) & 1;
        }
    }
}

void iz_mashinnogo(int *b, void *out, size_t size) {
    unsigned char *bytes = (unsigned char *)out;
    int total_bits = size * 8;
    memset(bytes, 0, size);
    
    for (int i = 0; i < total_bits; i++) {
        int byte_idx = i / 8;
        int bit_idx = 7 - (i % 8);
        if (b[i] == 1)
            bytes[byte_idx] |= (1 << bit_idx);
    }
}

void numberToMachinecode(void *num_ptr, size_t size, char *out_str) {
    int total_bits = size * 8;
    int *bits = (int *)malloc(total_bits * sizeof(int));
    
    if (!bits) {
        fprintf(stderr, "Ошибка: не удалось выделить память\n");
        strcpy(out_str, "ERROR");
        return;
    }
    
    v_mashinnoe(num_ptr, bits, size);
    
    char *ptr = out_str;
    for (int i = 0; i < total_bits; i++) {
        *ptr++ = bits[i] ? '1' : '0';
        if ((i + 1) % 4 == 0 && (i + 1) < total_bits)
            *ptr++ = ' ';
    }
    *ptr = '\0';
    
    free(bits);
}

void save_number_hex(FILE *fp, long double num) {
    if (!fp) {
        fprintf(stderr, "Ошибка: файл не открыт\n");
        return;
    }
    
    unsigned char *bytes = (unsigned char *)&num;
    for (size_t i = 0; i < sizeof(long double); i++) {
        fprintf(fp, "%02X", bytes[i]);
    }
    fprintf(fp, "\n");
}

long double load_number_hex(FILE *fp) {
    if (!fp) {
        fprintf(stderr, "Ошибка: файл не открыт\n");
        return 0.0L;
    }
    
    char hex[64];
    if (fscanf(fp, "%63s", hex) != 1) {
        fprintf(stderr, "Ошибка чтения hex из файла\n");
        return 0.0L;
    }
    
    long double num = 0.0L;  
    unsigned char *bytes = (unsigned char *)&num;
    memset(bytes, 0, sizeof(long double));
    
    size_t len = strlen(hex);
    for (size_t i = 0; i < len / 2 && i < sizeof(long double); i++) {
        unsigned int byte_val;
        if (sscanf(hex + i * 2, "%02X", &byte_val) != 1) {
            fprintf(stderr, "Ошибка парсинга hex\n");
            return 0.0L;
        }
        bytes[i] = (unsigned char)byte_val;
    }
    return num;
}

long double round_to_p(long double x, int p) {
    long double factor = powl(10.0L, p);
    return floorl(x * factor + 0.5L) / factor;
}

void input_file_for_study() {
    char *filename = "info.txt";
    int n, k, a, b, p, razryd;
    
    printf("Введите количество вариантов: ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Ошибка ввода количества вариантов\n");
        return;
    }
    
    printf("Введите количество заданий для одного студента: ");
    if (scanf("%d", &k) != 1) {
        fprintf(stderr, "Ошибка ввода количества заданий\n");
        return;
    }
    
    printf("Введите разрядность вещественного числа (32, 64, 128): ");
    if (scanf("%d", &razryd) != 1) {
        fprintf(stderr, "Ошибка ввода разрядности\n");
        return;
    }
    
    if (razryd != 32 && razryd != 64 && razryd != 128) {
        fprintf(stderr, "Ошибка: разрядность должна быть 32, 64 или 128\n");
        return;
    }
    
    printf("Введите диапазон (a b): ");
    if (scanf("%d %d", &a, &b) != 2) {
        fprintf(stderr, "Ошибка ввода диапазона\n");
        return;
    }
    
    if (a >= b) {
        fprintf(stderr, "Ошибка: a должно быть меньше b\n");
        return;
    }
    
    printf("Введите количество знаков после запятой (для вывода в Markdown): ");
    if (scanf("%d", &p) != 1) {
        fprintf(stderr, "Ошибка ввода количества знаков\n");
        return;
    }
    
    if (p < 0 || p > 15) {
        fprintf(stderr, "Ошибка: количество знаков должно быть от 0 до 15\n");
        return;
    }

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("Не удалось открыть файл info.txt");
        return;
    }
    
    fprintf(fp, "%d %d %d %d %d %d\n", n, k, razryd, a, b, p);
    fclose(fp);
    printf("✓ Конфигурация сохранена в info.txt\n");
}

void genereta_task() {
    FILE *fp = fopen("info.txt", "r");
    if (!fp) {
        perror("Не удалось открыть info.txt");
        return;
    }
    
    int n, k, razryd, a, b, p;
    if (fscanf(fp, "%d %d %d %d %d %d", &n, &k, &razryd, &a, &b, &p) != 6) {
        fprintf(stderr, "Ошибка чтения конфигурации\n");
        fclose(fp);
        return;
    }
    fclose(fp);

    if (mkdir("Task", 0777) != 0 && errno != EEXIST) {
        perror("Не удалось создать папку Task");
        return;
    }

    FILE *dp = fopen("data.txt", "w");
    if (!dp) {
        perror("Не удалось открыть data.txt");
        return;
    }

    printf("Генерация заданий...\n");
    
    for (int v = 1; v <= n; v++) {
        char path[256];
        snprintf(path, sizeof(path), "Task/variant_%d.md", v);  
        
        FILE *wp = fopen(path, "w");
        if (!wp) {
            fprintf(stderr, "Не удалось создать файл %s\n", path);
            continue;
        }
        
        fprintf(wp, "# Вариант %d\n\n| № | Число | Формат |\n|--|--|--|\n", v);

        for (int t = 1; t <= k; t++) {
            long double fraction = (long double)rand() / (RAND_MAX + 1.0L);
            long double num = a + fraction * (b - a);

            save_number_hex(dp, num);

            if (razryd == 32) {
                float f = (float)num;
                fprintf(wp, "| %d | %.*f | %d бит |\n", t, p, f, razryd);
            } else if (razryd == 64) {
                double d = (double)num;
                fprintf(wp, "| %d | %.*f | %d бит |\n", t, p, d, razryd);
            } else {  
                fprintf(wp, "| %d | %.*Lf | %d бит |\n", t, p, num, razryd);
            }
        }
        fclose(wp);
        printf("  ✓ Создан вариант %d\n", v);
    }
    fclose(dp);
    printf("✓ Задания успешно созданы в папке Task\n");
}

void solve_task() {
    FILE *info = fopen("info.txt", "r");
    FILE *dp = fopen("data.txt", "r");
    
    if (!info || !dp) {
        perror("Не удалось открыть файлы конфигурации");
        if (info) fclose(info);
        if (dp) fclose(dp);
        return;
    }

    int n, k, razryd, a, b, p;
    if (fscanf(info, "%d %d %d %d %d %d", &n, &k, &razryd, &a, &b, &p) != 6) {
        fprintf(stderr, "Ошибка чтения конфигурации\n");
        fclose(info);
        fclose(dp);
        return;
    }
    fclose(info);

    if (mkdir("Solve", 0777) != 0 && errno != EEXIST) {
        perror("Не удалось создать папку Solve");
        fclose(dp);
        return;
    }

    printf("Генерация решений...\n");
    
    for (int v = 1; v <= n; v++) {
        char path[256];
        snprintf(path, sizeof(path), "Solve/variant_%d.md", v);  
        
        FILE *wp = fopen(path, "w");
        if (!wp) {
            fprintf(stderr, "Не удалось создать файл %s\n", path);
            continue;
        }
        
        fprintf(wp, "# Решение Варианта %d\n\n", v);
        fprintf(wp, "| № | Исходное | Машинный код | Ошибка |\n|:-:|:-:|:-:|:-:|\n");

        for (int t = 1; t <= k; t++) {
            long double original = load_number_hex(dp);

            int bits_size = razryd + razryd / 4;  
            char *bits = (char *)malloc(bits_size);
            if (!bits) {
                fprintf(stderr, "Ошибка выделения памяти\n");
                continue;
            }
            
            long double error = 0.0L;

            if (razryd == 32) {
                float num = (float)original;
                long double rnd = round_to_p((long double)num, p);
                error = fabsl((long double)num - rnd);

                numberToMachinecode(&num, sizeof(float), bits);
                fprintf(wp, "| %d | %.*f | %s | %.3Le |\n", t, p, num, bits, error);
            }
            else if (razryd == 64) {
                double num = (double)original;
                long double rnd = round_to_p((long double)num, p);
                error = fabsl((long double)num - rnd);

                numberToMachinecode(&num, sizeof(double), bits);
                fprintf(wp, "| %d | %.*f | %s | %.3Le |\n", t, p, num, bits, error);
            }
            else if (razryd == 128) {
                long double num = original;
                long double rnd = round_to_p(num, p);
                error = fabsl(num - rnd);

                numberToMachinecode(&num, sizeof(long double), bits);
                fprintf(wp, "| %d | %.*Lf | %s | %.3Le |\n", t, p, num, bits, error);
            }
            
            free(bits);
        }
        fclose(wp);
        printf("  ✓ Создано решение для варианта %d\n", v);
    }
    fclose(dp);
    printf("✓ Решения успешно созданы в папке Solve\n");
}

int main() {
    srand((unsigned int)time(NULL));
    
    printf("=== Система генерации заданий ===\n\n");
    
    input_file_for_study();
    genereta_task();
    solve_task();
    
    printf("\n✓ Готово! Проверьте папки Task и Solve.\n");
    return 0;
}
