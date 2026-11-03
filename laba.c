#include <stdio.h>
#pragma warning(disable:4996)
#include <windows.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 32
#define INVENTORY_SIZE 10
#define hours_in_day 24
// это стр
#define MAX_ITEMS 10
#define MAX_NAME_LEN 32
#define MMAX_ITEM_NAME_LEN 50
#define MAX_LINE_LEN 256// ПОКА

#define wOOD_ID 1
#define STONE_ID 2
#define SEED_ID 3
#define PICKAXE_ID 4
#define TORCH_ID 5
#define FOOD_ID 6
#define ROPE_ID 7
#define MAP_ID 8
#define KEY_ID 9 

// наверно все названия лучше вперед
// имена файлов
const char* ITEMS_FILE = "items.txt";
const char* DIARY_FILE ="diary.txt";
const char* INPUT_FILE = "input.txt";
const char* OUTPUT_FILE = "output.txt";

// глобальные переменные
char farmer_name[MAX_NAME_LEN];
char item_names[MAX_ITEMS][MMAX_ITEM_NAME_LEN];
int inventory[MAX_ITEMS];

// переместили

int get_int_input(){
    int value;
    char c;

    while (scanf("%d", &value) !=1){
        while ((c=getchar()) != '\n' && c!= EOF);
        printf("ОШИБКА. НЕКОРРЕКТНЫЙ КОДЮ ВВВЕДИТЕ, ПОЖАЙЛУСТА ЧИСЛО:");  
    }
    while((c = getchar()) !='\n' && c != EOF);
    
    return value;
}

void print_inventory(int inv[INVENTORY_SIZE]);
const char* get_item_name(int id);
//     int get_int_input();
// }
void print_inventory(int inv[INVENTORY_SIZE]){
    for (int i = 0; i < INVENTORY_SIZE; i++){
        printf("слот %d: [%d] (%s)\n",i, inv[i], get_item_name(inv[i]));
    }
}
const char* get_item_name(int id){
    switch (id){
        case 0:
            return "пустое";
        case wOOD_ID:
            return "дерево";
        case STONE_ID:
            return "камень";
        case SEED_ID:
            return "семена";
        case PICKAXE_ID:
            return "кирка";
        case TORCH_ID:
            return "факел";
        case FOOD_ID:
            return "еда";
        case ROPE_ID:
            return "веревка";
        case MAP_ID:
            return "карта";
        case KEY_ID:
            return "ключ";
        default:
            return "неизвестный предмет";
    }
}

int main() {
    //setlocale(LC_ALL, "Russian");
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);
    int current_day=1;
    int current_hour = 8;

    int inventory[INVENTORY_SIZE]={
        0,wOOD_ID, SEED_ID, STONE_ID,PICKAXE_ID,0,0,0,0,0
    };
    int choice;

    while(1){
        printf("\n ---  МЕНЮ ---\n");
        printf("Выход\n");
        printf("посмотреть на часы\n");
        printf("промотать время(поработать)\n");
        printf("посмотреть инквентарь\n");
        printf("положить предмет в слот\n");
        printf("выбросить предмет\n");
        printf("выполнить задание по варианту\n");

        choice=get_int_input("выберете пункт меню:0");

        switch(choice) {

            case 0:{
                printf("Выход из игры! До встречиЁ\n");
                return 0;
            }
            case 1:
            printf("текущее время : день %d,%02d:00\n",current_day, current_hour);
            break;

            case 2:{
                printf("сколько часов хотите поработат?", 0, 100000);
                int hours = get_int_input();
                if (hours <0){
                    printf("ошибка: кол-во часов не может быть отрицательным\n");
                } else{
                    current_day+=hours/hours_in_day;
                    current_hour = (current_hour+hours) % hours_in_day;
                    printf("стало: день %d,%02d:00\n",current_day,current_hour);
                }
                break;
            }
            case 3:
                printf("содержимое инвенторя\n");
                print_inventory(inventory);
                break;
            case 4:{
                printf("введите индекс слота(0-%d):", INVENTORY_SIZE - 1);
                int slot = get_int_input();

                if (slot <0 || slot >= INVENTORY_SIZE){
                    printf ("ошибка: индекс слота выходит за границы массива\n");
                } else{
                    printf("введите id предмета (1-9):");
                    int item_id=get_int_input();
                    inventory[slot]= item_id;
                    printf("предмет с id %d успешно добавлен в слот\n", slot);
                }
                break;
            }

            case 5:{
                printf("введите индекс слота на удаление(0-%d):", INVENTORY_SIZE - 1);
                int slot = get_int_input();
                  if (slot <0 || slot >= INVENTORY_SIZE){
                    printf ("ОШИБКА: неверный индекс слота\n");
                } else{
                    printf("введите id предмета (1-9):");
                    int item_id=get_int_input();
                    inventory[slot]=item_id;
                    printf("слот %d успешно очищен %d.\n,item_id, slot");
                }
                break;
            }
            case 6:{
                printf("инвентарь до выполнения задания\n");
                print_inventory(inventory);

                printf("введите id для быстрого доступа:");
                int target_id = get_int_input();

                int found_index= -1;
                for (int i =0; i< INVENTORY_SIZE; i++){
                    if (inventory[i] == target_id){
                        found_index = i;
                        break;
                    }
                }
                if (found_index == -1) {
                    printf("предмет с id %d не найден в инвенторе\n", target_id);
                } else if (found_index==0) {
                    printf("предмет с id %d уже находиться в нулевом слоте\n", target_id);
                } else {

                    int temp =inventory[0];
                    inventory[0] = inventory [found_index];
                    inventory [found_index] = temp;
                    printf("предмет с id %d перемещен в слот 0(обмен с ячейкой %d)\n", target_id,found_index);
                }

                printf("инвентарь после выполнения задания\n");
                print_inventory(inventory);
                break;
            }
            default:
                printf("невнрный пункт меню. Выберете от 0 до 6ю\n");
                break;
        
        }
    }
    return 0;
}





