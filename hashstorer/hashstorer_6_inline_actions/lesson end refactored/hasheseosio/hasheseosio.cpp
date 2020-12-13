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
      void sethash( name storer, name recipient, asset stake, string ipfshash ) {
         if ( check_transfer( storer, recipient, stake ) ) { 
            update_hash( storer, stake.amount, ipfshash );
         }
      }

      ACTION unstakehash( name storer ){ 
         require_auth( storer );
         unstake_hash( storer );
      }


   private:

      const symbol staketoken_symbol;

      bool check_transfer( name storer, name recipient, asset stake ) {
         if ( storer != get_self() ) {
            check( recipient == get_self(), "Nice try, you can\'t send to yourself, contract." );
            check( stake.amount > 0, "Waste my contract\'s time, will you? Guards!" );
            check( stake.symbol == staketoken_symbol, "We don\'t accept your rocks here!" );
            return true;
         }
         return false;
      }

      void unstake_hash( name storer ) {
         hashes_table _hashes( get_self(), get_self().value );

         auto hash_itr = _hashes.find( storer.value );
         check( hash_itr != _hashes.end(), "No record of any staking found!" );
         refund_stake( storer, hash_itr->stakeamount );
         _hashes.erase( hash_itr );
      }

      void refund_stake( name recipient, int64_t stake_amount ) {
         // refund the user's stake (minus a percentage)
         double refund_amount = stake_amount * 0.998; // minus 0.2% fee
         asset refund( refund_amount, staketoken_symbol );
         form_refund_action( recipient, refund).send();
      }

      action form_refund_action( name recipient, asset refund ) {
         return action(
            permission_level{ get_self(), "active"_n },  //authorization
            "eosio.token"_n,                             //contract code name
            "transfer"_n,                                //action name
            std::make_tuple( get_self(), recipient, refund, string( "Thank you for staking!" ) ) //data
         );
      }

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