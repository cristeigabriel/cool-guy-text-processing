#include "main.h"

using namespace Stream;

int main()
{
	try
	{
		//	CPP nonpragma instance
		using cpp_game_type = CStream<ELanguage::CPP, false, true>;

		//	ease
		namespace cpp_nest = Nesting::CPP;
		namespace cpp_append = Append::CPP;

		cpp_game_type game( "..\\output\\c++.h" );

		game.newline();
		game.append( cpp_append::IAppend<cpp_append::AppendType_t::MULTILINE_COMMENT>::make( __DATE__ ) );
		game.append( cpp_append::IAppend<cpp_append::AppendType_t::PREPROCESSOR>::make( "include", "<cstdint>" ) );
		game.newline();

		game.nest<cpp_nest::NamedNestType_t::STRUCT>(
			"hello", [&]( cpp_game_type &obj )
		{
			obj.append( cpp_append::IAppend<cpp_append::AppendType_t::MULTILINE_COMMENT>
						::make( "This comment was made by " APPLICATION_NAME ) );
			obj.append( cpp_append::IAppend<cpp_append::AppendType_t::INLINE_COMMENT>
						::make( "This comment was made by " APPLICATION_NAME ) );
			obj.nest<cpp_nest::NamedNestType_t::NAMESPACE>
				( "CoolNamespace", [&]( cpp_game_type &obj )
			{
				obj.append( cpp_append::IAppend<cpp_append::AppendType_t::CONSTANT>
							::make( "cool_value", "0x152345125" ) );
			} );
		} );

		//	C pragma instance
		using c_game_type = CStream<ELanguage::C, true, true>;
		c_game_type newgame( "..\\output\\c.h" );

		//	ease
		namespace c_nest = Nesting::C;
		namespace c_append = Append::C;

		newgame.newline();
		newgame.append( c_append::IAppend<c_append::AppendType_t::MULTILINE_COMMENT>::make( __DATE__ ) );
		newgame.append( c_append::IAppend<c_append::AppendType_t::PREPROCESSOR>::make( "include", "<stdio.h>" ) );
		newgame.newline();

		newgame.nest<c_nest::UnnamedNestType_t::CONTROL_PATH>( [&]( c_game_type &obj )
		{
			//	This is an illegal expression but still supported by Cool Guy Text Processing
			obj.nest<c_nest::NamedNestType_t::STRUCT>( "yeh", [&]( c_game_type &obj )
			{

			} );
		} );
	}
	catch( const std::exception &except )
	{
		_RPT0( _CRT_ERROR, except.what() );
		DebugBreak();
	}

	return EXIT_SUCCESS;
}