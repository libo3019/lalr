// 
// ParserGrammar.cpp
// Copyright (c) Charles Baker. All rights reserved.
//

#include "ParserGrammar.hpp"
#include "ParserGenerator.hpp"
#include "ParserProduction.hpp"
#include "ParserAction.hpp"
#include "GrammarSymbol.hpp"
#include "ErrorCode.hpp"
#include "assert.hpp"
#include <memory>
#include <stdio.h>

using std::set;
using std::vector;
using std::unique_ptr;
using namespace sweet;
using namespace sweet::lalr;
using namespace sweet::lalr;

/**
// Constructor.
//
// @param actions_reserve
//  The number of actions to reserve space for in this ParserGrammar.
//
// @param productions_reserve
//  The number of productions to reserve space for in this ParserGrammar.
//
// @param symbols_reserve
//  The number of symbols to reserve space for in this ParserGrammar.
*/
ParserGrammar::ParserGrammar( size_t actions_reserve, size_t productions_reserve, size_t symbols_reserve )
: identifier_(),
  actions_(),
  productions_(),
  symbols_(),
  start_symbol_( NULL ),
  end_symbol_( NULL ),
  error_symbol_( NULL )
{
    actions_.reserve( actions_reserve );
    productions_.reserve( productions_reserve );
    symbols_.reserve( symbols_reserve );
    start_symbol_ = add_non_terminal( ".start", 0 );
    end_symbol_ = add_symbol( SYMBOL_END, ".end", 0 );
    error_symbol_ = add_terminal( ".error", 0 );
}

ParserGrammar::~ParserGrammar()
{
    for ( auto i = actions_.begin(); i != actions_.end(); ++i )
    {
        ParserAction* action = i->get();
        SWEET_ASSERT( action );
        action->destroy();
    }
}

/**
// Get the identifier of this ParserGrammar.
//
// @return
//  The identifier.
*/
std::string& ParserGrammar::identifier()
{
    return identifier_;
}

/**
// Get the actions in this ParserGrammar.
//
// @return
//  The actions.
*/
std::vector<std::unique_ptr<ParserAction> >& ParserGrammar::actions()
{
    return actions_;
}

/**
// Get the productions in this ParserGrammar.
//
// @return
//  The productions.
*/
std::vector<std::unique_ptr<ParserProduction> >& ParserGrammar::productions()
{
    return productions_;
}

/**
// Get the symbols in this ParserGrammar.
//
// @return
//  The symbols.
*/
std::vector<std::unique_ptr<ParserSymbol> >& ParserGrammar::symbols()
{
    return symbols_;
}

/**
// Get the start symbol in this ParserGrammar.
//
// @return
//  The start symbol.
*/
ParserSymbol* ParserGrammar::start_symbol() const
{
    return start_symbol_;
}

/**
// Get the end symbol in this ParserGrammar.
//
// @return
//  The end symbol.
*/
ParserSymbol* ParserGrammar::end_symbol() const
{
    return end_symbol_;
}

/**
// Get the error symbol in this ParserGrammar.
//
// @return
//  The error symbol.
*/
ParserSymbol* ParserGrammar::error_symbol() const
{
    return error_symbol_;
}

ParserSymbol* ParserGrammar::symbol( const GrammarSymbol* grammar_symbol )
{
    SWEET_ASSERT( grammar_symbol );
    SymbolType type = grammar_symbol->lexeme_type() == LEXEME_NULL ? SYMBOL_NON_TERMINAL : SYMBOL_TERMINAL;
    ParserSymbol* symbol = ParserGrammar::symbol( type, grammar_symbol->lexeme(), 0 );
    symbol->set_associativity( grammar_symbol->associativity() );
    symbol->set_precedence( grammar_symbol->precedence() );
    return symbol;
}

ParserSymbol* ParserGrammar::symbol( SymbolType type, const std::string& identifier, int line )
{
    auto i = symbols_.begin();
    while ( i != symbols_.end() && (*i)->get_lexeme() != identifier )
    {
        ++i;
    }
    if ( i == symbols_.end() )
    {
        return add_symbol( type, identifier, line );
    }
    SWEET_ASSERT( i != symbols_.end() );
    return i->get();
}

ParserSymbol* ParserGrammar::terminal( const std::string& identifier, int line )
{
    return symbol( SYMBOL_TERMINAL, identifier, line );
}

ParserSymbol* ParserGrammar::non_terminal( const std::string& identifier, int line )
{
    return symbol( SYMBOL_NON_TERMINAL, identifier, line );
}

ParserAction* ParserGrammar::action( const std::string& identifier )
{
    auto i = actions_.begin();
    while ( i != actions_.end() && (*i)->identifier != identifier )
    {
        ++i;
    }
    if ( i == actions_.end() )
    {
        return add_action( identifier );
    }
    SWEET_ASSERT( i != actions_.end() );
    return i->get();
}

