// SPDX-License-Identifier: MIT
pragma solidity ^0.7.0;

contract hasheseth {
    mapping ( address=>string ) ipfshashes;
    address[] public accts;

    event hashUpdated( string _ipfshash, address _address );

    function setHash( address _address, string memory _ipfshash ) public {
        require( _address == msg.sender );
        ipfshashes[_address] = _ipfshash;
        accts.push( _address );
        emit hashUpdated( _ipfshash, _address );
    }

    function getAcctsLength() public view
    returns( uint256 acctsLength ) {
        return accts.length;
    }

    function getHash( address _address ) view public
    returns ( string memory ipfshash ) {
        return ( ipfshashes[_address] );
    }
}