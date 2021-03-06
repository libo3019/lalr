#ifndef LALR_LEXER_HPP_INCLUDED
#define LALR_LEXER_HPP_INCLUDED

#include <vector>
#include <functional>

namespace lalr
{

class ErrorPolicy;
class LexerAction;
class LexerTransition;
class LexerState;
class LexerStateMachine;

/**
// A lexical analyzer.
*/
template <class Iterator, class Char = typename std::iterator_traits<Iterator>::value_type, class Traits = typename std::char_traits<Char>, class Allocator = typename std::allocator<Char> >
class Lexer
{
    typedef std::function<void (Iterator* begin, Iterator end, std::basic_string<Char, Traits, Allocator>* lexeme, const void** symbol)> LexerActionFunction;

    struct LexerActionHandler
    {
        const LexerAction* action_;
        LexerActionFunction function_;
        LexerActionHandler( const LexerAction* action, LexerActionFunction function );
    };

    const LexerStateMachine* state_machine_; ///< The state machine for this lexer.
    const LexerStateMachine* whitespace_state_machine_; ///< The whitespace state machine for this lexer.
    const void* end_symbol_; ///< The value to return to indicate that the end of the input has been reached.
    ErrorPolicy* error_policy_; ///< The error policy this lexer uses to report errors and debug information.
    std::vector<LexerActionHandler> action_handlers_; ///< The action handlers for this Lexer.
    Iterator position_; ///< The current position of this Lexer in its input sequence.
    Iterator end_; ///< One past the last position of the input sequence for this Lexer.
    std::basic_string<Char, Traits, Allocator> lexeme_; ///< The most recently matched lexeme.
    const void* symbol_; ///< The most recently matched symbol or null if no symbol has been matched.
    bool full_; ///< True when this Lexer scanned all of its input otherwise false.

    public:
        Lexer( const LexerStateMachine* state_machine, const LexerStateMachine* whitespace_state_machine = nullptr, const void* end_symbol = nullptr, ErrorPolicy* error_policy = nullptr );
        void set_action_handler( const char* identifier, LexerActionFunction function );
        const std::basic_string<Char, Traits, Allocator>& lexeme() const;
        const void* symbol() const;
        const Iterator& position() const;
        bool full() const;
        void reset( Iterator start, Iterator finish );
        void advance();
        
    private:
        void skip();
        const void* run();
        void error();
        void fire_error( int line, int error, const char* format, ... ) const;
        const LexerTransition* find_transition_by_character( const LexerState* state, int character ) const;
};

}

#include "Lexer.ipp"

#endif