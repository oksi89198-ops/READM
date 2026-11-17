// Фермер + Приключения в Башне 
// напрямую через терминал (cntrl+~ ) + .\bashnatel.exe

#include <stdio.h>
#pragma warning(disable:4996) // Отключаем ругань VS на scanf/fopen
#include <windows.h>  // Для настройки консоли Windows
#include <direct.h>  // Для функции _getcwd + узнать текущую папку
#include <locale.h>   
// Для русского языка
#include <stdlib.h>   // Для malloc, free, rand, srand
#include <string.h>   // Для работы со строками (strcmp, strcpy и т.д.)
#include <ctype.h>    // Для isdigit, tolower
#include <time.h>     // Для time(NULL)
#include <stdbool.h>  // Для типа bool (true/false)
#include <stdio.h>
#include <conio.h>
#pragma warning(disable:4996) // Отключаем ругань VS на scanf/fopen

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

// --- Константы для ЛР 2-3 (Фермер) ---
#define NAME_LEN 32               // Длина имени
#define INVENTORY_SIZE 10         // Количество слотов
#define hours_in_day 24           // Часов в сутках
#define MAX_ITEMS 10              // Количество видов предметов
#define MMAX_ITEM_NAME_LEN 50     // Длина названия предмета
#define MAX_LINE_LEN 256          // Длина строки при чтении файла
#define MAX_INVENTORY 10

#define MENU_MIN 0                // Мин. пункт меню
#define MENU_MAX 10               // Макс. пункт меню

#define EMPTY_ID 0                // ID пустого слота
#define wOOD_ID 1
#define STONE_ID 2
#define SEED_ID 3
#define PICKAXE_ID 4
#define TORCH_ID 5
#define FOOD_ID 6
#define ROPE_ID 7
#define MAP_ID 8
#define KEY_ID 9 

// Константы для телепортт башни
#define MIN_SIZE 5                // Мин размер карты
#define MAX_SIZE 30               // Макс размер карты
#define CHANCE_OBSTACLE 25        // Шанс появления препятствия (%)

#define WALL '#'                  // Стена
#define MAP_EMPTY '.'             // Пустая клетка (не путать с EMPTY_ID)
#define TREE 'T'                  // Дерево
#define ROCK 'R'                  // Камень
#define HERO '@'                  // Герой
#define LADDER_UP 'U'             // Лестница вверх
#define LADDER_DOWN 'D'           // Лестница вниз
#define RUNE '^'                  // Руна

// Имена файлов 
const char* ITEMS_FILE = "items.txt";
const char* DIARY_FILE = "diary.txt";
const char* INPUT_FILE = "input.txt";
const char* OUTPUT_FILE = "output.txt";

// ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ 
char farmer_name[NAME_LEN];
char item_names[MAX_ITEMS][MMAX_ITEM_NAME_LEN];//таблица названий предметов
int inventory[INVENTORY_SIZE] = {0, wOOD_ID, SEED_ID, STONE_ID, PICKAXE_ID, 0, 0, 0, 0, 0};
int current_day = 1;
int current_hour = 8;

// Флаг поддержки ANSI-цветов (включается в main при успехе SetConsoleMode)
static bool g_useColor = false;

// Печать одного символа с цветом (если цвета не поддерживаются — просто символ)
static void print_colored(char ch, const char* color) {
    if (g_useColor && color) {
        printf("%s%c\033[0m", color, ch);
    } else {
        printf("%c", ch);
    }
}

// СТРУКТУРЫ Башня)
typedef struct {
    int x; // Столбец
    int y; // Строка
} Point;

typedef struct {
    char **map;      // Двумерный динамический массив (сама карта)
    int width;       // Ширина
    int height;      // Высота
    Point hero;      // Координаты героя
    int floor;       // Этаж
    bool isRunning;  // Флаг работы игры
} GameState;


