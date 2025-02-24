#include <stddef.h>
#include "cnf.h"

//
// LOGIN: xholinp00
//

/** Funkce demonstrující vytvoření nové (arbitrárně vybrané) klauzule
* ve tvaru "h_{0,1} || -v_{0,1}" do výrokové formule
* @param formula výroková formule, do níž bude klauzule přidána
*/
void conditions_example(CNF* formula) {
    assert(formula != NULL);

    // vytvoření nové klauzule
    Clause* cl = create_new_clause(formula);
    
    // přidání proměnné h_{0,1} do klauzule
    // proměnná říká, že v regionu 0 je produkován produkt 1 jako hlavní
    // cl - klauzule, do níž přidáváme literál
    // true - značí, že přidaný literál je pozitivní proměnná
    // MAIN_PRODUCT - značí, že pracujeme s hlavním produktem
    // 0 - značí region s indexem 0
    // 1 - značí produkt s indexem 1
    add_literal_to_clause(cl, true, MAIN_PRODUCT, 0, 1);

    // přidání proměnné -v_{0,1} do klauzule
    // proměnná říká, že v regionu 0 není produkován produkt 1 jako vedlejší
    // cl - klauzule, do níž přidáváme literál
    // false - značí, že přidaný literál je negativní proměnná
    // SIDE_PRODUCT - značí, že pracujeme s vedlejším produktem
    // 0 - značí region s indexem 0
    // 1 - značí produkt s indexem 1
    add_literal_to_clause(cl, false, SIDE_PRODUCT, 0, 1);
}

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* je produkován alespoň jeden hlavní produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_regions_min_one_main_product(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);
    
    for (unsigned k = 0; k < num_of_regions; ++k) {
        Clause* cl = create_new_clause(formula);
        for (unsigned p = 0; p < num_of_products; ++p) {
            add_literal_to_clause(cl, true, MAIN_PRODUCT, k, p);
        }
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* je produkován nejvýše jeden hlavní produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_regions_max_one_main_product(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);
    
    for (unsigned k = 0; k < num_of_regions; ++k) {
        for (unsigned p_1 = 0; p_1 < num_of_products; ++p_1) {
            for (unsigned p_2 = 0; p_2 < num_of_products; ++p_2) {
                if (p_1 >= p_2) { continue; }
                Clause* cl = create_new_clause(formula);
                add_literal_to_clause(cl, false, MAIN_PRODUCT, k, p_1);
                add_literal_to_clause(cl, false, MAIN_PRODUCT, k, p_2);
            }
        }
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* je produkován nejvýše jeden vedlejší produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_regions_max_one_side_product(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);

    for (unsigned k = 0; k < num_of_regions; ++k) {
        for (unsigned p_1 = 0; p_1 < num_of_products; ++p_1) {
            for (unsigned p_2 = 0; p_2 < num_of_products; ++p_2) {
                if (p_1 >= p_2) { continue; }
                Clause* cl = create_new_clause(formula);
                add_literal_to_clause(cl, false, SIDE_PRODUCT, k, p_1);     //  ¬v{k,p1} ∨ ¬v{k,p2}
                add_literal_to_clause(cl, false, SIDE_PRODUCT, k, p_2);     //  kde k = index kraje; p1, p2 = index 2 ruznych produktu
            }
        }
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* se hlavní a vedlejší produkt liší
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void main_side_products_different(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);

    for (unsigned k = 0; k < num_of_regions; ++k) {
        for (unsigned p = 0; p < num_of_products; ++p) {
            Clause* cl = create_new_clause(formula);
            add_literal_to_clause(cl, false, SIDE_PRODUCT, k, p);       //  ¬v{k,p} ∨ ¬h{k,p}
            add_literal_to_clause(cl, false, MAIN_PRODUCT, k, p);       //  kde k = index kraje; p = index produktu
        }
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, že 
* sousední regiony nesdílejí hlavní produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
* @param neighbours seznamy sousedů
*/
void neighbour_regions_different_main_products(CNF* formula, unsigned num_of_regions, unsigned num_of_products, const NeighbourLists *neighbours) {
    assert(formula != NULL);
    assert(num_of_regions > 0);
    assert(neighbours != NULL);

    for (unsigned k_1 = 0; k_1 < num_of_regions; ++k_1) {
        for (unsigned k_2 = 0; k_2 < num_of_regions; ++k_2) {
            for (unsigned p = 0; p < num_of_products; ++p) {
                if (k_1 >= k_2) { continue; }
                if (are_neighbours(neighbours, k_1, k_2)){
                    Clause* cl = create_new_clause(formula);
                    add_literal_to_clause(cl, false, MAIN_PRODUCT, k_1, p);       //  ¬h{k_1,p} ∨ ¬h{k_2,p}
                    add_literal_to_clause(cl, false, MAIN_PRODUCT, k_2, p);       //  kde k_1,k_2 = index ruznych sousedicich kraju; p = index produktu
                }
            }
        }
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, že 
* každý produkt je v některém regionu hlavním produktem.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_products_at_least_once_main_products(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);
    
    for (unsigned p = 0; p < num_of_products; ++p) {
        Clause* cl = create_new_clause(formula);
        for (unsigned k = 0; k < num_of_regions; ++k) {
            add_literal_to_clause(cl, true, MAIN_PRODUCT, k, p);
        }
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, 
* že v hlavním regionu (v regionu 0) neexistuje žádný vedlejší produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void no_side_product_in_main_region(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);
    
    for (unsigned p = 0; p < num_of_products; ++p) {
        Clause* cl = create_new_clause(formula);
        add_literal_to_clause(cl, false, SIDE_PRODUCT, 0, p);
    }
}

/** Funkce vytvářející klauzule ošetřující podmínku, 
* že produkt produkovaný v hlavním regionu (v regionu 0) jako hlavní produkt
* je ještě v některém regionu produkován jako vedlejší.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void main_region_main_product_as_side_product_elsewhere(CNF* formula, unsigned num_of_regions, unsigned num_of_products) {
    assert(formula != NULL);
    assert(num_of_regions > 0);

    for (unsigned p = 0; p < num_of_products; ++p) {
        Clause* cl = create_new_clause(formula);
        for (unsigned k = 1; k < num_of_regions; ++k) {
                add_literal_to_clause(cl, false, MAIN_PRODUCT, 0, p);   //  ¬h{0,p} ∨ v{k,p} ∨ v{k,p} ∨ ...
                add_literal_to_clause(cl, true, SIDE_PRODUCT, k, p);    //  kde k zastupuje vsechny ostatni kraje (pokud num_of_regions > 1)
            }
        }
}

/** Bonusová funkce k projektu
* @return vrací bonusovou odpověď
*/
char *bonus(void) {

    // Doplňte správný řetězec
    return "";
}
