#include <stdlib.h>
#include <stdio.h>

#include "cnf.h"

/** Funkce obslouží chybový stav programu
* @param error_msg chybový výstup
*/
void error(char* error_msg) {
    fprintf(stderr, "%s\n", error_msg);
    exit(-1);
}

/********************************************
**                                         **
**       Literály, klauzule a formule      **
**                                         **
********************************************/


struct Literal {
    int var;
    struct Literal *next_literal;
};

struct Clause {
    Literal* first_literal;
    Literal* last_literal;

    struct Clause* next_clause;

    unsigned num_of_regions;
    unsigned num_of_products;
};

struct CNF {
    Clause* first_clause;
    Clause* last_clause;

    unsigned num_of_clauses;
    unsigned num_of_regions;
    unsigned num_of_products;
};

/** Funkce přidá klauzuli do formule
* @param clause klauzule
* @param formula výroková formule
*/
void add_clause_to_formula(Clause *clause, CNF *formula) {
    assert(clause != NULL);
    assert(formula != NULL);

    if (formula->last_clause == NULL) {
        assert(formula->first_clause == NULL);
        formula->first_clause = clause;
    } else {
        formula->last_clause->next_clause = clause;
    }
    formula->last_clause = clause;
    clause->num_of_regions = formula->num_of_regions;
    clause->num_of_products = formula->num_of_products;

    ++formula->num_of_clauses;
}

/** Funkce vytvoří novou klauzuli
* @param formula výroková formule
* @return vytvořená klauzule
*/
Clause* create_new_clause(CNF* formula) {
    Clause *new_clause = malloc(sizeof(Clause));
    new_clause->first_literal = NULL;
    new_clause->last_literal = NULL;
    new_clause->next_clause = NULL;
    add_clause_to_formula(new_clause, formula);
    return new_clause;
}

/** Funkce přidá literál do klauzule. Literál je pozitivní nebo negativní
* výroková proměnná.
* @param clause klauzule
* @param is_positive příznak udávající, zda je proměnná pozitivní
* @param is_main_product příznak udávající, zda proměnná odpovídá hlavnímu produktu
* @param region index regionu
* @param product index produktu
*/
void add_literal_to_clause(Clause *clause, bool is_positive, bool is_main_product, unsigned region, unsigned product) {
    assert(clause != NULL);

    Literal *new_literal = malloc(sizeof(Literal));

    unsigned num_of_regions = clause->num_of_regions;
    unsigned num_of_products = clause->num_of_products;

    if (region >= num_of_regions) {
        error("Invalid region used.");
    }

    if (product >= num_of_products) {
        error("Invalid product used.");
    }

    // výpočet indexu proměnné
    int lit_num = num_of_products * region + product + 1;

    // indexy vedlejších proměnných jsou odsazeny o hodnotu K * P
    if (!is_main_product) { lit_num += num_of_products * num_of_regions; }

    // negativní proměnné jsou vyjádřeny pomocí záporného čísla
    if (!is_positive) {
        lit_num = -lit_num;
    }
    new_literal->var = lit_num;
    new_literal->next_literal = NULL;

    if (clause->last_literal == NULL) {
        assert(clause->first_literal == NULL);
        clause->first_literal = new_literal;
    } else {
        clause->last_literal->next_literal = new_literal;
    }
    clause->last_literal = new_literal;
}

/** Funkce vrátí počet proměnných výrokové formule
* @param formula výroková formule
*/
unsigned get_num_of_variables(CNF* formula) {
    assert(formula != NULL);
    return 2 * formula->num_of_products * formula->num_of_regions;
}

/** Funkce vrátí počet klauzulí výrokové formule
* @param formula výroková formule
*/
unsigned get_num_of_clauses(CNF* formula) {
    assert(formula != NULL);
    return formula->num_of_clauses;
}

/** Funkce uvolní paměť alokovanou pro uchování klauzule
* @param cl klauzule
*/
void clear_clause(Clause* cl) {
    assert(cl != NULL);
    while (cl->first_literal != NULL) {
        Literal *cur_lit = cl->first_literal;
        cl->first_literal = cl->first_literal->next_literal;
        free(cur_lit);
    }
    cl->last_literal = NULL;
}

