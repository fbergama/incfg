/*! \mainpage incfg

incfg is a small C++ library to easily manage program configuration options (aka. command line arguments,
program options, etc).

incfg handles configuration options simply as key-value pairs whose keys are defined and checked
at compile time and values are gathered automatically just before the program starts (ie. before the
execution of ```main()```).

Once defined, the presence and type of a key-value pair is guaranteed at compile time so that it
can be used throughout the code with minimal effort. (See Usage Example below for a quick start).

Configuration values can be loaded from a "configuration string" like:

```
key1 = value1
key2 = value2
....
keyN = valueN
```

or via classic ```argv, argc``` command-line string arrays.

Type conversion (from a type ```T``` to/from ```std::string```) is handled automatically via ```std::stringstream```
or can be extended easily for custom types.


# Usage Example

Suppose that we have a function
```
void logtofile( std::string log_data )
{
    // saves log_data to a file...
    // ....
}

```

in which we foresee to use 3 configuration options:

- An ```unsigned int BUFFER_SIZE``` option for the file copy algorithm (default value 4096)
- A  ```bool DEBUG_LOG``` option to enable/disable debug logging (default value ```false```)
- A ```std::string LOGFILENAME``` option to specify the log file name (default value ```"log.txt"```)


We start by declaring the options we need *before* the function definition:

```
INCFG_REQUIRE( unsigned int, BUFFER_SIZE, 4096, "Buffer size used to write the log file" )
INCFG_REQUIRE( bool, DEBUG_LOG, false, "Enable verbose debug" )
INCFG_REQUIRE( std::string, LOGFILENAME, "log.txt", "Default log filename" )

void logtofile( std::string log_data )
{
    // saves log_data to a file...
    // ....
}

```

The ```INCFG_REQUIRE()``` macro registers a new configuration option by specifying:
1. A type
2. A name (the key)
3. A default value (must be castable to the type specified before)
4. A string describing the configuration option (Used to generate the configuration string/file automatically)


At this point, options can be freely (and safely!) retrieved inside the function body:


```
INCFG_REQUIRE( unsigned int, BUFFER_SIZE, 4096, "Buffer size used to write the log file" )
INCFG_REQUIRE( bool, DEBUG_LOG, false, "Enable verbose debug" )
INCFG_REQUIRE( std::string, LOGFILENAME, "log.txt", "Default log filename" )

void logtofile( std::string log_data )
{
    char* buff = new char[ INCFG_GET( BUFFER_SIZE ) ];

    std::string filename = INCFG_GET( LOGFILENAME );

    if( INCFG_GET( DEBUG_LOG ) )
    {
        // do some debug
    }

    // ...
}
```

Each time ```INCFG_GET()``` macro is used, the existence of the requested key is guaranteed
with a value that can be either its default value or something else (if the associated value
changed during the program execution).


## Loading configuration options from a string (or file)

One of the key features of incfg is that the list of configuration options is known before
the program execution. As a consequence, a configuration string can be automatically generated
with all the options required by the program.
The ```incfg::ConfigOptions``` singleton class is used to manage the options list. For example,
to generate a configuration string (that can be optionally saved to a file)


```
int main()
{
    std::string cfg = incfg::ConfigOptions::instance().to_config_string();
    std::ofstream ofs( "config.txt" );
    ofs << cfg;
    ofs.close();
}

```

With the options defined before, the following ```config.txt``` will be generated:

```
# Buffer size used to write the log file
#
#BUFFER_SIZE=4096

# Enable verbose debug
#
#DEBUG_LOG=false

# Default log filename
#
#LOGFILENAME="log.txt"

```

As you may expect, the same ```incfg::ConfigOptions``` singleton can be used to load config
options values from a configuration string:

```
int main()
{
    std::ifstream ifs( "config.txt" );

    try
    {
        incfg::ConfigOptions::instance().load( ifs );

    } catch( std::runtime_error& er )
    {
        // An exception is thrown if the configuration string/file cannot be parsed
    }
}
```

## Loading configuration options from command-line

Configuration options can be loaded from command-line by passing the command line
options given in ```argc``` and ```argv```:

```
int main( int argc, char* argv[] )
{
    try
    {
        incfg::ConfigOptions::instance().load( argc, argv );

    } catch( std::runtime_error& er )
    {
        std::cout << "Error: " << er.what() << std::endl;
    }
}
```

Each option key is prepended by a double dash and followed by its value. For the example
given before, a correct command line would be:

```
$ yourprogramname --BUFFER_SIZE 1024 --DEBUG_LOG --LOGFILENAME test.txt
```


## Customizing configuration option types

Each configuration option is saved/loaded as a string. By default the conversion
between an option type and a string is handled by ```std::stringstream``` but uncommon
types can be handled as well by defining the two template functions:

```
namespace incfg
{
    template <  >
    inline std::string to_string_helper< T >( T val )
    {
        // return an std::string representation of T
    }

    template < >
    inline T from_string_helper( std::string str, const T& mytype )
    {
        // parse and return T from its string representation str
    }
}
```


# Installing

Just import ```incfg.hpp``` and ```incfg.cpp``` in your project :)


# License

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

#ifndef INCFG_INCFG_HPP
#define INCFG_INCFG_HPP


#include <stddef.h>
#include <iosfwd>
#include <string>
#include <map>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>


/*! \file incfg.hpp
 * \brief incfg hpp header
 *
 */

