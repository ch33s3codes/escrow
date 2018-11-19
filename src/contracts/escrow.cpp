#include "escrow.hpp"

ACTION escrow::initiatedeal(name sponsor, name contractor,
                            name arbiter, string memo, asset amount,
                            uint64_t no_of_sponsors, uint64_t threshold,
                            uint64_t timeout_of_funding, uint64_t timeout_of_acceptance,
                            vector<uint64_t> payout_schedule, uint64_t payout_term)
{
    eosio_assert(is_account(contractor), "enter a valid EOS account");
    eosio_assert(is_account(arbiter), "arbitrators must be a valid EOS account holders");

    require_auth(sponsor);
    int64_t payable = escrow::get_payable(1);
    asset p = asset(150);
    print(p);
    _deals.emplace(get_self(), [&](auto &d) {
        d.id = _deals.available_primary_key();
        d.sponsor = sponsor;
        d.contractor = contractor;
        d.arbiter = arbiter;
        d.memo = memo;
        d.amount = amount;
        d.no_of_sponsors = no_of_sponsors;
        d.threshold = threshold;
        d.timeout_of_funding = timeout_of_funding;
        d.timeout_of_acceptance = timeout_of_acceptance;
        d.payout_schedule = payout_schedule;
        d.payout_term = payout_term;
    });
}

ACTION escrow::addsponsors(name sponsor, uint64_t &dealId, std::vector<name> coSponsors)
{
    require_auth(sponsor);
    eosio_assert(coSponsors.size() > 0, "this action requires at least one cosponsor");
    escrow::get_payable(1);

    auto i = _deals.find(dealId);
    if (i != _deals.end())
    {
        _deals.modify(i, sponsor, [&](auto &d) {
            d.coSponsors = coSponsors;
        });
    }
}

ACTION escrow::checkdeposit(name from, name to, asset quantity, string memo)
{
    if (from == _self)
    {
        // Handle refunds?
        return;
    }

    eosio_assert(to == _self, "contract is not involved in this transfer"); // TODO - Handle refunds as well
    eosio_assert(quantity.symbol.is_valid(), "invalid quantity");
    eosio_assert(quantity.amount > 0, "only positive quantity allowed");
    eosio_assert(quantity.symbol == EOS_SYMBOL, "only EOS tokens allowed"); // TODO - Handle other tokens as well. Dont Lock to EOS alone.

    eosio_assert(isNumeric(memo), "the memo needs to represent the claim ID");

    escrow::get_payable(1);

    uint64_t dealId = std::stoi(memo);
    if (from != _self)
    {
        _deposits.emplace(get_self(), [&](auto &dep) {
            dep.id = _deposits.available_primary_key();
            dep.dealId = dealId;
            dep.sponsor = from;
            dep.amount = quantity;
        });
    }
}

extern "C" void apply(uint64_t receiver, uint64_t code, uint64_t action)
{
    if (code == "eosio.token"_n.value && action == "transfer"_n.value)
    {
        eosio::execute_action(
            eosio::name(receiver), eosio::name(code), &escrow::checkdeposit);
    }
    else if (code == receiver)
    {
        switch (action)
        {
            EOSIO_DISPATCH_HELPER(escrow, (initiatedeal)(addsponsors))
        }
    }
}