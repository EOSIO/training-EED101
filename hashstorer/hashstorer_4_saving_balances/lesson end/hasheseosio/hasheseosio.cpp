#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
using namespace eosio;
using namespace std;

CONTRACT hasheseosio : public contract {
   public:

      using contract::contract;

      hasheseosio(
         name receiver,
         name code,
         datastream<const char *> ds
      ):contract( receiver, code, ds ),
         staketoken_symbol( "SYS", 4 )
      {}

      TABLE hashes {
         name storer;
         string ipfshash;
         int64_t stakeamount;
         auto primary_key() const { return storer.value; }
      };
      typedef multi_index<name("hashes"), hashes> hashes_table;

      [[eosio::on_notify( "eosio.token::transfer ")]]
      // name from, name to, asset quantity, string memo
      void sethash( name storer, name recipient, asset stake, string ipfshash ) {
         // run simple checks on the transfer
         check( storer != get_self() && recipient == get_self(), "Nice try, you can't send to yourself, contract." );
         check( stake.amount > 0, "Waste my contract's time, will you? Guards!" );
         check( stake.symbol == staketoken_symbol, "We don't accept your seashells here!");

         // pass in stake balance to store
         update_hash( storer, stake.amount, ipfshash );
      }

   private:
      const symbol staketoken_symbol;
      void update_hash( name storer, int64_t stake_amount, string ipfshash ) {
         hashes_table _hashes( get_self(), get_self().value );
         
         //search for record
         auto hash_itr = _hashes.find( storer.value );
         //if end state, emplace
         if ( hash_itr == _hashes.end() ) {
            _hashes.emplace( storer, [&]( auto& newrecord) {
               newrecord.storer = storer;
               newrecord.ipfshash = ipfshash;
               newrecord.stakeamount = stake_amount;
            });
         } else {
         //if not end state, record exists so modify
            _hashes.modify( hash_itr, storer, [&]( auto& record_to_modify ) {
               record_to_modify.ipfshash = ipfshash;
               record_to_modify.stakeamount += stake_amount;
            });
         }
      }
};