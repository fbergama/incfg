/*!
    incfg library (See incfg.hpp for description/usage)
--------------------------------------------------------------------------------

The MIT License
Copyright (c) 2015 Filippo Bergamasco

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include "incfg.hpp"
#include <iostream>


namespace incfg {


ConfigOptions& ConfigOptions::instance()
{
    static ConfigOptions co;
    return co;
}


void ConfigOptions::load( int argc, char* argv[] )
{
    if( argc<2 )
        return;

    for( size_t idx=1; idx<static_cast<size_t>(argc); idx++ )
    {
        std::string key( argv[idx] );
        //std::cout << "KEY: <" << key << ">" << std::endl;
        if( key.length()<3 )
            throw ConfigOptionsLoadException("Invalid configuration option: "+key);

        if( key[0]!='-' && key[1]!='-' )
            throw ConfigOptionsLoadException("Invalid configuration option: "+key);

        key = key.substr(2,key.length()-1);

        if( options.find(key) == options.end() )
        {
            throw ConfigOptionsLoadException("Unexpected key: " + key );
        }


        if( options[key]->is_bool() )
        {
            options[key]->parse_value_from_str( std::string("true") );
        }
        else
        {
            if( idx==argc )
                throw ConfigOptionsLoadException("A value is expected for configuration option " + key );

            std::string value( argv[++idx] );

            if( value.length()>1 && value[0]=='-' && value[1]=='-' )
                throw ConfigOptionsLoadException( value + " is an invalid value for key " + key );

            //std::cout << "VALUE: <" << value << ">" << std::endl;
            options[key]->parse_value_from_str( value );
        }
    }
}


void ConfigOptions::load( std::istream& _isr)
{
    if( _isr.fail() )
    {
        throw ConfigOptionsLoadException("IO Error.");
    }

    unsigned int linenum=0;
    std::string buff;
    std::getline( _isr, buff );

    while( !_isr.eof() || !_isr.fail() )
    {
        if( buff[0] == '#' || buff[0]==0  || buff[0]=='\n' || buff[0]=='\r' )
        {
            std::getline( _isr, buff );linenum++;
            continue;
        }

        // Remove all spaces before and after a "
        size_t end_idx = buff.find_first_of("\"");
        if( end_idx == std::string::npos )
            end_idx = buff.length();
        buff.erase(std::remove( buff.begin(), buff.begin()+end_idx, ' '), buff.begin()+end_idx );
        size_t start_idx = buff.find_last_of("\"");
        if( start_idx == std::string::npos )
            start_idx = 0;
        buff.erase(std::remove( buff.begin()+start_idx, buff.end(), ' '), buff.end() );


        size_t eq_idx = buff.find_first_of('=');

        if( eq_idx == std::string::npos || eq_idx==0 )
        {
            std::stringstream err;
            err << "Parse error at line " << linenum << ": No key found (<key> = <value> expected)";
            throw ConfigOptionsLoadException(err.str());
        }

        std::string key = buff.substr(0,eq_idx);
        std::string value = buff.substr(eq_idx+1, buff.length() );

        //std::cout << "READ: <" << key << "> = <" << value << ">" << std::endl;

        try
        {
            add_if_possible( key, value );

        } catch( StringParseException& ex )
        {
            std::stringstream errstr;
            errstr << "Config file error for key <" << key << "> (Line " << linenum-1 << "): " << ex.what();
            throw StringParseException( errstr.str() );
        }

        std::getline( _isr, buff );linenum++;
    }
}


void ConfigOptions::load( std::string& _str )
{
    std::istringstream ss(_str );
    load( ss );
}


void ConfigOptions::add_if_possible( const std::string& key, const std::string& value )
{
    if( options.find(key) == options.end() )
    {
        throw ConfigOptionsLoadException("Unexpected key: " + key );
    }

    options[key]->parse_value_from_str( value );
}

}

