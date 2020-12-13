#include <eosio/eosio.hpp>
using namespace eosio;
using namespace std;

CONTRACT hasheseosio : public contract {
   public:

      using contract::contract;

      TABLE hashes {
         name storer;
         string ipfshash;
         auto primary_key() const { return storer.value; }
      };
      typedef multi_index<name("hashes"), hashes> hashes_table;
   
      ACTION sethash( name storer, string ipfshash ) {
         require_auth( storer );
         hashes_table _hashes( get_self(), get_self().value );
         
         //search for record
         auto hash_itr = _hashes.find( storer.value );
         //if end state, emplace
         if ( hash_itr == _hashes.end() ) {
            _hashes.emplace( storer, [&]( auto& newrecord) {
               newrecord.storer = storer;
               newrecord.ipfshash = ipfshash;
            });
         } else {
         //if not end state, record exists so modify
            _hashes.modify( hash_itr, storer, [&]( auto& record_to_modify ) {
               record_to_modify.ipfshash = ipfshash;
            });
         }
      }

   private:
};