// ПРОТОТИПЫ ФУНКЦИЙ
int get_int_input(const char* prompt);
void safe_input_string(const char* prompt, char* buffer, int max_len);
void load_catalog(char item_names[MAX_ITEMS][MMAX_ITEM_NAME_LEN]);
void input_farmer_name(char name[]);
void print_menu();
void PRINT_CLOCK();
void handle_work();
void show_inventory_only();
void modify_inventory();
void drop_item();        
void quick_access_task();
void search_item();
void write_diary();
int highlight_word(char *line, const char *search_word);
void process_encyclopedia();

void clearScreen();
void generateMap(GameState *game);
void freeMap(GameState *game);
void drawMap(const GameState *game);
void placeHero(GameState *game, int startX, int startY);
void placeLadderAndRunes(GameState *game);
void handleRuneTeleport(GameState *game);
void processInput(GameState *game, char input);
void start_adventure();


// Безопасный ввод числа. Если ввести букву — попросит заново.
int get_int_input(const char* prompt) {
    int value;
    char c;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            // Очищаем буфер от остатков (Enter и лишние символы)
            while ((c = getchar()) != '\n' && c != EOF); 
            return value;
        }
        if (feof(stdin)) {
            printf("\n[Конец ввода]\n");
            exit(1);
        }
        while ((c = getchar()) != '\n' && c != EOF);
        printf("ОШИБКА. ВВЕДИТЕ ЧИСЛО: ");
    }
}

// Безопасный ввод строки (не переполняет буфер)
void safe_input_string(const char* prompt, char* buffer, int max_len) {
    printf("%s", prompt);
    if (fgets(buffer, max_len, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = 0; // Убираем \n
    }
}

// Загрузка каталога из файла или стандартных названий
void load_catalog(char item_names[MAX_ITEMS][MMAX_ITEM_NAME_LEN]) {
    const char* default_names[MAX_ITEMS] = {
        "пусто", "дерево", "камень", "семена", "кирка",
        "факел", "еда", "веревка", "карта", "ключ"
    };
    for (int i = 0; i < MAX_ITEMS; i++) {
        snprintf(item_names[i], MMAX_ITEM_NAME_LEN, "%s", default_names[i]);
    }

    FILE* file = fopen(ITEMS_FILE, "r");
    if (file == NULL) {
        printf("\n[Внимание: файл %s не найден! Загружены стандартные названия]\n", ITEMS_FILE);
        return;
    }
    // ... далее цикл while (fgets(...))
    char line[MAX_LINE_LEN];
    int next_free = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char* space = strchr(line, ' ');
        if (space != NULL && isdigit((unsigned char)line[0])) {
            int id = atoi(line);                 // ? используем ID из файла
            if (id >= 0 && id < MAX_ITEMS) {
                snprintf(item_names[id], MMAX_ITEM_NAME_LEN, "%s", space + 1);
            }
        } else {
            if (next_free < MAX_ITEMS) {
                snprintf(item_names[next_free], MMAX_ITEM_NAME_LEN, "%s", line);
                next_free++;
            }
        }
    }
    fclose(file);
}

// Ввод имени фермера
void input_farmer_name(char name[]) {
    while (1) {
        printf("Введите имя фермера (до 31 символа): ");
        fflush(stdout);
        if (fgets(name, NAME_LEN, stdin) == NULL) {
            name[0] = '\0';
            return;
        }
        size_t len = strlen(name);
        if (len > 0 && name[len - 1] == '\n') {
            name[len - 1] = '\0';
        } else {
            int c;
            while ((c = getchar()) != '\n' && c != EOF) { }
        }
        len = strlen(name);
        if (len > 0 && name[len - 1] == '\r') name[len - 1] = '\0'; // Для Windows

        if (name[0] != '\0') return; // Имя не пустое — выходим
        printf("Имя не может быть пустым.\n");
    }
}

