#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct blk {
    uint32_t index;
    uint32_t index_of_data;
    uint32_t size_of_data;
    uint32_t index_of_prev_block;
    uint32_t index_of_next_block;
    struct blk *prev;
    struct blk *next;
};

typedef struct blk block;

void dump();
int alloc_at_begining(uint32_t);
int alloc_before_first_block(uint32_t);
int alloc_between(int, uint32_t);
int alloc_at_end(uint32_t);
void alloc(uint32_t);
void erase(uint32_t);
void fill(uint32_t, uint32_t, uint32_t);
void map_block_to_arena(block *);
void unmap_block_from_arena(block *);
void parse_command(char*);

int blocks = 0;
unsigned char* arena = NULL;
uint32_t size_of_arena;
block *first = NULL;
block *last = NULL;

int main(void)
{
    ssize_t read;
    char* line = NULL;
    size_t len;

    /* parse input line by line */
    while ((read = getline(&line, &len, stdin)) != -1) {
        /* print every command to stdout */
        printf("%s", line);

        parse_command(line);

    }

    free(line);

    return 0;
}

void parse_command(char* cmd)
{
    const char* delims = " \n";

    char* cmd_name = strtok(cmd, delims);
    if (!cmd_name) {
        goto invalid_command;
    }

    if (strcmp(cmd_name, "INITIALIZE") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        /* alocarea memoriei pentru arena si setarea ei pe 0 */
        size_of_arena = size;
        arena = malloc(size_of_arena);
        memset(arena, 0, size_of_arena);

    } else if (strcmp(cmd_name, "FINALIZE") == 0) {
        /* eliberarea memoriei arenei */
        free(arena);
        block *p = first;
        /* cat timp exista blocul curent */
        while(p != NULL) {
            /* primul bloc este marcat ca find blocul urmator celui curent */
            first = first->next;
            /* stergerea blocului curent */
            p->prev = NULL;
            p->next = NULL;
            free(p);
            /* blocul curent devine primul bloc */
            p = first;
        }
        first = NULL;
        last = NULL;
        arena = NULL;
        blocks = 0;

    } else if (strcmp(cmd_name, "DUMP") == 0) {
        dump();

    } else if (strcmp(cmd_name, "ALLOC") == 0) {
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        alloc(size);

    } else if (strcmp(cmd_name, "FREE") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        erase(index);

    } else if (strcmp(cmd_name, "FILL") == 0) {
        char* index_str = strtok(NULL, delims);
        if (!index_str) {
            goto invalid_command;
        }
        uint32_t index = atoi(index_str);
        char* size_str = strtok(NULL, delims);
        if (!size_str) {
            goto invalid_command;
        }
        uint32_t size = atoi(size_str);
        char* value_str = strtok(NULL, delims);
        if (!value_str) {
            goto invalid_command;
        }
        uint32_t value = atoi(value_str);
        fill(index, size, value);

    } else if (strcmp(cmd_name, "ALLOCALIGNED") == 0) {
        exit(0);

    } else if (strcmp(cmd_name, "REALLOC") == 0) {
        exit(0);

    } else {
        goto invalid_command;
    }

    return;

invalid_command:
    printf("Invalid command: %s\n", cmd);
    exit(1);
}

void map_block_to_arena(block *b) {
    /* indexul de inceput al zonei de date este intotdeauna cu 12 octeti mai
        mare decat indexul de inceput al blocului */
    b->index_of_data = b->index + 12;

    /* pointer de tip int la inceputul zonei de gestiune a blocului */
    uint32_t *int_arena = (uint32_t*)(arena + b->index);
    /* scrierea zonei de gestiune a blocului in arena */
    int_arena[0] = b->index_of_next_block;
    int_arena[1] = b->index_of_prev_block;
    int_arena[2] = b->size_of_data;
}

void unmap_block_from_arena(block *b) {
    /* setarea pe 0 a tuturor octetilor blocului */
    for (uint32_t i = 0; i < b->size_of_data + 12; ++i)
        arena[b->index + i] = 0;
    /* eliberarea memoriei blocului */
    b->next = NULL;
    b->prev = NULL;
    free(b);
}

