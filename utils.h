#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> 

#define HASH_TABLE_SIZE 524287

typedef struct BDDNode {
    int var;
    char* bfunkcia;
    struct BDDNode* low;
    struct BDDNode* high;
    struct BDDNode* term_0;
    struct BDDNode* term_1;
} BDDNode;

typedef struct BDD {
    int num_vars;
    int size;
    BDDNode* root;
} BDD;

typedef struct HashEntry {
    int var;
    BDDNode* low;
    BDDNode* high;
    BDDNode* node;
} HashEntry;

typedef struct HashTable {
    HashEntry* entries[HASH_TABLE_SIZE];
} HashTable;

char* my_strdup(const char* str) {
    if (str == NULL) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        strcpy(copy, str);
    }
    return copy;
}

unsigned int hash(int var, BDDNode* low, BDDNode* high) {
    return ((size_t)low + (size_t)high + var) % HASH_TABLE_SIZE;
}

BDDNode* find_or_insert_node(HashTable* table, int var, char* bfunkcia, BDDNode* low, BDDNode* high, int* size_counter) {
    unsigned int h = hash(var, low, high);
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        int idx = (h + i) % HASH_TABLE_SIZE;
        HashEntry* e = table->entries[idx];
        if (e == NULL) break;
        if (e->var == var && e->low == low && e->high == high) {
            return e->node;
        }
    }

    BDDNode* node = (BDDNode*)malloc(sizeof(BDDNode));
    node->var = var;
    node->bfunkcia = my_strdup(bfunkcia);
    node->low = low;
    node->high = high;
    node->term_0 = NULL;
    node->term_1 = NULL;
    (*size_counter)++;

    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        int idx = (h + i) % HASH_TABLE_SIZE;
        if (table->entries[idx] == NULL) {
            table->entries[idx] = (HashEntry*)malloc(sizeof(HashEntry));
            table->entries[idx]->var = var;
            table->entries[idx]->low = low;
            table->entries[idx]->high = high;
            table->entries[idx]->node = node;
            break;
        }
    }

    return node;
}

// zjednodusi funkciu 'bfunkcia' (string) dosadenim hodnoty 'log_value' (0/1)
// za premennu 'lit' (urcenu pomocou 'poradie' a 'index').
// vrati novy string, ktory treba potom uvolnit cez free().
char* simplify_function(char* bfunkcia, char* poradie, int index, int log_value) {
    char lit = poradie[index];          // Premenna (literal), ktoru dosadzame
    size_t len = strlen(bfunkcia);      // Dlzka povodnej funkcie

    // Alokujeme pamat pre vysledny string (+1 pre '\0')
    // Alokujeme maximalnu moznu dlzku, aj ked vysledok bude casto kratsi.
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL; // Kontrola alokacie

    size_t result_len = 0;              // Aktualna dlzka vysledneho stringu
    int first_term = 1;                 // Priznak, ci pridavame prvy platny term (kvoli '+')
    char* term_start = bfunkcia;        // Ukazovatel na zaciatok aktualne spracovavaneho termu

    while (1) { // Cyklus prechadza term po terme
        char* term_end = term_start; // Najdeme koniec aktualneho termu ('+' alebo '\0')
        while (*term_end != '+' && *term_end != '\0') {
            term_end++;
        }

        // --- Spracovanie jedneho termu (segment od term_start po term_end) ---

        // Osetrenie prazdneho termu (napr. medzi "++" alebo za koncovym "+")
        if (term_start < term_end) {
            int skip_term = 0;           // Priznak, ci sa ma term zahodit (lebo je 0)
            char temp_term[128];         // Buffer pre zjednoduseny term
            size_t temp_len = 0;         // Dlzka zjednoduseneho termu
            char* term_ptr = term_start; // Ukazovatel na znaky v terme

            // Prejdeme cely aktualny term (neprazdny segment)
            while (term_ptr < term_end) {
                // Pripad: Negovany literal !lit
                // (pridana kontrola term_ptr + 1 < term_end, aby sme necitali mimo segment)
                if (*term_ptr == '!' && *(term_ptr + 1) == lit && term_ptr + 1 < term_end) {
                    if (log_value == 1) { // dosadzame 1: !1 = 0 -> cely term je 0
                        skip_term = 1;
                        break;
                    }
                    // ak dosadzame 0: !0 = 1 -> literal vypustime
                    term_ptr += 2;
                }
                // Pripad: Nenegovany literal lit
                // (opravena kontrola voci zaciatku povodneho stringu bfunkcia)
                else if (*term_ptr == lit && (term_ptr == bfunkcia || *(term_ptr - 1) != '!')) {
                    if (log_value == 0) { // dosadzame 0: 0 -> cely term je 0
                        skip_term = 1;
                        break;
                    }
                    // ak dosadzame 1: 1 -> literal vypustime
                    term_ptr++;
                }
                // Pripad: Iny literal
                else {
                    // Prekopirujeme ho do docasneho termu
                    if (temp_len < sizeof(temp_term) - 1) {
                        temp_term[temp_len++] = *term_ptr;
                    }
                    else {
                        // Chyba: Term je prilis dlhy pre buffer temp_term
                        free(result);
                        return NULL;
                    }
                    term_ptr++;
                }
            } // koniec while (term_ptr < term_end)

            temp_term[temp_len] = '\0'; // Ukoncime string zjednoduseneho termu

            // --- Pridanie zjednoduseneho termu do vysledku ---
            if (!skip_term) { // Ak sa term nema zahodit
                // Pridame '+' ak to nie je prvy term vo vysledku
                if (!first_term) {
                    // Kontrola, ci mame miesto na '+'
                    if (result_len < len) {
                        result[result_len++] = '+';
                    }
                    else { // Malo by miesto byt, ale pre istotu
                        free(result); return NULL;
                    }
                }

                // Ak sa NEPRAZDNY term zjednodusil na prazdny retazec -> znamena '1'
                if (temp_len == 0) {
                    // Kontrola, ci mame miesto na "1" a '\0'
                    if (result_len + 1 < len + 1) {
                        strcpy(result + result_len, "1");
                        result_len += 1;
                    }
                    else { // Pretecenie vysledneho buffera
                        free(result); return NULL;
                    }
                }
                else { // Inak pridame zjednoduseny term z temp_term
                    // Kontrola, ci mame miesto na term a '\0'
                    if (result_len + temp_len < len + 1) {
                        strcpy(result + result_len, temp_term);
                        result_len += temp_len;
                    }
                    else { // Pretecenie vysledneho buffera
                        free(result); return NULL;
                    }
                }
                first_term = 0; // Zaznacime, ze uz sme pridali aspon jeden platny term
            } // koniec if (!skip_term)
        } // koniec if (term_start < term_end) - preskocenie prazdneho termu

        // --- Posun na dalsi term alebo ukoncenie ---
        if (*term_end == '\0') {
            break; // Dosli sme na koniec vstupneho retazca
        }
        else {
            // Posunieme zaciatok dalsieho termu za aktualny '+'
            term_start = term_end + 1;
        }
    } // koniec while (1)

    result[result_len] = '\0'; // Ukoncime vysledny retazec
    return result;
}