// Вывод главного меню
void print_menu() {
    printf("\n ---  МЕНЮ (Фермер: %s) ---\n", farmer_name);
    printf("0. Выход\n");
    printf("1. посмотреть на часы\n");
    printf("2. промотать время(поработать)\n");
    printf("3. посмотреть инвентарь\n");
    printf("4. положить предмет в слот\n");
    printf("5. выбросить предмет\n");
    printf("6. выполнить задание по варианту\n");
    printf("7. поиск предмета по названию\n");
    printf("8. записать состояние в дневник\n");
    printf("9. расшифровать старые записи\n");
    printf("10. Отправиться в приключение (Башня)\n"); 
}

// Показ времени
void PRINT_CLOCK() {
    printf("Текущее время: день %d %02d:00\n", current_day, current_hour);
}

// Промотка времени
void handle_work() {
    int hours = get_int_input("Сколько часов хотите поработать? ");
    if (hours < 0) {
        printf("Ошибка: кол-во часов не может быть отрицательным\n");
    } else {
        current_day += hours / hours_in_day; // Добавляем дни
        current_hour = (current_hour + hours) % hours_in_day; // Обновляем часы
        printf("Стало: день %d, %02d:00\n", current_day, current_hour);
    }
}

// Только показать инвентарь
void show_inventory_only() {
    printf("Содержимое инвентаря:\n");
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        printf("  Слот %d: [%d] - %s\n", i, inventory[i], item_names[inventory[i]]);
    }
}

// Добавить предмет в слот
void modify_inventory() {
    int slot, item_id;
    char prompt[64];
    
    show_inventory_only(); // Сначала покажем, что есть

    snprintf(prompt, sizeof(prompt), "Введите индекс слота (0-%d): ", INVENTORY_SIZE - 1);
    slot = get_int_input(prompt);
    
    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Ошибка: индекс слота выходит за границы массива\n");
        return;
    }

    item_id = get_int_input("Введите id предмета (1-9): ");
    if (item_id < 0 || item_id >= MAX_ITEMS) {
        printf("Ошибка: неверный ID предмета.\n");
        return;
    }
    
    inventory[slot] = item_id;
    printf("Предмет с id %d помещён в слот %d\n", item_id, slot);
}
// п 5. удаление слота(добавила в main + в прототипы)
// Выбросить предмет из слота (очистить слот)   
void drop_item() {
    int slot;
    char prompt[64];

    show_inventory_only();

    snprintf(prompt, sizeof(prompt), "Введите индекс слота для очистки (0-%d): ", INVENTORY_SIZE - 1);
    slot = get_int_input(prompt);

    if (slot < 0 || slot >= INVENTORY_SIZE) {
        printf("Ошибка: индекс слота выходит за границы массива\n");
        return;
    }

    if (inventory[slot] == EMPTY_ID) {
        printf("Слот %d уже пуст — выбрасывать нечего.\n", slot);
        return;
    }

    printf("Из слота %d выброшен предмет: %s\n", slot, item_names[inventory[slot]]);
    inventory[slot] = EMPTY_ID;
}
//
// Задание по варианту (обмен с нулевым слотом)
void quick_access_task() {
    int target_id, found_index, temp;

    printf("Инвентарь до выполнения задания:\n");
    show_inventory_only(); 

    target_id = get_int_input("Введите id для быстрого доступа: ");
    
    found_index = -1;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] == target_id) {
            found_index = i;
            break;
        }
    }

    if (found_index == -1) {
        printf("Предмет с id %d не найден в инвентаре\n", target_id);
    } else if (found_index == 0) {
        printf("Предмет с id %d уже находится в нулевом слоте\n", target_id);
    } else {
        temp = inventory[0];
        inventory[0] = inventory[found_index];
        inventory[found_index] = temp;
        printf("Предмет с id %d перемещен в слот 0 (обмен с ячейкой %d)\n", target_id, found_index);
    } 

    printf("Инвентарь после выполнения задания:\n");
    show_inventory_only(); 
}

