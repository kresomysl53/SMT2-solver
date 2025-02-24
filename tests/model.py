from collections import Counter

STATUS_SAT = "SAT"
STATUS_UNSAT = "UNSAT"

class InputError(Exception):
    pass

class ModelError(Exception):
    pass

class Input:
    def __init__(self, num_of_regions, num_of_products, neighbours):
        self.num_of_regions = num_of_regions
        self.num_of_products = num_of_products
        self.neighbours = neighbours

    @staticmethod
    def load(path):
        with open(path) as f:
            lines = f.read().split("\n")
            header = lines[0].split(" ")
            if len(header) != 2:
                raise InputError(f"Invalid header: {header}. Exactly two numbers expected.")
            num_of_regions = int(header[0])
            num_of_products = int(header[1])
            
            if num_of_regions <= 0 or num_of_products <= 0:
                raise InputError(f"Invalid header: {header}. Each number in the header should be positive.")

            neighbours = {i : set() for i in range(num_of_regions)}
            for (i, line) in enumerate(lines[1:]):
                if line not in ["", "\n"]:
                    edge = line.split(" ")
                    if len(edge) != 2:
                        raise InputError(f"Invalid neighbours on line {i+1}: {line}. Exactly two numbers expected.")
                    src = int(edge[0])
                    dst = int(edge[1])
                    if src >= num_of_regions or dst >= num_of_regions:
                        raise InputError(f"Invalid neighbours on line {i+1}: {line}. Both numbers have to be less than {num_of_regions}.")
                    neighbours[src].add(dst)
                    neighbours[dst].add(src)
            return Input(num_of_regions, num_of_products, neighbours)

    def compute_var_index(self, is_primary, region, product):
        return self.num_of_products * region + product + 1 + (1 - bool(is_primary)) * self.num_of_products * self.num_of_regions


class Model:
    def __init__(self, status, literals, input):
        self.status = status
        self.literals = literals
        self.input = input

    def is_sat(self):
        return self.status == STATUS_SAT

    @staticmethod
    def load(path, input):
        """
        The output of the minisat always has the form:
            STATUS
            [MODEL 0]
        """
        with open(path, "r") as f:
            lines = f.read().split("\n")
            status = lines[0]

            if status == STATUS_UNSAT:
                return Model(status, None, input)
            else:
                model = lines[1].split(" ")[0:-1]  # Discard '0'

                if model == [""]:
                    return Model(status, [], input)

                model = list(map(lambda x: int(x), model))
                return Model(status, model, input)

    def __getitem__(self, key):
        is_primary, region, product = key
        var = self.input.compute_var_index(*key)

        if var in self.literals:
            return True
        elif -var in self.literals:
            return False
        else:
            return True  # variable is undefined
            
    def get_region_primary_products(self, region):
        acc = []
        for product in range(self.input.num_of_products):
            acc.append(self[True, region, product])
        return acc        
        
    def get_region_secondary_products(self, region):
        acc = []
        for product in range(self.input.num_of_products):
            acc.append(self[False, region, product])
        return acc
        
    def get_primary_product_regions(self, product):
        acc = []
        for region in range(self.input.num_of_regions):
            acc.append(self[True, region, product])
        return acc

    def get_secondary_product_regions(self, product):
        acc = []
        for region in range(self.input.num_of_regions):
            acc.append(self[False, region, product])
        return acc

    # CONDITION 1, 2
    def check_exactly_one_primary_product(self):
        for region in range(self.input.num_of_regions):
            products = self.get_region_primary_products(region)
            if not sum(products):
                raise ModelError(f"Invalid model. Region {region} has no primary product.")
            if sum(products) >= 2:
                raise ModelError(f"Invalid model. Region {region} has multiple primary products.")    

    # CONDITION 3
    def check_at_mose_one_secondary_product(self):
        for region in range(self.input.num_of_regions):
            products = self.get_region_secondary_products(region)
            if sum(products) >= 2:
                raise ModelError(f"Invalid model. Region {region} has multiple secondary products.")

    # CONDITION 4
    def check_distinct_primary_and_secondary_products(self):
        for region in range(self.input.num_of_regions):
            primary = self.get_region_primary_products(region)
            secondary = self.get_region_secondary_products(region)
            if (True, True) in zip(primary, secondary):
                raise ModelError(f"Invalid model. Region {region} has the same primary and secondary product.")
                
    # CONDITION 5
    def check_distinct_neighbour_primary_products(self):
        for region_1 in range(self.input.num_of_regions):
            for region_2 in self.input.neighbours[region_1]:
                primary_1 = self.get_region_primary_products(region_1)
                primary_2 = self.get_region_primary_products(region_2)
                if (True, True) in zip(primary_1, primary_2):
                    raise ModelError(f"Invalid model. Neighbouring regions {region_1} and {region_2} share the same primary product.")
                    
    # CONDITION 6
    def check_each_product_is_primarily_produced(self):
        for product in range(self.input.num_of_products):
            regions = self.get_primary_product_regions(product)
            if not sum(regions):
                raise ModelError(f"Invalid model. Product {product} is not a primary product in any region.")

    # CONDITION 7
    def check_no_secondary_product_in_main_region(self):
        products = self.get_region_secondary_products(0)
        if sum(products):
            raise ModelError(f"Invalid model. Main region shall not have any secondary product.")
            
    # CONDITION 8
    def check_primary_product_in_main_region_is_nowhere_secondary(self):
        for product in range(self.input.num_of_products):
            regions_2 = self.get_secondary_product_regions(product)
            if self[True, 0, product]:
                regions_2 = self.get_secondary_product_regions(product)
                if len(regions_2) > 1:
                    if not sum(regions_2[1:]):
                        raise ModelError(f"Invalid model. The product which is primary in the main region has to be secondary somewhere.")

    def check(self):
        self.check_exactly_one_primary_product()
        self.check_at_mose_one_secondary_product()
        self.check_distinct_primary_and_secondary_products()
        self.check_distinct_neighbour_primary_products()
        self.check_each_product_is_primarily_produced()
        self.check_no_secondary_product_in_main_region()
        self.check_primary_product_in_main_region_is_nowhere_secondary()

    def print(self):
        if self.status == STATUS_UNSAT:
            return

        minisat_model = " ".join([str(literal) for literal in self.literals])

        human_readable_model = ""
        for literal in self.literals:

            num_of_products = self.input.num_of_products
            num_of_regions = self.input.num_of_regions
            literal_abs = abs(literal)

            negation = True if literal < 0 else False

            product_type = "h"
            if literal_abs > num_of_products * num_of_regions:
                product_type = "v"
                literal_abs -= num_of_products * num_of_regions

            region = int((literal_abs - 1) / num_of_products)
            product = int((literal_abs - 1) % num_of_products)

            human_readable_model += f"{'-' if negation else ''}{product_type}_{region},{product} "

        print("Models and encoding:")
        print(f"{'Problem' : >8} -> {'MiniSat' : >8}")
        # print(" Problém -> MiniSat")
        for (minisat_literal, problem_literal) in zip(minisat_model.split(), human_readable_model.split()):
            print(f"{problem_literal : >8} -> {minisat_literal : >5}")
        print("Status:", self.status)

