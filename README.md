# incfg

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


