import numpy as np

from pyscipopt import Model

import ecole.scip


class CombinatorialAuctionGenerator:
    def __init__(self, parameter_generator=None):
        self.parameter_generator = parameter_generator
        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        param_dict = next(self.parameter_generator)
        model = self.generate_instance(**param_dict)

        return ecole.scip.Model.from_pyscipopt(model)

    def seed(self, seed):
        self.rng.seed(seed)

    def generate_instance(
        self,
        n_items=100,
        n_bids=500,
        min_value=1,
        max_value=100,
        value_deviation=0.5,
        add_item_prob=0.9,
        max_n_sub_bids=5,
        additivity=0.2,
        budget_factor=1.5,
        resale_factor=0.5,
        integers=False,
        warnings=False,
    ):
        """
		Generate a Combinatorial Auction instance with specified characteristics, and writes
		it to a file in the LP format.
		Algorithm described in:
			Kevin Leyton-Brown, Mark Pearson, and Yoav Shoham. (2000).
			Towards a universal test suite for combinatorial auction algorithms.
			Proceedings of ACM Conference on Electronic Commerce (EC-00) 66-76.
		section 4.3., the 'arbitrary' scheme.
		Parameters
		----------
		n_items : int
			The number of items.
		n_bids : int
			The number of bids.
		min_value : int
			The minimum resale value for an item.
		max_value : int
			The maximum resale value for an item.
		value_deviation : int
			The deviation allowed for each bidder's private value of an item, relative from max_value.
		add_item_prob : float in [0, 1]
			The probability of adding a new item to an existing bundle.
		max_n_sub_bids : int
			The maximum number of substitutable bids per bidder (+1 gives the maximum number of bids per bidder).
		additivity : float
			Additivity parameter for bundle prices. Note that additivity < 0 gives sub-additive bids, while additivity > 0 gives super-additive bids.
		budget_factor : float
			The budget factor for each bidder, relative to their initial bid's price.
		resale_factor : float
			The resale factor for each bidder, relative to their initial bid's resale value.
		integers : logical
			Should bid's prices be integral ?
		warnings : logical
			Should warnings be printed ?
		"""

        assert min_value >= 0 and max_value >= min_value
        assert add_item_prob >= 0 and add_item_prob <= 1

        def choose_next_item(bundle_mask, interests, compats, add_item_prob, rng):
            n_items = len(interests)
            prob = (1 - bundle_mask) * interests * compats[bundle_mask, :].mean(axis=0)
            prob /= prob.sum()
            return rng.choice(n_items, p=prob)

        # common item values (resale price)
        values = min_value + (max_value - min_value) * self.rng.rand(n_items)

        # item compatibilities
        compats = np.triu(self.rng.rand(n_items, n_items), k=1)
        compats = compats + compats.transpose()
        compats = compats / compats.sum(1)

        bids = []
        n_dummy_items = 0

        # create bids, one bidder at a time
        while len(bids) < n_bids:

            # bidder item values (buy price) and interests
            private_interests = self.rng.rand(n_items)
            private_values = values + max_value * value_deviation * (2 * private_interests - 1)

            # substitutable bids of this bidder
            bidder_bids = {}

            # generate initial bundle, choose first item according to bidder interests
            prob = private_interests / private_interests.sum()
            item = self.rng.choice(n_items, p=prob)
            bundle_mask = np.full(n_items, 0)
            bundle_mask[item] = 1

            # add additional items, according to bidder interests and item compatibilities
            while self.rng.rand() < add_item_prob:
                # stop when bundle full (no item left)
                if bundle_mask.sum() == n_items:
                    break
                item = choose_next_item(
                    bundle_mask, private_interests, compats, add_item_prob, self.rng
                )
                bundle_mask[item] = 1

            bundle = np.nonzero(bundle_mask)[0]

            # compute bundle price with value additivity
            price = private_values[bundle].sum() + np.power(len(bundle), 1 + additivity)
            if integers:
                price = int(price)

            # drop negativaly priced bundles
            if price < 0:
                if warnings:
                    print("warning: negatively priced bundle avoided")
                continue

            # bid on initial bundle
            bidder_bids[frozenset(bundle)] = price

            # generate candidates substitutable bundles
            sub_candidates = []
            for item in bundle:

                # at least one item must be shared with initial bundle
                bundle_mask = np.full(n_items, 0)
                bundle_mask[item] = 1

                # add additional items, according to bidder interests and item compatibilities
                while bundle_mask.sum() < len(bundle):
                    item = choose_next_item(
                        bundle_mask, private_interests, compats, add_item_prob, self.rng
                    )
                    bundle_mask[item] = 1

                sub_bundle = np.nonzero(bundle_mask)[0]

                # compute bundle price with value additivity
                sub_price = private_values[sub_bundle].sum() + np.power(
                    len(sub_bundle), 1 + additivity
                )
                if integers:
                    sub_price = int(sub_price)

                sub_candidates.append((sub_bundle, sub_price))

            # filter valid candidates, higher priced candidates first
            budget = budget_factor * price
            min_resale_value = resale_factor * values[bundle].sum()
            for bundle, price in [
                sub_candidates[i] for i in np.argsort([-price for bundle, price in sub_candidates])
            ]:

                if len(bidder_bids) >= max_n_sub_bids + 1 or len(bids) + len(bidder_bids) >= n_bids:
                    break

                if price < 0:
                    if warnings:
                        print("warning: negatively priced substitutable bundle avoided")
                    continue

                if price > budget:
                    if warnings:
                        print("warning: over priced substitutable bundle avoided")
                    continue

                if values[bundle].sum() < min_resale_value:
                    if warnings:
                        print("warning: substitutable bundle below min resale value avoided")
                    continue

                if frozenset(bundle) in bidder_bids:
                    if warnings:
                        print("warning: duplicated substitutable bundle avoided")
                    continue

                bidder_bids[frozenset(bundle)] = price

            # add XOR constraint if needed (dummy item)
            if len(bidder_bids) > 2:
                dummy_item = [n_items + n_dummy_items]
                n_dummy_items += 1
            else:
                dummy_item = []

            # place bids
            for bundle, price in bidder_bids.items():
                bids.append((list(bundle) + dummy_item, price))

        # generate SCIP instance from problem
        model = Model()
        model.setMaximize()

        bids_per_item = [[] for item in range(n_items + n_dummy_items)]

        # add variables
        for i, bid in enumerate(bids):
            bundle, price = bid
            model.addVar(name=f"x{i+1}", vtype="B", obj=price)
            for item in bundle:
                bids_per_item[item].append(i)

        # add constraints
        model_vars = model.getVars()
        for item_bids in bids_per_item:
            cons_lhs = 0
            if item_bids:
                for i in item_bids:
                    cons_lhs += model_vars[i]
                model.addCons(cons_lhs <= 1)

        return model