void dump() {
    /* se parcurge pe rand fiecare octat din arena */
    for (unsigned int i = 0; i < size_of_arena; ++i) {
        /* daca este octatul de inceput de rand afisam si indexul curent */
        if (i % 16 == 0)
            printf("%08X\t", i);
        /* dupa fiecare al 8-lea octet se afiseaza un spatiu suplimentar */
        else if (i % 8 == 0 && i != 0)
            printf(" ");
        /* afisarea octetului in format hexazecimal */
        printf("%02X ", arena[i]);
        /* daca urmatorul octet este multiplu de 16, trecem la un rand nou */
        if ((i + 1) % 16 == 0 && i != 0)
            printf("\n");
    }
    /* dac ultimul rand din arena nu a fost complet vom trece la un rand nou */
    if (size_of_arena % 16 != 0)
        printf("\n");
}

void alloc(uint32_t size) {
    /* daca nu este nici un bloc alocat in arena, vom aloca la inceputul arenei */
    if (first == NULL) {
        printf("%d\n", alloc_at_begining(size));
        return;
    }
    /* daca este spatiu, vom aloca inaintea primul bloc */
    if (first->index >= size + 12) {
        printf("%d\n", alloc_before_first_block(size));
        return;
    }
    /* parcurgem toate blocurile si incercam alocarea intre blocul i si blocul
        i + 1, daca alocarea a avut succes ne oprim */
    for (int i = 0; i < blocks - 1; ++i) {
        int output = alloc_between(i, size);
        if (output != 0) {
            printf("%d\n", output);
            return;
        }
    }
    /* alocarea blocului la final */
    printf("%d\n", alloc_at_end(size));
}

int alloc_at_begining(uint32_t size) {
    /* daca exista spatiu suficient */
    if (size + 12 > size_of_arena)
        return 0;
    /* incrementarea numarului de blocuri si alocarea unei structuri block */
    blocks++;
    block *b = malloc(sizeof(block));
    /* completarea campurilor structurii block si maparea acesteia in arena */
    b->index = 0;
    b->size_of_data = size;
    b->index_of_prev_block = 0;
    b->index_of_next_block = 0;
    b->prev = NULL;
    b->next = NULL;
    map_block_to_arena(b);
    /* blocul nou alocat va fi si primul si ultimul */
    first = b;
    last = b;
    /* indexul zonei de date */
    return b->index_of_data;
}

int alloc_before_first_block(uint32_t size) {
    /* incrementarea numarului de blocuri si alocarea unei structuri block */
    blocks++;
    block *b = malloc(sizeof(block));
    /* completarea campurilor structurii alocate */
    b->index = 0;
    b->size_of_data = size;
    b->index_of_prev_block = 0;
    b->index_of_next_block = first->index;
    b->prev = NULL;
    /* marcarea legaturii intre blocul first si blocul nou alocat */
    first->index_of_prev_block = b->index;
    b->next = first;
    first->prev = b;
    /* remaparea blocului first si maparea blocului nou alocat */
    map_block_to_arena(b);
    map_block_to_arena(first);
    /* blocul nou alocat va deveni primul */
    first = b;
    /* indexul zonei de date */
    return b->index_of_data;
}

int alloc_between(int block_index, uint32_t size) {
    block *a = first;
    /* se parcurg blocurile, pana la gasirea celui cu indexul block_index */
    for (int i = 0; i < block_index; ++i)
        a = a->next;
    /* blocul consecutiv celui gasit anterior */
    block *c = a->next;
    /* daca nu exitsta spatiu suficient intre cele 2 blocuri abandonam alocarea
        intre acestea 2 */
    if (size + 12 > c->index - (a->index_of_data + a->size_of_data))
        return 0;
    /* incrementarea numarului de blocuri si alocarea unei structuri block */
    blocks++;
    block *b = malloc(sizeof(block));
    /* completarea campurilor structurii alocate */
    b->index_of_prev_block = a->index;
    b->index_of_next_block = c->index;
    b->size_of_data = size;
    b->index = a->index_of_data + a->size_of_data;

    /* actualizarea indexilor blocurilor vecine */
    c->index_of_prev_block = b->index;
    a->index_of_next_block = b->index;

    /* formarea legaturii intre blocul nou adaugat si cele vecine */
    a->next = b;
    b->prev = a;
    b->next = c;
    c->prev = b;

    /* remaparea blocurilor in arena */
    map_block_to_arena(a);
    map_block_to_arena(b);
    map_block_to_arena(c);

    /* indexul zonei de date */
    return b->index_of_data;
}

