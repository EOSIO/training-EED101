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
      typedef multi_index<name("hashes2"), hashes> hashes_table;

      [[eosio::on_notify( "eosio.token::transfer")]]
      // name from, name to, asset quantity, string memo
      void sethash( name storer, name recipient, asset stake, string ipfshash ) {
         // make sure that the contract isn't the sender
         if ( storer != get_self() ) { 
            // run simple checks on the transfer
            check( storer != get_self() && recipient == get_self(), "Nice try, you can\'t send to yourself, contract." );
            check( stake.amount > 0, "Waste my contract\'s time, will you? Guards!" );
            check( stake.symbol == staketoken_symbol, "We don\'t accept your rocks here!" );

            // pass in stake balance to store
            update_hash( storer, stake.amount, ipfshash );
         }
      }

      ACTION unstakehash( name storer ){ 
         require_auth( storer );
         hashes_table _hashes( get_self(), get_self().value );

         // check that the hash record actually exists
         auto hash_itr = _hashes.find( storer.value );
         check ( hash_itr != _hashes.end(), "No record of any staking found!" );

         // refund the user's stake (minus a percentage)
         double payout_amount = hash_itr->stakeamount * 0.998; // minus 0.2% fee
         asset payout_asset( payout_amount, staketoken_symbol );
         action payout_action = action(
            permission_level{ get_self(), "active"_n },  //authorization
            "eosio.token"_n,                             //contract code name
            "transfer"_n,                                //action name
            std::make_tuple( get_self(), storer, payout_asset, string( "Thank you for staking!" ) )
            // data: name from, name to, asset quantity, string memo
         );
         payout_action.send();

         // delete the hash record
         _hashes.erase( hash_itr );
      }

   private:
      const symbol staketoken_symbol;a
      void update_hash( name storer, int64_t stake_amount, string ipfshash ) {
         hashes_table _hashes( get_self(), get_self().value );
         
         //search for record
         auto hash_itr = _hashes.find( storer.value );
         //if end state, emplace
         if ( hash_itr == _hashes.end() ) {
            _hashes.emplace( get_self(), [&]( auto& newrecord) {
               newrecord.storer = storer;
               newrecord.ipfshash = ipfshash;
               newrecord.stakeamount = stake_amount;
            });
         } else {
         //if not end state, record exists so modify
            _hashes.modify( hash_itr, get_self(), [&]( auto& record_to_modify ) {
               record_to_modify.ipfshash = ipfshash;
               record_to_modify.stakeamount += stake_amount;
            });
         }
      }
};