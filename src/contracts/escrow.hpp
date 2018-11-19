#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/print.hpp>
#include <cmath>

#define EOS_SYMBOL symbol("EOS", 4)

using namespace std;
using namespace eosio;
using eosio::action;

CONTRACT escrow : public eosio::contract
{
  public:
    using contract::contract;

    escrow(eosio::name self, eosio::name code, eosio::datastream<const char *> ds) : eosio::contract(self, code, ds), _deals(self, code.value), _deposits(self, code.value) {}

    const float FEE = 0.1; // 1% charged as fees for arbiter and escrow account.

    bool isNumeric(const std::string &input)
    {
        return std::all_of(input.begin(), input.end(), ::isdigit);
    }

    ACTION initiatedeal(name sponsor, name contractor,
                        name arbiter, string memo, asset amount,
                        uint64_t no_of_sponsors, uint64_t threshold,
                        uint64_t timeout_of_funding, uint64_t timeout_of_acceptance,
                        vector<uint64_t> payout_schedule, uint64_t payout_term);

    ACTION addsponsors(name sponsor, uint64_t & dealId, std::vector<name> coSponsors);

    void checkdeposit(
        eosio::name from,
        eosio::name to,
        eosio::asset quantity,
        std::string memo);

    int64_t get_payable(const uint64_t &dealId)
    {
        auto d = _deals.find(dealId);
        if (d != _deals.end())
        {

            int64_t pres = (int64_t)d->amount.symbol.precision();

            int64_t div = std::pow(10, pres);

            int64_t payable = (d->amount.amount) / div;
            return payable;
        }
    }

  private:
    TABLE deal
    {
        uint64_t id;
        name sponsor;
        name contractor;
        name arbiter;
        std::vector<name> coSponsors;
        string memo;
        asset amount;
        uint64_t no_of_sponsors;
        uint64_t threshold;
        uint64_t timeout_of_funding;
        uint64_t timeout_of_acceptance;
        vector<uint64_t> payout_schedule;
        uint64_t payout_term;

        uint64_t primary_key() const { return id; }
        uint64_t by_sponsor() const { return sponsor.value; }
        uint64_t by_contractor() const { return contractor.value; }
    };
    typedef eosio::multi_index<"deals"_n, deal,
                               eosio::indexed_by<"sponsor"_n, eosio::const_mem_fun<deal, uint64_t, &deal::by_sponsor>>,
                               eosio::indexed_by<"contractor"_n, eosio::const_mem_fun<deal, uint64_t, &deal::by_contractor>>>
        deals_table;

    TABLE deposit
    {
        uint64_t id;
        uint64_t dealId;
        name sponsor;
        asset amount;

        uint64_t primary_key() const { return id; }
        uint64_t by_dealId() const { return dealId; }
        uint64_t by_sponsor() const { return sponsor.value; }
    };

    typedef eosio::multi_index<"deposits"_n, deposit,
                               eosio::indexed_by<"dealid"_n, eosio::const_mem_fun<deposit, uint64_t, &deposit::by_dealId>>,
                               eosio::indexed_by<"sponsor"_n, eosio::const_mem_fun<deposit, uint64_t, &deposit::by_sponsor>>>
        deposits_table;

    deals_table _deals;
    deposits_table _deposits;
};
