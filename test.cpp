#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "incfg.hpp"

// All incfg requirements must be in the global scope
//
INCFG_REQUIRE( int, opt1, 10, "option 1")
INCFG_REQUIRE( double, opt2, 20.1, "option 2")
INCFG_REQUIRE( std::string, opt3, "opt3!", "option 3")
INCFG_REQUIRE( bool, opt4, true, "boolean true option")
INCFG_REQUIRE( bool, opt5, false, "boolean false option")


SCENARIO("Requiring/Getting options", "[Basic]")
{

    GIVEN( "Some options required" )
    {
        REQUIRE( INCFG_GET(opt1)==10 );
        REQUIRE( INCFG_GET(opt2)==20.1 );
    }
}


SCENARIO("Config File generation", "[ConfigGen]")
{

    GIVEN( "An option called opt3 required" )
    {
        REQUIRE( INCFG_GET(opt3)==std::string("opt3!") );

        WHEN("Config string is generated")
        {
            THEN("Should appear in the config string")
            {
                REQUIRE( incfg::ConfigOptions::instance().to_config_string().find( std::string("opt3=\"opt3!\"") ) != std::string::npos );
            }
            THEN("Should have default value")
            {
                REQUIRE( incfg::ConfigOptions::instance().to_config_string().find( std::string("#opt3=\"opt3!\"") ) != std::string::npos );
            }
        }
    }

    GIVEN( "3 options named: opt1 opt2 opt3 required")
    {
        // required is not needed here because options existence is enforced at compile time
        INCFG_GET(opt1);
        INCFG_GET(opt2);
        INCFG_GET(opt3);

        WHEN("Config file is generated")
        {
            THEN("All of them should appear in the config string")
            {
                std::string cfg = incfg::ConfigOptions::instance().to_config_string();
                //std::cout << cfg << std::endl;
                REQUIRE( cfg.find("opt1") != std::string::npos );
                REQUIRE( cfg.find("opt2") != std::string::npos );
                REQUIRE( cfg.find("opt3") != std::string::npos );
            }
        }
        WHEN("The value of opt2 is set to a non-default value")
        {
            INCFG_SET(opt2,100.0);
            THEN("In the config string it should not be commented")
            {
                std::string cfg = incfg::ConfigOptions::instance().to_config_string();
                REQUIRE( cfg.find("#opt2") == std::string::npos );
            }
        }

    }
}

SCENARIO("Config String parsing", "[ConfigParse]")
{
    GIVEN( "An option called opt1 of type int" )
    {
        REQUIRE( INCFG_GET(opt1) );

        WHEN("Config string is parsed with an int value")
        {
            std::string confstr( "opt1=30\n");
            THEN("Parse should be successful")
            {
                incfg::ConfigOptions::instance().load( confstr );
                REQUIRE( INCFG_GET(opt1)==30 );
            }
        }

        WHEN("Config string is parsed with a non int parsable value")
        {
            std::string confstr( "opt1=aaa30\n");
            THEN("Exception should be thrown")
            {
                try {
                    incfg::ConfigOptions::instance().load( confstr );
                    REQUIRE( false );
                } catch( incfg::StringParseException& ex )
                {
                    REQUIRE( true ); // Exception should be thrown
                }
            }
        }
    }

    GIVEN( "An option called opt3 of type string" )
    {
        WHEN("Config string is parsed with a string value")
        {
            std::string confstr( "opt3=\"test\"\n");
            THEN("Parse should be successful")
            {
                incfg::ConfigOptions::instance().load( confstr );
                //std::cout << INCFG_GET(opt3) << std::endl;
                REQUIRE( INCFG_GET(opt3).compare("test") == 0 );
            }
        }

        /*
        WHEN("Config string is parsed with a non string parsable value")
        {
            std::string confstr( "opt3=  30\n");
            THEN("Exception should be thrown")
            {
                try {
                    incfg::ConfigOptions::instance().load( confstr );
                    REQUIRE( false );
                } catch( incfg::StringParseException& ex )
                {
                    REQUIRE( true ); // Exception should be thrown
                }
            }
        }
        */

        WHEN("Config string is parsed with an empty string")
        {
            std::string confstr( "opt3=         \"\"    \n");
            THEN("Parse should be successful")
            {
                incfg::ConfigOptions::instance().load( confstr );
                REQUIRE( INCFG_GET(opt3).length() == 0 );
            }
        }
        WHEN("String contains spaces")
        {
            std::string confstr( "opt3=\" test test \"\n");
            THEN("Parse should be successful")
            {
                incfg::ConfigOptions::instance().load( confstr );
                REQUIRE( INCFG_GET(opt3).compare(std::string(" test test "))== 0 );
            }
        }
    }

    GIVEN( "An option called opt4 of type bool" )
    {
        REQUIRE( incfg::ConfigOptions::instance().get("opt4")->is_bool() );
        WHEN("Config string is parsed with a boolean true value")
        {
            std::string confstr( "opt4   =  true\n");
            THEN("Parse should be successful")
            {
                incfg::ConfigOptions::instance().load( confstr );
                REQUIRE( INCFG_GET(opt4) );
            }
        }
        WHEN("Config string is parsed with a boolean false value")
        {
            std::string confstr( "opt4   =false\n");
            THEN("Parse should be successful")
            {
                incfg::ConfigOptions::instance().load( confstr );
                REQUIRE( !INCFG_GET(opt4) );
            }
        }
        WHEN("Config string is parsed with a non boolean parsable value")
        {
            std::string confstr( "opt4=  30\n");
            THEN("Exception should be thrown")
            {
                try {
                    incfg::ConfigOptions::instance().load( confstr );
                    REQUIRE( false );
                } catch( incfg::StringParseException& ex )
                {
                    REQUIRE( true ); // Exception should be thrown
                }
            }
        }
    }
}