/** Funkce uvolní paměť alokovanou pro uchování formule
* @param formula výroková formule
*/
void clear_cnf(CNF* formula) {
    assert(formula != NULL);
    while (formula->first_clause != NULL) {
        Clause *this_cl = formula->first_clause;
        formula->first_clause = formula->first_clause->next_clause;
        clear_clause(this_cl);
        free(this_cl);
    }
    formula->last_clause = NULL;
    formula->num_of_clauses = 0;
}

/** Funkce vytiskne vytvořenou formuli ve formátu DIMACS
* @param formula výroková formule
*/
void print_formula(CNF* formula) {
    assert(formula != NULL);

    printf("p cnf %u %u\n", get_num_of_variables(formula), get_num_of_clauses(formula));
    Clause *next_cl = formula->first_clause;
    while (next_cl != 0) {
        Literal *next_lit = next_cl->first_literal;
        while (next_lit != 0) {
            printf("%d ", next_lit->var);
            next_lit = next_lit->next_literal;
        }
        next_cl = next_cl->next_clause;
        printf("0\n");
    }
}

/*******************************
**                            **
**       Seznamy sousedů      **
**                            **
********************************/

/** Struktura uchovává jeden seznam sousedů a jeho velikost.
* Vyjadřuje veškeré sousedy jednoho konkrétního regionu, jemuž
* je seznam přidělen.
*/
struct NeighbourList {
    unsigned size; /**< velikost seznamu sousedů */
    unsigned *data; /**< indexy sousedů */
};

/** Struktura uchovává větší množství seznamů sousedů a počet těchto seznamů.
* Index do pole data odpovídá indexu regionu. Hodnota na tomto indexu obsahuje
* všechny sousedy daného regionu.
*/
struct NeighbourLists {
    unsigned size; /**< počet seznamů sousedů */
    NeighbourList *data; /**< seznamy sousedů */
};


/** Funkce přidá informace o dvou sousedících regionech fst, snd
* do seznamu sousedů. Informace o sousedící dvojici je přidána jen tehdy,
* pokud
* 1) dosud neexistuje
* 2) indexy sousedů nepřesahují povolený limit
* 3) nejde o dva stejné indexy (region nesousedí sám se sebou)
* @param lists seznam sousedů
* @param fst první soused
* @param snd druhý soused
*/
void add_neighbour(NeighbourLists *lists, unsigned fst, unsigned snd) {
    if (lists == NULL || !lists->size) {
        error("Internal error.\n");
    }
    if (fst >= lists->size || snd >= lists->size) {
        error("Neighbour indices are too high.\n");
    }
    if (fst == snd) {
        error("Reflexive neighbours are not allowed.\n");
    }

    unsigned curr_size = lists->data[fst].size;
    // kontrola existující informace o sousednosti
    for (unsigned i = 0; i < curr_size; ++i) {
        if (lists->data[fst].data[i] == snd) { return; }
    }

    // přidávání nového souseda
    unsigned *tmp = (unsigned *)realloc(lists->data[fst].data, (curr_size + 1) * sizeof(unsigned));
    if (tmp == NULL) {
        error("Internal error.\n");
    }
    tmp[curr_size] = snd;
    lists->data[fst].data = tmp;
    ++lists->data[fst].size;
}

/** Pomocná funkce, která zobrazuje, jakým způsobem byl vstupní soubor
* převeden na seznam sousedů.
* @param lists seznam sousedů
*/
void print_neighbours(NeighbourLists *lists) {
    printf("size: %d\n",lists->size);
    printf("data:\n");
    for (unsigned i = 0; i < lists->size; ++i) {
        printf("%d -> ",i);
        for (unsigned j = 0; j < lists->data[i].size; ++j) {
            printf("%d ",lists->data[i].data[j]);
        }
        printf("\n");
    }
}

