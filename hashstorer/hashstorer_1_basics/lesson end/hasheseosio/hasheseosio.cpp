#include <eosio/eosio.hpp>
using namespace eosio;
using namespace std;

CONTRACT hasheseosio : public contract {
   public:

      using contract::contract;
   
      // set up hashes table struct and type here

      ACTION sethash( name storer, string ipfshash ) {
         require_auth( storer );
         
         // instantiate table, find record, modify if found, emplace if not found
      }

   private:
};