SCENARIO("Loading a self-generated config string", "[ConfigParseSelf]")
{
    GIVEN("ALL Options changed from default value");
    {
        INCFG_SET(opt1,30);
        INCFG_SET(opt2,31.2);
        INCFG_SET(opt3,"test");
        INCFG_SET(opt4,false);
        INCFG_SET(opt5,true);
        WHEN("Config String is generated")
        {
            std::string conf = incfg::ConfigOptions::instance().to_config_string();
            //std::cout << "###" << conf << "###" << std::endl;
            incfg::ConfigOptions::instance().load( conf );
            THEN("All options should be parsed correctly")
            {
                REQUIRE( INCFG_GET(opt1)==30 );
                REQUIRE( INCFG_GET(opt2)==31.2 );
                REQUIRE( INCFG_GET(opt3).compare("test")==0 );
                REQUIRE( !INCFG_GET(opt4) );
                REQUIRE( INCFG_GET(opt5) );
            }
        }
    }
}


SCENARIO("Loading from command line", "[CmdLine]" )
{
    GIVEN("opt1 given correctly from command line")
    {
        char* argv[] = {"exename", "--opt1", "4"};
        WHEN("Command line is parsed")
        {
            incfg::ConfigOptions::instance().load( 3, argv );
            THEN("opt1 should be parsed correctly")
            {
                REQUIRE(INCFG_GET(opt1)==4);
            }
        }
    }

    GIVEN("opt1 given with no value from command line")
    {
        char* argv[] = {"exename", "--opt1", "--opt2"};
        WHEN("Command line is parsed")
        {
            THEN("opt1 should not be parsed correctly")
            {
                try{
                    incfg::ConfigOptions::instance().load( 3, argv );
                    REQUIRE(false);
                }catch(...)
                {
                    REQUIRE(true);
                }
            }
        }
    }

    GIVEN("opt1 given with a bad value from command line")
    {
        char* argv[] = {"exename", "--opt1", "a"};
        WHEN("Command line is parsed")
        {
            THEN("opt1 should not be parsed correctly")
            {
                try{
                    incfg::ConfigOptions::instance().load( 3, argv );
                    REQUIRE(false);
                }catch(...)
                {
                    REQUIRE(true);
                }
            }
        }
    }

    GIVEN("string option given from command line")
    {
        char* argv[] = {"exename", "--opt3", "test"};
        WHEN("Command line is parsed")
        {
            THEN("option should be parsed correctly")
            {
                try{
                    incfg::ConfigOptions::instance().load( 3, argv );
                    REQUIRE(true);
                }catch( std::runtime_error& ex )
                {
                    std::cout << "Exception: " << ex.what() << std::endl;
                    REQUIRE(false);
                }
            }
        }

    }
}