int alloc_at_end(uint32_t size) {
    /* daca nu exista spatiu dupa ultimul bloc din arena nu se poate aloca blocul cerut*/
    if (last->index_of_data + last->size_of_data + size + 12 > size_of_arena)
        return 0;
    /* incrementarea numarului de blocuri si alocarea unei structuri block */
    blocks++;
    block *b = malloc(sizeof(block));
    /* completarea campurilor structurii alocate */
    b->index_of_prev_block = last->index;
    b->index_of_next_block = 0;
    b->size_of_data = size;
    /* actualizarea indexilor blocurilor nou alocat si ultimului bloc */
    b->index = last->index_of_data + last->size_of_data;
    last->index_of_next_block = b->index;

    /* formarea legaturii intre ultimul bloc si cel nou alocat */
    b->prev = last;
    b->next = NULL;
    last->next = b;

    /* remaparea blocurilor in arena */
    map_block_to_arena(b);
    map_block_to_arena(last);

    /* blocul nou alocat va deveni ultimul */
    last = b;
    /* indexul zonei de date */
    return b->index_of_data;
}

void fill(uint32_t index, uint32_t s, uint32_t value) {
    block *b = first;
    /* cautarea blocului al carui index de date este egal cu indexul primit */
    while (b != NULL && b->index_of_data != index)
        b = b->next;
    int size = s;
    /* cat timp mai sunt date de scris si exista blocul curent */
    while (size > 0 && b != NULL) {
        /* din dimensiunea ce mai trebuie scrisa scadem dimensiunea de date
            a blocului curent */
        size -= b->size_of_data;
        /* daca diemnsiunea ce mai trebuie scrisa este pozitica vom completa
            tot blocul curent */
        if (size > 0)
            for (unsigned int i = 0; i < b->size_of_data; ++i)
                arena[b->index_of_data + i] = value;
        else
            /* altfel vom completa doar o parte a blocului curent */
            for (unsigned int i = 0; i < b->size_of_data + size; ++i)
                arena[b->index_of_data + i] = value;
        /* mutarea pe blocul urmator */
        b = b->next;
    }
}

void erase(uint32_t index) {
    /* decrementarea numarului de blocuri */
    blocks--;
    block *b = first;
    /* cat timp exista blocul curent */
    while (b != NULL) {
        /* daca este blocul cu indexul cautat */
        if (b->index_of_data == index) {
            /* blocurile vecine blocului curent */
            block *a = b->prev;
            block *c = b->next;

            /* daca are ambii vecini */
            if (a != NULL && c != NULL) {
                /* refacem legatura intre ei si remapam blocurilei n arena */
                a->next = c;
                a->index_of_next_block = c->index;
                c->prev = a;
                c->index_of_prev_block = a->index;
                map_block_to_arena(a);
                map_block_to_arena(c);
            /* daca are doar vecinul stang */
            } else if (a != NULL && c == NULL) {
                /* marcam vecinul stang ca ultimul bloc si il remapam in arena */
                last = a;
                a->index_of_next_block = 0;
                a->next = NULL;
                map_block_to_arena(a);
            /* daca are doar vecinul drept */
            } else if (a == NULL && c != NULL) {
                /* marcam vecinul drept ca primul bloc si il remapam in arena */
                first = c;
                c->index_of_prev_block = 0;
                c->prev = NULL;
                map_block_to_arena(c);
            /* nu are nici un vecin */
            } else if (a == NULL && c == NULL) {
                first = NULL;
                last = NULL;
            }

            /* demapam blocul din arena si oprim cautarea */
            unmap_block_from_arena(b);
            break;
        } else {
            /* mutarea pe blocul urmator */
            b = b->next;
        }
    }
}
