#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <string.h>  
#include <time.h>

void float32ToMachinecode_file(float num, FILE *fp) {
    uint32_t bits;
    memcpy(&bits, &num, sizeof(bits));  // Безопасное копирование
    
    for (int i = 31; i >= 0; i--) {
        fprintf(fp, "%d", (bits >> i) & 1);
        if (i % 4 == 0) 
            fprintf(fp, " "); 
    }
}

void double64ToMachinecode_file(double num, FILE *fp) {
    uint64_t bits;
    memcpy(&bits, &num, sizeof(bits));  // Безопасное копирование
    
    for (int i = 63; i >= 0; i--) {
        fprintf(fp, "%d", (bits >> i) & 1);
        if (i % 4 == 0) 
            fprintf(fp, " ");
    }
}

float MachinecodeToFloat(uint32_t bits) {
    return *(float*)&bits;
}

double MachinecodeToDouble(uint64_t bits) {
    return *(double*)&bits;
}


void input_file_for_study(){

    char *filename = "info.txt";
    int n, k, a, b, p, razryd;
    printf("Введите количество вариантов: ");
    scanf("%d", &n);
    while (n <= 0){
        printf("Количество вариантов должно быть положительным! Введите снова: ");
        scanf("%d", &n);
    }

    printf("Введите количество заданий для одного студента: ");
    scanf("%d", &k);
    while (k <= 0){
        printf("Количество заданий должно быть положительным! Введите снова: ");
        scanf("%d", &k);
    }

    printf("Введите разрядность вещественного числа (32, 64): ");
    scanf("%d", &razryd);
    while (razryd != 32 && razryd != 64){
        printf("Ошибка! Введите 32, 64: ");
        scanf("%d", &razryd);
    }

    printf("Введите диапазон представления вещественного числа (a b): ");
    scanf("%d", &a);
    scanf("%d", &b);
    while (a > b || a < 0 || b < 0){
        printf("Ошибка! a должно быть <= b. Введите снова: ");
        scanf("%d", &a);
        scanf("%d", &b);
    }

    printf("Введите количество знаков после запятой: ");
    scanf("%d", &p);
    while (p < 0){
        printf("Количество знаков не может быть отрицательным! Введите снова: ");
        scanf("%d", &p);
    }

    FILE *fp = fopen(filename, "w");
    if (!fp){
        printf("Ошибка открытия файла!\n");
        return;
    }

    fprintf(fp, "Количество вариантов: %d\n", n);
    fprintf(fp, "Количество заданий: %d\n", k);
    fprintf(fp, "Разрядность: %d\n", razryd);
    fprintf(fp, "Диапазон: [%d : %d]\n", a, b);
    fprintf(fp, "Количество знаков после запятой: %d\n", p);

    fclose(fp);

    printf("\nДанные успешно записаны в файл %s\n", filename);
}

void genereta_task(){

    char *filename = "info.txt";
    char *datafile = "data.txt";
    int arr[6], x, i = 0;

    FILE *dp = fopen(datafile, "w");
    FILE *fp = fopen(filename, "r");
    if(!fp){
        printf("Ошибка чтения файла!\n");
        return;
    }

    if(!dp){
        printf("Ошибка открытия файла");
        return;
    }

    while (fscanf(fp, "%*[^0-9]%d", &x) == 1 && i < 6) {
        arr[i] = x;
        i++;
    }
    fclose(fp);

    const char *dir = "Task";
    if (mkdir(dir, 0777) != 0 && errno != EEXIST) {
        perror("Ошибка создания каталога");
        return;
    }

    char variant_filename[256];
    for (int v = 1; v <= arr[0]; v++) {
        sprintf(variant_filename, "%s/variant_%d.md", dir, v);

        FILE *wp = fopen(variant_filename, "w");
        if (!wp) {
            printf("Ошибка создания файла %s!\n", variant_filename);
            return;
        }

        fprintf(wp, "# Вариант %d\n\n", v);
        fprintf(wp, "№ \tВещественное число\tФормат\n");

        for (int t = 1; t <= arr[1]; t++) {
            double num = arr[3] + (double)rand() / RAND_MAX * (arr[4] - arr[3]);
            fprintf(dp, "%lf ", num);
            fprintf(wp, "%d.\t%.*f\t\t\t\t %d бит.\n", t, arr[5], num, arr[2]);
        }

        fclose(wp);

        printf("Файл %s успешно создан!\n", variant_filename);
    }
        fclose(dp);

}

void solve_task() {
    FILE *fp = fopen("data.txt", "r");
    if (!fp) {
        printf("Ошибка открытия файла data.txt\n");
        return;
    }

    int arr[6], x, i = 0;
    FILE *info = fopen("info.txt", "r");
    if (!info) {
        printf("Ошибка открытия info.txt\n");
        fclose(fp);
        return;
    }

    while (fscanf(info, "%*[^0-9]%d", &x) == 1 && i < 6) {
        arr[i] = x;
        i++;
    }
    fclose(info);


    const char *dir = "Solve";
    if (mkdir(dir, 0777) != 0 && errno != EEXIST) {
        perror("Ошибка создания каталога Solve");
        fclose(fp);
        return;
    }

    const int size = arr[0];
    for (int v = 1; v <= size; v++) {
        char filename[256];
        sprintf(filename, "%s/variant_%d.md", dir, v);

        FILE *wp = fopen(filename, "w");
        if (!wp) {
            printf("Ошибка создания файла %s\n", filename);
            continue;
        }

        fprintf(wp, "# Решение Варианта %d\n\n", v);
        fprintf(wp, "№ \tВещественное число\tМашинное представление\t\t\tОшибка\n");

        for (int t = 1; t <= arr[1]; t++) {
            double num;
            if (fscanf(fp, "%lf", &num) != 1) {
                printf("Недостаточно чисел в data.txt\n");
                break;
            }

            double rounded = num;  
            double error = (arr[2] == 32) ? fabs(num - (double)(float)num) : 0.0;
            fprintf(wp, "%d.\t%.*f\t", t, arr[5], num);

        if (arr[2] == 32) {
        float num32 = (float)num; 
        float32ToMachinecode_file(num32, wp);
        } else if (arr[2] == 64) {
        double64ToMachinecode_file(num, wp);
        }
            fprintf(wp, "\t%.3e\n", error); 
        }

        fclose(wp);
        printf("Файл решения %s создан!\n", filename);
    }

    fclose(fp);
}

int main(){
    srand(time(NULL));
    printf("\nЭто программа для помощи преподователям в создании учебно-практического материала.\nДля начала работы нужно указать некотурую информацию\n\n");
    input_file_for_study();
    genereta_task();
    printf("\n");
    solve_task();
}
