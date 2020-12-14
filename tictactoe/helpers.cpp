TABLE game
{

   static constexpr uint16_t board_width = 3;
   static constexpr uint16_t board_height = board_width;
   
   game() : board( board_width * board_height, 0 ){}

   name challenger;
   name host;
   name turn;              // = account name of host/ challenger
   name winner = none;     // = none/ draw/ name of host/ name of challenger

   std::vector<uint8_t> board;

   // Initialize board with empty cells
   void initialize_board()
   {
      board.assign( board_width * board_height, 0 );
   }

   // Reset game
   void reset_game()
   {
      initialize_board();
      turn = host;
      winner = "none"_n;
   }

   auto primary_key() const { return challenger.value; }
   EOSLIB_SERIALIZE( game, (challenger)(host)(turn)(winner)(board) )
};

typedef eosio::multi_index<"games"_n, game> games;



bool is_empty_cell( const uint8_t &cell )
{
    return cell == 0;
}

bool is_valid_movement( const uint16_t &row, const uint16_t &column, const std::vector<uint8_t> &board )
{
    uint32_t movement_location = row * tic_tac_toe::game::board_width + column;
    bool is_valid = movement_location < board.size() && is_empty_cell( board[movement_location] );
    return is_valid;
}

name get_winner( const tic_tac_toe::game &current_game )
{
    auto &board = current_game.board;

    bool is_board_full = true;

    // Use bitwise AND operator to determine the consecutive values of each column, row and diagonal
    // Since 3 == 0b11, 2 == 0b10, 1 = 0b01, 0 = 0b00
    std::vector<uint32_t> consecutive_column( tic_tac_toe::game::board_width, 3 );
    std::vector<uint32_t> consecutive_row( tic_tac_toe::game::board_height, 3 );
    uint32_t consecutive_diagonal_backslash = 3;
    uint32_t consecutive_diagonal_slash = 3;

    for (uint32_t i = 0; i < board.size(); i++)
    {
        is_board_full &= is_empty_cell( board[i] );
        uint16_t row = uint16_t( i / tic_tac_toe::game::board_width );
        uint16_t column = uint16_t( i % tic_tac_toe::game::board_width );

        // Calculate consecutive row and column value
        consecutive_row[column] = consecutive_row[column] & board[i];
        consecutive_column[row] = consecutive_column[row] & board[i];
        // Calculate consecutive diagonal \ value
        if ( row == column )
        {
            consecutive_diagonal_backslash = consecutive_diagonal_backslash & board[i];
        }
        // Calculate consecutive diagonal / value
        if ( row + column == tic_tac_toe::game::board_width - 1 )
        {
            consecutive_diagonal_slash = consecutive_diagonal_slash & board[i];
        }
    }

    // Inspect the value of all consecutive row, column, and diagonal and determine winner
    std::vector<uint32_t> aggregate = {consecutive_diagonal_backslash, consecutive_diagonal_slash};
    aggregate.insert( aggregate.end(), consecutive_column.begin(), consecutive_column.end() );
    aggregate.insert( aggregate.end(), consecutive_row.begin(), consecutive_row.end() );

    for ( auto value : aggregate )
    {
        if ( value == 1 )
        {
            return current_game.host;
        }
        else if ( value == 2 )
        {
            return current_game.challenger;
        }
    }
    // Draw if the board is full, otherwise the winner is not determined yet
    return is_board_full ? tic_tac_toe::draw : tic_tac_toe::none;
}