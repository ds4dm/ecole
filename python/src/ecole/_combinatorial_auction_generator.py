import numpy as np
import logging

import ecole.scip


class CombinatorialAuctionGenerator:
    def __init__(
        self,
        n_items: int = 100,
        n_bids: int = 500,
        min_value: int = 1,
        max_value: int = 100,
        value_deviation: float = 0.5,
        add_item_prob: float = 0.9,
        max_n_sub_bids: int = 5,
        additivity: float = 0.2,
        budget_factor: float = 1.5,
        resale_factor: float = 0.5,
        integers: float = False,
    ):
        """Constructor for the CombinatorialAuctionGenerator generator.

        The parameters passed in this constructor will be used when a user calls next() or iterates
        over the object.

        Parameters
        ----------
        n_items:
            The number of items.
        n_bids:
            The number of bids.
        min_value:
            The minimum resale value for an item.
        max_value:
            The maximum resale value for an item.
        value_deviation:
            The deviation allowed for each bidder's private value of an item, relative from max_value.
        add_item_prob:
            The probability of adding a new item to an existing bundle.
            This parameters must be in the range [0,1].
        max_n_sub_bids:
            The maximum number of substitutable bids per bidder (+1 gives the maximum number of bids per bidder).
        additivity:
            Additivity parameter for bundle prices. Note that additivity < 0 gives sub-additive bids, while additivity > 0 gives super-additive bids.
        budget_factor:
            The budget factor for each bidder, relative to their initial bid's price.
        resale_factor:
            The resale factor for each bidder, relative to their initial bid's resale value.
        integers:
            Determines if the bid prices should be integral.

        """
        self.n_items = n_items
        self.n_bids = n_bids
        self.min_value = min_value
        self.max_value = max_value
        self.value_deviation = value_deviation
        self.add_item_prob = add_item_prob
        self.max_n_sub_bids = max_n_sub_bids
        self.additivity = additivity
        self.budget_factor = budget_factor
        self.resale_factor = resale_factor
        self.integers = integers

        self.logger = logging.getLogger(self.__class__.__name__)

        self.rng = np.random.RandomState()

    def __iter__(self):
        return self

    def __next__(self):
        """Gets the next instances of a combinatorial auction problem.

        This method calls generate_instance() with the parameters passed in
        the constructor and returns the ecole.scip.Model.

        Returns
        -------
        model:
            an ecole model of a combinatorial auction instance.

        """
        return self.generate_instance(
            self.n_items,
            self.n_bids,
            self.min_value,
            self.max_value,
            self.value_deviation,
            self.add_item_prob,
            self.max_n_sub_bids,
            self.additivity,
            self.budget_factor,
            self.resale_factor,
            self.integers,
            self.logger,
            self.rng,
        )

    def seed(self, seed: int):
        """Seeds CombinatorialAuctionGenerator.

        This method sets the random seed of the CombinatorialAuctionGenerator.

        Parameters
        ----------
        seed:
            The seed in which to set the random number generator with.

        """
        self.rng.seed(seed)

    @staticmethod
    def generate_instance(
        n_items: int,
        n_bids: int,
        min_value: int,
        max_value: int,
        value_deviation: float,
        add_item_prob: float,
        max_n_sub_bids: int,
        additivity: float,
        budget_factor: float,
        resale_factor: float,
        integers: bool,
        logger: logging.Logger,
        rng: np.random.RandomState,
    ):
        """Generate an instance of a combinatorial auction problem.

        This method generates an instance of a combinatorial auction problem based on the
        specified parameters and returns it as an ecole model.

        Algorithm described in
        Kevin Leyton-Brown, Mark Pearson, and Yoav Shoham. (2000).
        Towards a universal test suite for combinatorial auction algorithms.
        Proceedings of ACM Conference on Electronic Commerce (EC-00) 66-76.
        section 4.3., the 'arbitrary' scheme.

        Parameters
        ----------
        n_items:
            The number of items.
        n_bids:
            The number of bids.
        min_value:
            The minimum resale value for an item.
        max_value:
            The maximum resale value for an item.
        value_deviation:
            The deviation allowed for each bidder's private value of an item, relative from max_value.
        add_item_prob:
            The probability of adding a new item to an existing bundle.
            This parameters must be in the range [0,1].
        max_n_sub_bids:
            The maximum number of substitutable bids per bidder (+1 gives the maximum number of bids per bidder).
        additivity:
            Additivity parameter for bundle prices. Note that additivity < 0 gives sub-additive bids, while additivity
            > 0 gives super-additive bids.
        budget_factor:
            The budget factor for each bidder, relative to their initial bid's price.
        resale_factor:
            The resale factor for each bidder, relative to their initial bid's resale value.
        integers:
            Determines if the bid prices should be integral.
        rng:
            A random state used to sample random numbers.

        Returns
        -------
        model:
            An ecole model of a combinatorial auction instance.

        """
        assert min_value >= 0 and max_value >= min_value
        assert add_item_prob >= 0 and add_item_prob <= 1

        def choose_next_item(bundle_mask, interests, compats, add_item_prob, rng):
            n_items = len(interests)
            prob = (1 - bundle_mask) * interests * compats[bundle_mask, :].mean(axis=0)
            prob /= prob.sum()
            return rng.choice(n_items, p=prob)

        # common item values (resale price)
        values = min_value + (max_value - min_value) * rng.rand(n_items)

        # item compatibilities
        compats = np.triu(rng.rand(n_items, n_items), k=1)
        compats = compats + compats.transpose()
        compats = compats / compats.sum(1)

        bids = []
        n_dummy_items = 0

        # create bids, one bidder at a time
        while len(bids) < n_bids:

            # bidder item values (buy price) and interests
            private_interests = rng.rand(n_items)
            private_values = values + max_value * value_deviation * (2 * private_interests - 1)

            # substitutable bids of this bidder
            bidder_bids = {}

            # generate initial bundle, choose first item according to bidder interests
            prob = private_interests / private_interests.sum()
            item = rng.choice(n_items, p=prob)
            bundle_mask = np.full(n_items, 0)
            bundle_mask[item] = 1

            # add additional items, according to bidder interests and item compatibilities
            while rng.rand() < add_item_prob:
                # stop when bundle full (no item left)
                if bundle_mask.sum() == n_items:
                    break
                item = choose_next_item(bundle_mask, private_interests, compats, add_item_prob, rng)
                bundle_mask[item] = 1

            bundle = np.nonzero(bundle_mask)[0]

            # compute bundle price with value additivity
            price = private_values[bundle].sum() + np.power(len(bundle), 1 + additivity)
            if integers:
                price = int(price)

            # drop negativaly priced bundles
            if price < 0:
                logger.debug("Negatively priced bundle avoided")
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
                        bundle_mask, private_interests, compats, add_item_prob, rng
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
                    logger.debug("Negatively priced substitutable bundle avoided")
                    continue

                if price > budget:
                    logger.debug("Over priced substitutable bundle avoided")
                    continue

                if values[bundle].sum() < min_resale_value:
                    logger.debug("Substitutable bundle below min resale value avoided")
                    continue

                if frozenset(bundle) in bidder_bids:
                    logger.debug("Duplicated substitutable bundle avoided")
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

        model = ecole.scip.Model.prob_basic()
        pyscipopt_model = model.as_pyscipopt()
        pyscipopt_model.setMaximize()

        bids_per_item = [[] for item in range(n_items + n_dummy_items)]

        # add variables
        for i, bid in enumerate(bids):
            bundle, price = bid
            pyscipopt_model.addVar(name=f"x{i+1}", vtype="B", obj=price)
            for item in bundle:
                bids_per_item[item].append(i)

        # add constraints
        pyscipopt_model_vars = pyscipopt_model.getVars()
        for item_bids in bids_per_item:
            cons_lhs = 0
            if item_bids:
                for i in item_bids:
                    cons_lhs += pyscipopt_model_vars[i]
                pyscipopt_model.addCons(cons_lhs <= 1)

        return model