BDDNode* BDD_recursive(char* bfunkcia, char* poradie, int index, int num_vars, int* size_counter, BDDNode* terminal_0, BDDNode* terminal_1, HashTable* table) {
    if (index >= num_vars) {
        return strstr(bfunkcia, "1") ? terminal_1 : terminal_0;
    }

    char* low_func = simplify_function(bfunkcia, poradie, index, 0);
    char* high_func = simplify_function(bfunkcia, poradie, index, 1);

    BDDNode* low = (low_func == NULL || strstr(low_func, "1")) ? terminal_1 :
        (strcmp(low_func, "\0") == 0) ? terminal_0 :
        BDD_recursive(low_func, poradie, index + 1, num_vars, size_counter, terminal_0, terminal_1, table);

    BDDNode* high = (high_func == NULL || strstr(high_func, "1")) ? terminal_1 :
        (strcmp(high_func, "\0") == 0) ? terminal_0 :
        BDD_recursive(high_func, poradie, index + 1, num_vars, size_counter, terminal_0, terminal_1, table);

    BDDNode* result = find_or_insert_node(table, index, bfunkcia, low, high, size_counter);

    free(low_func);
    free(high_func);

    return result;
}

BDD* BDD_create(char* bfunkcia, char* poradie) {
    BDD* bdd = (BDD*)malloc(sizeof(BDD));
    bdd->num_vars = strlen(poradie);
    bdd->size = 1; // je to od 1, kvoli root uzlu

    BDDNode* terminal_0 = malloc(sizeof(BDDNode));
    terminal_0->var = -1;
    terminal_0->bfunkcia = my_strdup("0");
    terminal_0->low = NULL;
    terminal_0->high = NULL;

    BDDNode* terminal_1 = malloc(sizeof(BDDNode));
    terminal_1->var = -1;
    terminal_1->bfunkcia = my_strdup("1");
    terminal_1->low = NULL;
    terminal_1->high = NULL;

    HashTable* table = (HashTable*)calloc(1, sizeof(HashTable));
    bdd->root = BDD_recursive(bfunkcia, poradie, 0, bdd->num_vars, &bdd->size, terminal_0, terminal_1, table);

    return bdd;
}


char* BDD_use(BDD* bdd, char* vstupy) {
    BDDNode* temp = bdd->root;
    size_t num_inputs = strlen(vstupy);

    for (int i = 0; i < num_inputs; i++) {
        if (temp->low == NULL && temp->high == NULL) {
            break;
        }
        if (vstupy[i] == '0') {
            temp = temp->low;
        }
        else { // Assume input is '1' if not '0'
            temp = temp->high;
        }
    }
    return temp->bfunkcia;
}

char* shuffle_string(char* str) {
    if (str == NULL) return;
    int n = strlen(str);
    if (n <= 1) return;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
    return str;
}


BDD* BDD_create_with_best_order(char* bfunkcia, char* poradie) {
    BDD* temp = NULL;
    for (int i = 0; i < 5; i++) {
        clock_t t_start = clock();
        BDD* current = BDD_create(bfunkcia, shuffle_string(poradie));
        clock_t t_end = clock();
        double cas = ((double)(t_end - t_start)) / CLOCKS_PER_SEC;
        printf("Cas: %f s\n", cas);
        if (temp == NULL) {
            printf("first: current %d\n", current->size);
            temp = current;
        }
        else if (current->size < temp->size) {
            printf("CORRECT: current: %d\t temp: %d\n", current->size, temp->size);
            temp = current;
        }
        else {
            printf("WRONG: current: %d\t temp: %d\n", current->size, temp->size);
        }
    }

    return temp;
}