namespace incfg
{
    /*!
     * \brief StringParseException is thrown if a string cannot be parsed to its requested value type
     */
    class StringParseException : public std::runtime_error
    {
    public:
        explicit StringParseException (const std::string& what_arg) : std::runtime_error( what_arg ) {}
    };


    /*!
     * \brief ConfigOptionsLoadException is thrown if an error occurred while loading configuration options from a source
     */
    class ConfigOptionsLoadException : public std::runtime_error
    {
    public:
        explicit ConfigOptionsLoadException (const std::string& what_arg) : std::runtime_error( what_arg ) {}
    private:
    };


    /*!
     *  Generic to string converter via std::stringstream
     */
    template <typename T> std::string to_string_helper( T val )
    {
        std::stringstream ss;
        ss << val;
        return ss.str();
    }


    /*!
     * Generic from string converter via std::stringstream
     */
    template <typename T>
    T from_string_helper( std::string str, const T& mytype )
    {
        T val;
        std::stringstream ss(str);
        ss >> val;
        if( ss.fail() )
            throw StringParseException("Unable to parse "+str+" to its defined type");

        return val;
    }

    template < >
    inline bool from_string_helper( std::string str, const bool& mytype )
    {
        if( str.compare(std::string("true"))!=0 && str.compare(std::string("false"))!=0 )
            throw StringParseException("Unable to parse "+str+" to \"true\" or \"false\"");

        return str.compare( std::string("true") )==0;
    }


    template < >
    inline std::string from_string_helper( std::string str, const std::string& mytype )
    {
        if( str.length()<2 )
            return str;

        // remove quotes if necessary
        if( str[0]=='\"' && str[str.length()-1]=='\"')
            return str.substr(1,str.length()-2);

        return str;
    }

    template <  >
    inline std::string to_string_helper< std::string >( std::string val )
    {
        return std::string("\"")+val+std::string("\"");
    }

    template <  >
    inline std::string to_string_helper< char* >( char* val )
    {
        return std::string("\"")+val+std::string("\"");
    }

    template <  >
    inline std::string to_string_helper< bool >( bool val )
    {
        return val ? std::string("true") : std::string("false");
    }


    template <typename T>
    inline bool is_boolean( T _type ) { return false; }

    template < >
    inline bool is_boolean< bool >( bool _type ) { return true; }


    /**
     * @brief Configuration option interface
     */
    class Option
    {
    public:
        Option( const char* _name, const char* _description );
        const std::string name;
        const std::string description;
        virtual void parse_value_from_str( std::string str ) = 0;
        virtual std::string get_value_as_str() const = 0;
        virtual bool is_default() const = 0;
        virtual bool is_bool() const = 0;
    };


    /*!
     * \brief ConfigOptions Class collects and manages all the required key-value pairs
     *
     * ConfigOptions is the main class used by incfg to manage all the required key-value pairs.
     * Before the execution of main(), all the REQUIRED configuration options (defined via INCFG_REQUIRE macro)
     * are automatically collected via ConfigOptions singleton class and initialized with their default value and
     * description.
     *
     * When your program is running, a single ConfigOptions instance is available to serve the following
     * two main tasks:
     *
     * 1. Generate a configuration string, that can be further written on a text file
     *
     * 2. Load configuration options from different sources like an std::string, std::istream or
     *    program input arguments (*argv[], argc)
     *
     * Additionally, ConfigOptions is internally used by the type-safe macro INCFG_GET( key ) to retrieve the
     * value of a configuration option given its key. The existence of a configuration option given its key
     * is guaranteed at compile time.
     *
     */
    class ConfigOptions
    {
    public:

        /*!
         * \return A reference of the singleton ConfigOptions object.
         */
        static ConfigOptions& instance();


        inline void add_option( Option* opt )
        {
            if( options.find( opt->name ) == options.end() ) {
                options[opt->name] = opt;
            }
        }


