#ifndef __CNF_H
#define __CNF_H

#include <stdbool.h>
#include <assert.h>

#define MAIN_PRODUCT true
#define SIDE_PRODUCT false

typedef struct Literal Literal;

typedef struct Clause Clause;

typedef struct CNF CNF;

typedef struct NeighbourList NeighbourList;

typedef struct NeighbourLists NeighbourLists;

/** Funkce vytvoří novou klauzuli
* @param formula výroková formule
* @return vytvořená klauzule
*/
Clause* create_new_clause(CNF *formula);

/** Funkce přidá literál do klauzule. Literál je pozitivní nebo negativní
* výroková proměnná.
* @param clause klauzule
* @param is_positive příznak udávající, zda je proměnná pozitivní
* @param is_main_product příznak udávající, zda proměnná odpovídá hlavnímu produktu
* @param region index regionu
* @param product index produktu
*/
void add_literal_to_clause(Clause *clause, bool is_positive, bool is_main_product, unsigned region, unsigned product);

/** Funkce demonstrující vytvoření nové (arbitrárně vybrané) klauzule
* ve tvaru "h_{0,1} || -v_{0,1}" do výrokové formule
* @param formula výroková formule, do níž bude klauzule přidána
*/
void conditions_example(CNF* formula);

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* je produkován alespoň jeden hlavní produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_regions_min_one_main_product(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* je produkován nejvýše jeden hlavní produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_regions_max_one_main_product(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* je produkován nejvýše jeden vedlejší produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_regions_max_one_side_product(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Funkce vytvářející klauzule ošetřující podmínku, že v každém regionu
* se hlavní a vedlejší produkt liší
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void main_side_products_different(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Funkce vytvářející klauzule ošetřující podmínku, že 
* sousední regiony nesdílejí hlavní produkt.
* Tip: Využijte implementovanou funkci are_neighbours.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
* @param neighbours seznamy sousedů
*/
void neighbour_regions_different_main_products(CNF* formula, unsigned num_of_regions, unsigned num_of_products, const NeighbourLists *neighbours);

/** Funkce vytvářející klauzule ošetřující podmínku, že 
* každý produkt je v některém regionu hlavním produktem.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void all_products_at_least_once_main_products(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Funkce vytvářející klauzule ošetřující podmínku, 
* že v hlavním regionu (v regionu 0) neexistuje žádný vedlejší produkt.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void no_side_product_in_main_region(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Funkce vytvářející klauzule ošetřující podmínku, 
* že produkt produkovaný v hlavním regionu (v regionu 0) jako hlavní produkt
* je ještě v některém regionu produkován jako vedlejší.
* @param formula výroková formule, do níž bude klauzule přidána
* @param num_of_regions počet regionů
* @param num_of_products počet produktů
*/
void main_region_main_product_as_side_product_elsewhere(CNF* formula, unsigned num_of_regions, unsigned num_of_products);

/** Predikát rozhodující, zda dané dva indexy odpovídají sousedícím regionům
* @param lists seznam sousedů
* @param fst první region
* @param snd druhý region
* @return true, pokud fst sousedí se snd
*/
bool are_neighbours(const NeighbourLists *lists, unsigned fst, unsigned snd);

/** Bonusová funkce k projektu
* @return vrací bonusovou odpověď
*/
char *bonus(void);
 
#endif