/** Uvolnění alokované paměti použité pro uchování seznamu sousedů.
* @param lists seznam sousedů
*/
void clear_neighbours(NeighbourLists *lists) {
    if (lists == NULL) { return; }
    for (unsigned i = 0; i < lists->size; ++i) {
        if (lists->data[i].data == NULL) { continue; }
        free(lists->data[i].data);
    }
    free(lists->data);
}

/** Predikát rozhodující, zda dané dva indexy odpovídají sousedícím regionům
* @param lists seznam sousedů
* @param fst první region
* @param snd druhý region
* @return true, pokud fst sousedí se snd
*/
bool are_neighbours(const NeighbourLists *lists, unsigned fst, unsigned snd) {
    if (lists == NULL || !lists->size) {
        error("Internal error.\n");
    }

    // indexy regionů nesmí přesahovat povolený limit
    if (fst >= lists->size || snd >= lists->size) { return false; }

    // kontrola sousednosti
    for (unsigned i = 0; i < lists->data[fst].size; ++i) {
        if (lists->data[fst].data[i] == snd) { return true; }
    }
    return false;
}

int main (int argc, char** argv) {

    // program musí být spuštěn s jediným argumentem odpovídajícím
    // názvu souboru v korektním formátu
    if (argc != 2) {
        error("Exactly one argument is expected. Please type the name of an input file.\n");
    }

    FILE *input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        error("The input file could not be opened.\n");
    }

    // načtení hlavičky vstupního souboru
    unsigned num_of_regions, num_of_products;
    if (fscanf(input_file, "%u %u", &num_of_regions, &num_of_products) != 2) {
        fclose(input_file);
        error("Invalid header. The header should contain exactly two numbers:\nnum_of_regions num_of_products\n");
    }

    // musí existovat alespoň jeden region
    if (num_of_regions == 0) {
        fclose(input_file);
        error("The number of regions has to be positive.\n");
    }

    // musí existovat alespoň jeden produkt
    if (num_of_products == 0) {
        fclose(input_file);
        error("The number of products has to be positive.\n");
    }

    // inicializace seznamu sousedů
    NeighbourLists neighbours = {.size = num_of_regions, .data = NULL};
    neighbours.data = (NeighbourList *)malloc(num_of_regions * sizeof(NeighbourList));
    if (neighbours.data == NULL) {
        error("Internal error.\n");
    }
    for (unsigned i = 0; i < num_of_regions; ++i) {
        neighbours.data[i].size = 0;
        neighbours.data[i].data = NULL;
    }

    // načítání informací o sousednosti regionů
    unsigned fst, snd;
    while (1) {
        int res = fscanf(input_file, "%u %u", &fst, &snd);
        if (res == 2) {
            add_neighbour(&neighbours, fst, snd);
            add_neighbour(&neighbours, snd, fst);
        } else if (res == EOF) { break; }

        else {
            fclose(input_file);
            error("Invalid input file.\n");
        }
    }

    // inicializace výsledné formule
    CNF f = { .first_clause = NULL, .last_clause = NULL, .num_of_clauses = 0, .num_of_regions = num_of_regions, .num_of_products = num_of_products };

    // konstrukce klauzulí
    all_regions_min_one_main_product(&f, num_of_regions, num_of_products);
    all_regions_max_one_main_product(&f, num_of_regions, num_of_products);
    all_regions_max_one_side_product(&f, num_of_regions, num_of_products);
    main_side_products_different(&f, num_of_regions, num_of_products);
    neighbour_regions_different_main_products(&f, num_of_regions, num_of_products, &neighbours);
    all_products_at_least_once_main_products(&f, num_of_regions, num_of_products);
    no_side_product_in_main_region(&f, num_of_regions, num_of_products);
    main_region_main_product_as_side_product_elsewhere(&f, num_of_regions, num_of_products);

    // výpis formule
    printf("c Formula:\n");
    print_formula(&f);

    // uvolnění alokované paměti
    clear_neighbours(&neighbours);
    clear_cnf(&f);

    return 0;
}