// Поиск предмета по названию 7п
void search_item() {
    char name[MMAX_ITEM_NAME_LEN];
    
    // Запрашиваем название
    safe_input_string("Введите название предмета (например, дерево): ", name, sizeof(name));

    // Если пользователь случайно нажал Enter, ничего не введя
    if (strlen(name) == 0) {
        printf("Ошибка: пустой запрос.\n");
        return;
    }

    int foundId = -1;
    // Ищем предмет в каталоге (строгое совпадение по регистру!)
    for (int j = 0; j < MAX_ITEMS; j++) {
        if (strcmp(item_names[j], name) == 0) {
            foundId = j;
            break;
        }
    }

    if (foundId == -1) {
        printf("Предмет \"%s\" не найден в каталоге.\n", name);
    } else {
        printf("Предмет \"%s\" имеет ID = %d.\n", name, foundId);

        int foundInInventory = 0;
        printf("Слоты с этим предметом: ");
        for (int i = 0; i < INVENTORY_SIZE; i++) {
            if (inventory[i] == foundId) {
                printf("%d ", i);
                foundInInventory = 1;
            }
        }

        if (!foundInInventory) printf("(не найдено в инвентаре)");
        printf("\n");
    }

    // ПАУЗА: чтобы вы успели прочитать результат до возврата в меню
    printf("\nНажмите Enter, чтобы вернуться в меню...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF); 
}

// Запись дневника
void write_diary() {
    FILE* file = fopen(DIARY_FILE, "a"); // "a" - дописываем в конец
    if (file == NULL) {
        printf("Ошибка открытия файла %s!\n", DIARY_FILE);
        return;
    }

    // Пишем в файл
    fprintf(file, "==================================\n");
    fprintf(file, "День %d, время %02d:00\n", current_day, current_hour);
    fprintf(file, "Фермер: %s\n", farmer_name);
    fprintf(file, "Инвентарь:\n");

    // Дублируем на экран
    printf("\n==================================\n");
    printf("День %d, время %02d:00\n", current_day, current_hour);
    printf("Фермер: %s\n", farmer_name);
    printf("Инвентарь:\n");

    int has_items = 0;
    for (int i = 0; i < INVENTORY_SIZE; i++) {
        if (inventory[i] != EMPTY_ID) {
            fprintf(file, "  - %s (слот %d)\n", item_names[inventory[i]], i);
            printf("  - %s (слот %d)\n", item_names[inventory[i]], i);
            has_items = 1;
        }
    }

    if (!has_items) {
        fprintf(file, "  (пусто)\n");
        printf("  (пусто)\n");
    }
    fprintf(file, "==================================\n\n");
    printf("==================================\n");

    fclose(file); // Файл закрыт и сохранен на диск
    
    char cwd[1024];
    if (_getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Файл diary.txt сохранен в папке: %s\n", cwd);
    }
    printf("Состояние успешно записано в файл %s\n", DIARY_FILE);

    // Пауза, чтобы успеть прочитать результат
    printf("\nНажмите Enter, чтобы вернуться в меню...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Ищет слово и оборачивает в [] (вспомогательная функция для энциклопедии)
int highlight_word(char *line, const char *search_word) {
    char temp_line[MAX_LINE_LEN];
    char temp_search[64];
    char result[MAX_LINE_LEN];   // <-- буфер для результата

    strcpy(temp_line, line);
    strcpy(temp_search, search_word);

    if (isupper((unsigned char)temp_line[0])) temp_line[0] = tolower((unsigned char)temp_line[0]);
    if (isupper((unsigned char)temp_search[0])) temp_search[0] = tolower((unsigned char)temp_search[0]);

    char *pos = strstr(temp_line, temp_search);
    if (pos == NULL) return 0;

    int start_index = (int)(pos - temp_line);
    int word_len = strlen(temp_search);
    int line_len = strlen(line);

    if (start_index + word_len > line_len) {
    return 0; // слово выходит за границы — не выделяем
    }

    // Собираем результат: до слова + '[' + слово + ']' + после слова
    strncpy(result, line, start_index);
    result[start_index] = '\0';
    strcat(result, "[");
    strncat(result, line + start_index, word_len);
    strcat(result, "]");
    strcat(result, line + start_index + word_len);

    // Копируем обратно в line
    strcpy(line, result);
    return 1;
}
// Расшифровка старых записей п 9
void process_encyclopedia() {
    char search_word[64];
    safe_input_string("Введите слово для поиска: ", search_word, sizeof(search_word));
    
    if (strlen(search_word) == 0) {
        printf("Ошибка: пустой запрос.\n");
        return;
    }

    FILE *fin = fopen(INPUT_FILE, "r"); // Читаем
    if (fin == NULL) {
        printf("[Ошибка] Не удалось открыть файл %s для чтения!\n", INPUT_FILE);
        return;
    }

    FILE *fout = fopen(OUTPUT_FILE, "w"); // Пишем
    if (fout == NULL) {
        printf("[Внимание] Не удалось создать %s (возможно, папка защищена).\n", OUTPUT_FILE);
        printf("Результаты будут выведены только на экран.\n");
    }

    char line[MAX_LINE_LEN];
    int line_number = 1;
    int found_count = 0;

    printf("\n --- Результат поиска ---\n");
    while (fgets(line, sizeof(line), fin)) {
        line[strcspn(line, "\r\n")] = 0; 
        
        if (highlight_word(line, search_word)) {
            found_count++;
            printf("%d: %s\n", line_number, line);
            
            // Защита: пишем в файл только если он открылся
            if (fout != NULL) {
                fprintf(fout, "%d : %s\n", line_number, line);
            }
        }
        line_number++;
    }

    fclose(fin);
    // Защита: закрываем файл только если он открылся
    if (fout != NULL) {
        fclose(fout);
    }

    if (found_count == 0) {
        printf("Слово \"%s\" не найдено ни в одной статье.\n", search_word);
    } else {
        printf("\n[УСПЕХ] Найдено статей: %d.\n", found_count);
        if (fout != NULL) {
            printf("Результат сохранен в %s\n", OUTPUT_FILE);
        }
    }

        // Надёжная пауза
    printf("\nНажмите Enter, чтобы вернуться в меню...");
    fflush(stdout);
    getchar(); // просто ждём один символ
}

// ТЕЛЕБАШНЯ

// Очистка экрана
void clearScreen() {
    printf("\033[H\033[J");
}

// ОСВОБОЖДЕНИЕ ПАМЯТИ (требование лабы!)
void freeMap(GameState *game) {
    if (game->map != NULL) {
        for (int i = 0; i < game->height; i++) {
            if (game->map[i] != NULL) {
                free(game->map[i]); // Чистим строки
            }
        }
        free(game->map); // Чистим массив указателей
        game->map = NULL; 
    }
    game->width = 0;
    game->height = 0;
}
 
// Генерация карты
void generateMap(GameState *game) {
    freeMap(game); // Очищаем старую
    
    // Случайные размеры
    game->height = MIN_SIZE + rand() % (MAX_SIZE - MIN_SIZE + 1);
    game->width = MIN_SIZE + rand() % (MAX_SIZE - MIN_SIZE + 1);
    
    // Выделяем память под массив указателей
    game->map = (char**)malloc(game->height * sizeof(char*));
    if (game->map == NULL) {
        printf("Ошибка выделения памяти!\n");
        exit(1);
    }
    
    // Выделяем память под каждую строку и заполняем
    for (int i = 0; i < game->height; i++) {
        game->map[i] = (char*)malloc(game->width * sizeof(char));
        if (game->map[i] == NULL) {
            printf("Ошибка выделения памяти!\n");
            freeMap(game); // Чистим то, что успели
            exit(1);
        }
        
        for (int j = 0; j < game->width; j++) {
            // Края - стены
            if (i == 0 || i == game->height - 1 || j == 0 || j == game->width - 1) {
                game->map[i][j] = WALL;
            } else {
                int r = rand() % 100;
                if (r < CHANCE_OBSTACLE) {
                    game->map[i][j] = (rand() % 2 == 0) ? TREE : ROCK;
                } else {
                    game->map[i][j] = MAP_EMPTY;
                }
            }
        }
    }
    placeLadderAndRunes(game); 
}

// Ставим героя
void placeHero(GameState *game, int startX, int startY) {
    game->hero.x = startX;
    game->hero.y = startY;
    game->map[startY][startX] = LADDER_DOWN; 
}

// Расстановка лестниц и рун
void placeLadderAndRunes(GameState *game) {
    bool placeDown = false;
    int startX = 0, startY = 0;
    int attempts = 0;
    
    // Ищем место для лестницы вниз
    while (!placeDown && attempts < 1000) { // Защита от зависания!
        int x = 1 + rand() % (game->width - 2);
        int y = 1 + rand() % (game->height - 2);  
        if (game->map[y][x] == MAP_EMPTY) {
            startX = x;
            startY = y;
            placeDown = true;
        }
        attempts++;
    }
    if (!placeDown) { // Если не нашли, ставим принудительно
        startX = game->width / 2;
        startY = game->height / 2;
        game->map[startY][startX] = MAP_EMPTY;
    }
    placeHero(game, startX, startY);
    
    // Ищем место для лестницы вверх
    bool placeUp = false;
    attempts = 0;
    while (!placeUp && attempts < 1000) {
        int x = 1 + rand() % (game->width - 2);
        int y = 1 + rand() % (game->height - 2);
        if (game->map[y][x] == MAP_EMPTY) {
            game->map[y][x] = LADDER_UP;
            placeUp = true;
        }
        attempts++;
    }
    
    // Расставляем руны (1-3 штуки)
    int numRunes = 1 + rand() % 3;
    for (int i = 0; i < numRunes; i++) {
        attempts = 0;
        while (attempts < 100) {
            int x = 1 + rand() % (game->width - 2);
            int y = 1 + rand() % (game->height - 2);
            if (game->map[y][x] == MAP_EMPTY) {
                game->map[y][x] = RUNE;
                break;
            }
            attempts++;
        }
    } 
}

// Отрисовка карты
void drawMap(const GameState *game) {
    printf("Этаж: %d\n", game->floor);
    printf("Управление: W(вверх), A(влево), S(вниз), D(вправо), Q(выход на D)\n\n");

    for (int i = 0; i < game->height; i++) {
        for (int j = 0; j < game->width; j++) {
            if (i == game->hero.y && j == game->hero.x) {
                printf("\033[1;32m%c\033[0m", HERO); // Герой (зеленый)
            } 
            else if (game->map[i][j] == LADDER_UP) {
                printf("\033[1;34m%c\033[0m", LADDER_UP); // Лестница вверх (синяя)
            } 
            else if (game->map[i][j] == LADDER_DOWN) {
                printf("\033[1;35m%c\033[0m", LADDER_DOWN); // Лестница вниз (фиолетовая)
            }
            else if (game->map[i][j] == RUNE) {
                printf("\033[1;31m%c\033[0m", RUNE); // Руна (красная)
            } 
            else {
                printf("%c", game->map[i][j]); 
            }
        }
        printf("\n");
    }
}

// Телепортация при наступании на руну
void handleRuneTeleport(GameState *game) {
    printf("\nВы наступили на руну. Телепортация...\n");
    bool found = false;
    int attempts = 0;
    
    while (!found && attempts < 1000) { // Защита от зависания
        int x = 1 + rand() % (game->width - 2);
        int y = 1 + rand() % (game->height - 2);
        if (game->map[y][x] == MAP_EMPTY) {
            game->map[game->hero.y][game->hero.x] = MAP_EMPTY; // Старая клетка пустеет
            game->hero.x = x;
            game->hero.y = y;
            found = true;
        }
        attempts++;
    }
}

// Обработка нажатий
void processInput(GameState *game, char input) {
    int dx = 0;
    int dy = 0;

    switch (tolower(input)) {
        case 'w': dy = -1; break; 
        case 's': dy = 1; break;  
        case 'a': dx = -1; break; 
        case 'd': dx = 1; break;  
        case 'q': 
            // Выход только на лестнице вниз
            if (game->map[game->hero.y][game->hero.x] == LADDER_DOWN) {
                printf("\nВы сбежали из башни домой!\n");
                game->isRunning = false;
            } else {
                printf("\nВыход домой (Q) работает только на лестнице вниз (D)!\n");
            }
            return;
        default: return;
    }

    int newX = game->hero.x + dx;
    int newY = game->hero.y + dy;

    // Проверка границ
    if (newX < 0 || newX >= game->width || newY < 0 || newY >= game->height) {
        return;
    }
    
    char targetCell = game->map[newY][newX];
    // Проверка препятствий
    if (targetCell == WALL || targetCell == TREE || targetCell == ROCK) {
        return; 
    }
    
    // Переход на этаж
    if (targetCell == LADDER_UP) {
        printf("\nВы перешли на следующий этаж!\n");
        game->floor++;
        generateMap(game); 
        return;
    }
    
    // Руна
    if (targetCell == RUNE) {
        game->hero.x = newX;
        game->hero.y = newY;
        handleRuneTeleport(game); 
        return;
    }
    
    // Обычный шаг
    game->hero.x = newX;
    game->hero.y = newY;
}

// Запуск игры в Башню
void start_adventure() {
    GameState game;
    game.map = NULL;
    game.width = 0;
    game.height = 0;
    game.floor = 1;
    game.isRunning = true;
    game.hero.x = 0;
    game.hero.y = 0;

    clearScreen();
    generateMap(&game); 

    while (game.isRunning) {
        clearScreen();
        drawMap(&game); 
        
        printf("\nВаш ход (W/A/S/D, Q - выход): ");
        char move;
        if (scanf(" %c", &move) != 1) {
            game.isRunning = false;
            break;
        }
        processInput(&game, move);
    }
    
    freeMap(&game); // Освобождаем память
    printf("\nВозврат в главное меню...\n");
    printf("Нажмите Enter для продолжения...");
    fflush(stdout);

    // 1) Съедаем остаток строки после scanf(" %c") — там лежит '\n'
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    // 2) Теперь реально ждём Enter
    while ((c = getchar()) != '\n' && c != EOF);
}

// MAIN (точка входа)
int main(void) {
#ifdef _WIN32
    // Проверяем GetConsoleMode; если флаг не поддерживается — просто игнорируем
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        // Если SetConsoleMode вернёт FALSE (старая Windows / Wine / перенаправление),
        // ничего страшного: цвета просто не будут отображаться
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif
    setlocale(LC_ALL, "Russian");
    srand((unsigned)time(NULL)); // Инициализация рандома

    // Стартовый инвентарь задан при глобальной инициализации массива inventory
    
    load_catalog(item_names);
    input_farmer_name(farmer_name);

    int choice;
    int running = 1;

    while (running) {
        print_menu();
        choice = get_int_input("Выберите пункт меню: ");

        if (choice < MENU_MIN || choice > MENU_MAX) {
            printf("Ошибка: введите число от %d до %d.\n", MENU_MIN, MENU_MAX);
            continue;
        }

        switch (choice) {
        case 0:  running = 0; break; // Выход
        case 1:  PRINT_CLOCK(); break;
        case 2:  handle_work(); break;           
        case 3:  show_inventory_only(); break;  
        case 4:  modify_inventory(); break;     
        case 5:  drop_item(); break; 
        case 6:  quick_access_task(); break;     
        case 7:  search_item(); break;
        case 8:  write_diary(); break;
        case 9:  process_encyclopedia(); break;  
        case 10: start_adventure(); break; 

        default: break;
        }
    }

    printf("Выход из программы. До встречи, %s!\n", farmer_name);
    return 0;
}
