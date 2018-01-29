#ifndef SWEET_LEXER_LEXERGENERATOR_HPP_INCLUDED
#define SWEET_LEXER_LEXERGENERATOR_HPP_INCLUDED

#include "LexerToken.hpp"
#include "shared_ptr_less.hpp"
#include <memory>
#include <vector>
#include <set>

namespace sweet
{

namespace error
{

class Error;

}

namespace lalr
{

class LexerState;
class LexerAction;
class LexerErrorPolicy;
class RegexSyntaxTree;

/**
// @internal 
//
// Generate a lexical analyzer.
*/
class LexerGenerator
{
    LexerErrorPolicy* event_sink_; ///< The event sink to report errors and debug information to or null to ignore errors and debug information.
    std::vector<std::shared_ptr<LexerAction>> actions_; ///< The lexical analyzer actions.
    std::set<std::shared_ptr<LexerState>, shared_ptr_less<LexerState>> states_; ///< The states generated for the lexical analyzer.
    std::set<std::shared_ptr<LexerState>, shared_ptr_less<LexerState>> whitespace_states_; ///< The states generated for the whitespace lexical analyzer.
    const LexerState* start_state_; ///< The starting state for the lexical analyzer.
    const LexerState* whitespace_start_state_; ///< The starting state for the whitespace lexical analyzer.
    std::vector<std::pair<int, bool>> ranges_; ///< Ranges generated for the current transition while generating.

    public:
        LexerGenerator( const LexerToken& token, LexerErrorPolicy* event_sink );
        LexerGenerator( const std::vector<LexerToken>& tokens, const std::vector<LexerToken>& whitespace_tokens, LexerErrorPolicy* event_sink );

        std::vector<std::shared_ptr<LexerAction> >& actions();
        std::set<std::shared_ptr<LexerState>, shared_ptr_less<LexerState>>& states();
        std::set<std::shared_ptr<LexerState>, shared_ptr_less<LexerState>>& whitespace_states();
        const LexerState* start_state() const;
        const LexerState* whitespace_start_state() const;
        const LexerAction* add_lexer_action( const std::string& identifier );

        void fire_error( int line, int error, const char* format, ... ) const;
        void fire_printf( const char* format, ... ) const;

    private:
        std::shared_ptr<LexerState> goto_( const LexerState* state, int begin, int end );
        void generate_states( const RegexSyntaxTree& syntax_tree, std::set<std::shared_ptr<LexerState>, shared_ptr_less<LexerState>>* states, const LexerState** start_state );
        void generate_indices_for_states();
        void generate_symbol_for_state( LexerState* state ) const;

        void clear();
        void insert( int begin, int end );
};

}

}

#endif