/**
// Add a symbol to this ParserGrammar.
//
// @param type
//  The type of symbol to add.
//
// @param identifier
//  The identifier to use for the symbol (for debugging purposes only).
//
// @param line
//  The line that the symbol first appears on (for debugging purposes only).
//
// @return
//  The symbol.
*/
ParserSymbol* ParserGrammar::add_symbol( SymbolType type, const std::string& identifier, int line )
{
    unique_ptr<ParserSymbol> symbol( new ParserSymbol(type, identifier, line) );
    symbols_.push_back( move(symbol) );
    return symbols_.back().get();
}

/**
// Add a terminal symbol to this ParserGrammar.
//
// @param identifier
//  The identifier to use for the symbol (for debugging purposes only).
//
// @param line
//  The line that the symbol first appears on (for debugging purposes only).
//
// @return
//  The terminal symbol.
*/
ParserSymbol* ParserGrammar::add_terminal( const std::string& identifier, int line )
{
    return add_symbol( SYMBOL_TERMINAL, identifier, line );
}

/**
// Add a non terminal symbol to this ParserGrammar.
//
// @param identifier
//  The identifier to use for the symbol (for debugging purposes only).
//
// @param line
//  The line that the symbol first appears on (for debugging purposes only).
//
// @return
//  The terminal symbol.
*/
ParserSymbol* ParserGrammar::add_non_terminal( const std::string& identifier, int line )
{
    return add_symbol( SYMBOL_NON_TERMINAL, identifier, line );
}

/**
// Add an action to this ParserGrammar.
//
// @param identifier
//  The identifier of the action to add.
//
// @return
//  The action.
*/
ParserAction* ParserGrammar::add_action( const std::string& identifier )
{
    SWEET_ASSERT( !identifier.empty() );    
    ParserAction* action = nullptr;
    if ( !identifier.empty() )
    {
        std::vector<std::unique_ptr<ParserAction> >::const_iterator i = actions_.begin();
        while ( i != actions_.end() && (*i)->identifier != identifier )
        {
            ++i;
        }        
        if ( i != actions_.end() )
        {
            action = i->get();
        }
        else
        {
            std::unique_ptr<ParserAction> new_action( new ParserAction(int(actions_.size()), identifier.c_str()) );
            actions_.push_back( move(new_action) );
            action = actions_.back().get();
        }
    }
    return action;
}

/**
// Set the identifier for this ParserGrammar (optional).
//
// @param identifier
//  The identifier for this ParserGrammar.
*/
void ParserGrammar::identifier( const std::string& identifier )
{
    identifier_ = identifier;
}

/**
// Start a production in this ParserGrammar.
//
// @param symbol
//  The symbol on the left hand side of the production (assumed not null).
//
// @param line
//  The line that the production starts on.
*/
void ParserGrammar::begin_production( ParserSymbol* symbol, int line )
{
    if ( productions_.empty() )
    {
        SWEET_ASSERT( start_symbol_ );
        SWEET_ASSERT( end_symbol_ );

        unique_ptr<ParserProduction> production( new ParserProduction(int(productions_.size()), start_symbol_, 0, NULL) );
        start_symbol_->append_production( production.get() );
        productions_.push_back( move(production) );
        ParserGrammar::symbol( symbol );
    }

    SWEET_ASSERT( symbol );
    unique_ptr<ParserProduction> production( new ParserProduction(int(productions_.size()), symbol, line, NULL) );
    symbol->append_production( production.get() );
    productions_.push_back( move(production) );
}

/**
// End a production.
//
// If there is more than one node then this end production has been matched as
// part of a top level '|' expression and the right hand node is really the 
// first symbol in another production for the same symbol as the current 
// production.  It will be reduced later when another top level '|' expression
// or the ';' at the end of the production is matched.
*/
void ParserGrammar::end_production()
{
}

/**
// Append a symbol node to the current production's right hand side.
//
// @param symbol
//  The symbol to append on the right hand side (assumed not null).
*/
void ParserGrammar::symbol( ParserSymbol* symbol )
{
    SWEET_ASSERT( symbol );
    SWEET_ASSERT( !productions_.empty() );
    ParserProduction* production = productions_.back().get();
    production->append_symbol( symbol );
}

/**
// Set the action to be taken when the current production is reduced.
//
// @param action
//  The action to take when the current production is reduced.
*/
void ParserGrammar::action( ParserAction* action )
{
    SWEET_ASSERT( !productions_.empty() );
    productions_.back()->set_action( action );
}

/**
// Set the precedence of the current production to match the precedence
// give to \e symbol.
//
// @param symbol
//  The symbol to set the precedence of the current production to match 
//  (assumed not null).
*/
void ParserGrammar::precedence_symbol( ParserSymbol* symbol )
{
    SWEET_ASSERT( !productions_.empty() );
    productions_.back()->set_precedence_symbol( symbol );
}