        /*!
         * \brief Returns the ```Option``` interface to a given option key
         * \param name option name (key)
         */
        inline Option* get( std::string name )  { return options[ name ]; }


        /*!
         * \brief Loads configuration options from an input stream
         * \param _isr Input stream
         */
        void load( std::istream& _isr);


        /*!
         * \brief Loads configuration options from an std::string
         * \param _str configuration string
         */
        void load( std::string& _str );


        /*!
         * \brief Loads configuration options from the command line
         */
        void load( int argc, char* argv[] );


        /*!
         * \brief returns a configuration string for the currently required list of options
         */
        inline std::string to_config_string() const
        {
            std::stringstream ss;
            //ss << "# Generated config file" << std::endl;
            for( std::map< std::string, Option*>::const_iterator it=options.begin(); it!=options.end(); ++it )
            {
                if( it->second->description.length() > 0 )
                {
                    ss << "# " << it->second->description << std::endl;
                    ss << "# " << std::endl;
                }
                ss << (it->second->is_default()?"#":"") << it->first <<  "=" << it->second->get_value_as_str() << std::endl << std::endl;
            }
            return ss.str();
        }


        /*!
         * \brief returns the number of currenlty managed configuration options
         */
        inline size_t size() const
        {
            return options.size();
        }


        /*!
         * \brief option_by_index returns the idx^th configuration option
         * \param idx index (0..size()-1) of the configuration option
         */
        inline Option* option_by_index( size_t idx ) const
        {
            std::map< std::string, Option* >::const_iterator it = options.begin();

            while( idx>0 )
            {
                ++it;
                --idx;
            }

            if( it!=options.end() )
            {
                return it->second;
            }

            return 0;
        }


    private:
        inline ConfigOptions() {}
        ConfigOptions( const ConfigOptions& other );
        ConfigOptions& operator=( const ConfigOptions& other );
        std::map< std::string, Option* > options;
        void add_if_possible( const std::string& key, const std::string& value );

    };


    inline Option::Option( const char* _name, const char* _description ) : name(_name), description(_description) {
        ConfigOptions::instance().add_option( this );
    }
}






/*!
 *  \brief Globally declares that a configuration key-value pair is required
 *  \hideinitializer
 *
 * \param TYPE Value type
 * \param CONFIGNAME Configuration option name (key)
 * \param DEFAULTVAL Default option value
 * \param DESCRIPTION Option description (c-string)
 */
#define INCFG_REQUIRE( TYPE, CONFIGNAME, DEFAULTVAL, DESCRIPTION )\
class incfg_  ## CONFIGNAME ## _Option : public incfg::Option \
{ \
public: \
incfg_  ## CONFIGNAME ## _Option() : incfg::Option(  # CONFIGNAME  , # DESCRIPTION ), is_def(false) {} \
incfg_  ## CONFIGNAME ## _Option( TYPE  v ) : incfg::Option(  # CONFIGNAME ,  DESCRIPTION  ), is_def(true), value(v) {}; \
inline void parse_value_from_str( std::string str ) \
{ \
    set( incfg::from_string_helper< TYPE >( str, value ) );\
}\
inline std::string get_value_as_str() const\
{\
    return incfg::to_string_helper< TYPE >( value );\
}\
inline bool is_default() const { return is_def; } \
inline bool is_bool() const { return incfg::is_boolean< TYPE >( value ); } \
inline TYPE get() const { return value; }\
inline void set( const TYPE& new_value )\
{\
    is_def = is_def && (new_value == value);\
    value = new_value;\
}\
private:\
TYPE value;\
bool is_def;\
};\
static incfg_  ## CONFIGNAME ## _Option incfg_  ## CONFIGNAME ## _Option_instance( DEFAULTVAL );


/*!
 * Returns the value associated to a CONFIGNAME key
 * \hideinitializer
 *
 */
#define INCFG_GET( CONFIGNAME )\
(incfg_  ## CONFIGNAME ## _Option(), dynamic_cast< incfg_  ## CONFIGNAME ## _Option* >( incfg::ConfigOptions::instance().get( # CONFIGNAME ))->get() )


/*!
 * \brief Associates a new VALUE to CONFIGNAME key
 * \hideinitializer
 *
 */
#define INCFG_SET( CONFIGNAME, VALUE )\
(incfg_  ## CONFIGNAME ## _Option(), dynamic_cast< incfg_  ## CONFIGNAME ## _Option* >( incfg::ConfigOptions::instance().get( # CONFIGNAME ))->set(VALUE) )



#endif //INCFG_INCFG_HPP_H
