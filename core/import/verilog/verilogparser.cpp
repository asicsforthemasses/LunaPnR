// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <sstream>
#include <iostream>
#include "verilogparser.h"
#include "common/logging.h"

using namespace ChipDB::Verilog;

Lexer::token Parser::previous() const
{
    return m_tokens->at(m_idx-1);
}

Lexer::token Parser::peek() const
{
    return m_tokens->at(m_idx);
}

void Parser::advance()
{
    if (!atEnd())
        m_idx++;
}

bool Parser::match(Lexer::tokenId tok)
{
    if (atEnd())
        return false;

    if (peek().m_tok == tok)
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::atEnd() const
{
    return m_idx >= m_tokens->size();
}

void Parser::error(const std::string &error)
{
    std::stringstream ss;
    ss << "Line: " << peek().m_line << " col: " << peek().m_col << " " << error;
    Logging::doLog(Logging::LogType::ERROR, ss.str());
}

bool Parser::execute(const std::vector<Lexer::token> &tokens)
{
    m_tokens = &tokens;
    m_idx = 0;

    while(!atEnd())
    {
        if (match(Lexer::TOK_MODULE))
        {
            if (!module())
            {
                error("Error parsing module.\n");
                return false;
            }
        }
        else if (match(Lexer::TOK_ATTRIBUTE))
        {
            onAttribute(previous().m_value);
        }
        else
        {
            // error?
            error("???\n");
            return false;
        }
    }

    return true;
}

bool Parser::module()
{
    // module_ident '(' ')' ';'

    if (!match(Lexer::TOK_IDENT))
    {
        error("Expected module name.\n");
        return false;
    }

    std::string modName = previous().m_value;
    std::vector<std::string> ports;

    if (!match(Lexer::TOK_LPAREN))
    {
        error("Expected '('\n");
        return false;
    }

    if (match(Lexer::TOK_IDENT))
    {
        // first port
        ports.push_back(previous().m_value);

        while(match(Lexer::TOK_COMMA))
        {
            if (!match(Lexer::TOK_IDENT))
            {
                error("Expected a port name\n");
                return false;
            }
            // additional port
            ports.push_back(previous().m_value);
        }
    }

    if (!match(Lexer::TOK_RPAREN))
    {
        error("Expected )\n");
        return false;
    }

    if (!match(Lexer::TOK_SEMICOL))
    {
        error("Expected ; at end of module declaration.\n");
        return false;
    }

    onModule(modName, ports);

    return moduleItems();
}


bool Parser::moduleItems()
{
    // items: INPUT, OUTPUT, WIRE, ASSIGN

    while(!match(Lexer::TOK_ENDMODULE))
    {
        if (match(Lexer::TOK_INPUT))
        {
            pInput();
        }
        else if (match(Lexer::TOK_OUTPUT))
        {
            pOutput();
        }
        else if (match(Lexer::TOK_ASSIGN))
        {
            pAssign();
        }
        else if (match(Lexer::TOK_WIRE))
        {
            pWire();
        }
        else if (match(Lexer::TOK_ATTRIBUTE))
        {
            onAttribute(previous().m_value);
        }
        else if (match(Lexer::TOK_IDENT))
        {
            // gate/module instantiation?
            pInstance();
        }
        else
        {
            std::stringstream ss;
            ss << "Unexpected token: " << peek().m_value << ".\n";
            error(ss.str());
            return false;
        }
    }

    return true;
}

bool Parser::pInput()
{
    // ('[' INTEGER ':' INTEGER ']') ident ';'

    bool hasRange = false;
    uint32_t start = 0; // range start default
    uint32_t stop  = 0; // range stop default
    if (match(Lexer::TOK_LBRACKET))
    {
        if (!match(Lexer::TOK_INTEGER))
        {
            error("Expected integer in range.\n");
            return false;
        }
        
        // FIXME: better type checking
        start = atoi(previous().m_value.c_str());

        if (!match(Lexer::TOK_COLON))
        {
            error("Expected ':' in range.\n");
            return false;       
        }

        if (!match(Lexer::TOK_INTEGER))
        {
            error("Expected integer in range.\n");
            return false;
        }    

        // FIXME: better type checking
        stop = atoi(previous().m_value.c_str());

        if (!match(Lexer::TOK_RBRACKET))
        {
            error("Expected ']' in range.\n");
            return false;
        }

        hasRange = true;
    }

    if (!match(Lexer::TOK_IDENT))
    {
        error("Expected identifier.\n");
        return false;
    }

    if (hasRange) 
        onInput(previous().m_value, start, stop);
    else
        onInput(previous().m_value);

    if (!match(Lexer::TOK_SEMICOL))
    {
        error("Expected semicol.\n");
        return false;
    }

    return true;
}

bool Parser::pOutput()
{
    // ('[' INTEGER ':' INTEGER ']') ident ';'

    bool hasRange = false;
    uint32_t start = 0; // range start default
    uint32_t stop  = 0; // range stop default
    if (match(Lexer::TOK_LBRACKET))
    {
        if (!match(Lexer::TOK_INTEGER))
        {
            error("Expected integer in range.\n");
            return false;
        }
        
        // FIXME: better type checking
        start = atoi(previous().m_value.c_str());

        if (!match(Lexer::TOK_COLON))
        {
            error("Expected ':' in range.\n");
            return false;       
        }

        if (!match(Lexer::TOK_INTEGER))
        {
            error("Expected integer in range.\n");
            return false;
        }    

        // FIXME: better type checking
        stop = atoi(previous().m_value.c_str());

        if (!match(Lexer::TOK_RBRACKET))
        {
            error("Expected ']' in range.\n");
            return false;
        }

        hasRange = true;
    }

    if (!match(Lexer::TOK_IDENT))
    {
        error("Expected identifier.\n");
        return false;
    }

    if (hasRange)
        onOutput(previous().m_value, start, stop);
    else
        onOutput(previous().m_value);

    if (!match(Lexer::TOK_SEMICOL))
    {
        error("Expected semicol.\n");
        return false;
    }

    return true;
}

bool Parser::pWire()
{
    // ('[' INTEGER ':' INTEGER ']') ident ';'
    // or a comma separated list of those.

    do 
    {
        uint32_t start = 0; // range start default
        uint32_t stop  = 0; // range stop default
        bool hasRange = false;
        if (match(Lexer::TOK_LBRACKET))
        {
            hasRange = true;
            if (!match(Lexer::TOK_INTEGER))
            {
                error("Expected integer in range.\n");
                return false;
            }
            
            // FIXME: better type checking
            start = atoi(previous().m_value.c_str());

            if (!match(Lexer::TOK_COLON))
            {
                error("Expected ':' in range.\n");
                return false;       
            }

            if (!match(Lexer::TOK_INTEGER))
            {
                error("Expected integer in range.\n");
                return false;
            }    

            // FIXME: better type checking
            stop = atoi(previous().m_value.c_str());

            if (!match(Lexer::TOK_RBRACKET))
            {
                error("Expected ']' in range.\n");
                return false;
            }
        }

        if (!match(Lexer::TOK_IDENT))
        {
            error("Expected identifier.\n");
            return false;
        }

        if (hasRange)
        {
            onWire(previous().m_value, start, stop);
        }
        else
        {
            onWire(previous().m_value);
        }

    } while(match(Lexer::TOK_COMMA));

    if (!match(Lexer::TOK_SEMICOL))
    {
        error("Expected semicol.\n");
        return false;
    }

    return true;
}

bool Parser::pAssign()
{
    // ident = ident ';'

    // ident ';'
    if (!match(Lexer::TOK_IDENT))
    {
        error("Expected identifier.\n");
        return false;
    }

    std::string left = previous().m_value;

    if (!match(Lexer::TOK_EQUAL))
    {
        error("Expected '='.\n");
        return false;
    }

    if (!match(Lexer::TOK_IDENT))
    {
        error("Expected identifier.\n");
        return false;
    }

    std::string right = previous().m_value;

    if (!match(Lexer::TOK_SEMICOL))
    {
        error("Expected ';'.\n");
        return false;
    }

    onAssign(left, right);

    return true;    
}

bool Parser::pInstance()
{
    std::string itype = previous().m_value;

    // iname '(' port_connection_list ')' ';'
    if (!match(Lexer::TOK_IDENT))
    {
        error("Expected instance name.\n");
        return false;
    }

    std::string iname = previous().m_value;

    if (!match(Lexer::TOK_LPAREN))
    {
        error("Expected '(' after instance name.\n");
        return false;
    }

    // emit onInstance before the port list
    onInstance(itype, iname);

    if (peek().m_tok == Lexer::TOK_PERIOD)
    {
        if (!pInstanceNamedPortList())
            return false;
    }
    else
    {
        if (!pInstanceOrderedPortList())
            return false;
    }

    if (!match(Lexer::TOK_RPAREN))
    {
        error("Expected ')' after port connections.\n");
        return false;
    }

    if (!match(Lexer::TOK_SEMICOL))
    {
        error("Expected ';' at end of instance.\n");
        return false;
    }

    return true;
}

bool Parser::pInstanceOrderedPortList()
{
    uint32_t portIndex = 0;
    // accept zero or more net names
    if (match(Lexer::TOK_IDENT))
    {
        std::string netname = previous().m_value;

        // optional slice [x] or [x:y]
        if (match(Lexer::TOK_LBRACKET))
        {
            if (!match(Lexer::TOK_INTEGER))
            {
                error("Expected integer in slice.\n");
                return false;
            }
            netname += '[';
            netname += previous().m_value;
            netname += ']';
            if (!match(Lexer::TOK_RBRACKET))
            {
                error("Expected ']' in slice.\n");
                return false;                
            }
        }
        onInstancePort(portIndex++, netname);

        while(match(Lexer::TOK_COMMA))
        {
            if (!match(Lexer::TOK_IDENT))
            {
                error("Expected net name.\n");
                return false;
            }
            netname = previous().m_value;

            // optional slice [x] or [x:y]
            if (match(Lexer::TOK_LBRACKET))
            {
                if (!match(Lexer::TOK_INTEGER))
                {
                    error("Expected integer in slice.\n");
                    return false;
                }
                netname += '[';
                netname += previous().m_value;
                netname += ']';
                if (!match(Lexer::TOK_RBRACKET))
                {
                    error("Expected ']' in slice.\n");
                    return false;                
                }
            }
            onInstancePort(portIndex++, netname);
        }
    }
    return true;
}

bool Parser::pInstanceNamedPortList()
{
    // accept zero or more net names
    
    if (peek().m_tok == Lexer::TOK_RPAREN)
    {
        return true;
    }

    do
    {
        if (!match(Lexer::TOK_PERIOD))
        {
            error("Expected '.' in named port connection.\n");
            return false;
        }

        if (!match(Lexer::TOK_IDENT))
        {
            error("Expected port name.\n");
            return false;
        }

        std::string portname = previous().m_value;

        if (!match(Lexer::TOK_LPAREN))
        {
            error("Expected '(' in named port connection.\n");
            return false;
        }

        if (!match(Lexer::TOK_IDENT))
        {
            error("Expected net name.\n");
            return false;
        }    

        std::string netname = previous().m_value;

        // optional slice [x] or [x:y]
        if (match(Lexer::TOK_LBRACKET))
        {
            if (!match(Lexer::TOK_INTEGER))
            {
                error("Expected integer in slice.\n");
                return false;
            }
            netname += '[';
            netname += previous().m_value;
            netname += ']';            
            if (!match(Lexer::TOK_RBRACKET))
            {
                error("Expected ']' in slice.\n");
                return false;                
            }
        }

        if (!match(Lexer::TOK_RPAREN))
        {
            error("Expected ')' in named port connection.\n");
            return false;
        }

        onInstanceNamedPort(portname, netname);
    } while(match(Lexer::TOK_COMMA));

    return true;
}

#if 0

    // if there is a '.', we have a connect-by-name list
    if (match(Lexer::TOK_PERIOD))
    {
        do
        {
            if (!match(Lexer::TOK_IDENT))
            {
                error("Expected port name\n");
                return false;
            }
            std::string port = previous().m_value;
            if (!match(Lexer::TOK_LPAREN))
            {
                error("Expected '('\n");
            }
            if (!match(Lexer::TOK_IDENT))
            {
                error("Expected net name\n");
                return false;
            }
            std::string net = previous().m_value;
            if (!match(Lexer::TOK_RPAREN))
            {
                error("Expected ')'\n");
            }
        } while(match(Lexer::TOK_COMMA));
    }
    else
    {
        // connect by order list
        if (!match(Lexer::TOK_IDENT))
        {
            error("Expected net name\n");
            return false;
        }
    }

#endif