#pragma once

//	Dependencies
#include <cassert>
#include <memory>
#include <functional>
#include <fstream>
#include <string>

//	Defines
#define APPLICATION_NAME      "COOL_GUY_TEXT_PROCESSING"
#define FORMAT_DELIMIT_INLINE_SYNTAX false
#define FORMAT_DELIMITER	  "\t"

#define INLINE_CONTROL_PATH_OPENING_BRACKET false
#define INLINE_STRUCT_OPENING_BRACKET       true
#define INLINE_NAMESPACE_OPENING_BRACKET    true
#define EXTRA_ENDLINE_PREPROCESSORS         false

//	This is purely a formatting matter
#define EXTRA_ENDLINE_AFTER_CONTROL_PATH_OPENING_BRACKET true
#define EXTRA_ENDLINE_AFTER_CONTROL_PATH_CLOSING_BRACKET true
#define EXTRA_ENDLINE_AFTER_NAMESPACE_OPENING_BRACKET    true
#define EXTRA_ENDLINE_AFTER_NAMESPACE_CLOSING_BRACKET    false
#define EXTRA_ENDLINE_AFTER_STRUCT_OPENING_BRACKET       true
#define EXTRA_ENDLINE_AFTER_STRUCT_CLOSING_BRACKET       false

namespace Stream
{
	using HandlerType_t = uint32_t;

	enum ELanguage
	{
		CPP,
		C
	};

	namespace Append
	{
		class CBaseAppend
		{
			public:
			void initialize( const char *content )
			{
				m_content = content;
			}

			void initialize( const char *content, const char *value )
			{
				m_content = content;
				m_value = value;
			}

			virtual void append( std::ofstream &stream,
								 unsigned int &enclosure_counter ) = 0;

			public:
			const char *m_content = nullptr;
			const char *m_value = nullptr;
		};

		namespace CPP
		{
			struct AppendType_t
			{
				enum : HandlerType_t
				{
					PREPROCESSOR,
					INLINE_COMMENT,
					MULTILINE_COMMENT,
					CONSTANT,
					LENGTH
				};
			};

			template <HandlerType_t type>
			class IAppend : public CBaseAppend
			{
				public:
				static std::unique_ptr<IAppend> make( const char *name )
				{
					std::unique_ptr<IAppend> ret = std::make_unique<IAppend>();
					ret.get()->initialize( name );
					return ret;
				}

				static std::unique_ptr<IAppend> make( const char *name, const char *value )
				{
					std::unique_ptr<IAppend> ret = std::make_unique<IAppend>();
					ret.get()->initialize( name, value );
					return ret;
				}

				void append( std::ofstream &stream,
							 unsigned int &enclosure_counter ) override
				{
					if( !m_content )
					{
						throw std::runtime_error( "Failed appending (m_content was nullptr)" );
					}

					for( auto i = 0u; i < enclosure_counter; ++i )
					{
						stream << '\t';
					}

					if( type == AppendType_t::PREPROCESSOR )
					{
						if( m_value != nullptr )
						{
							stream << "#" << m_content << ' ' << m_value
							#if EXTRA_ENDLINE_PREPROCESSORS == true
								<< '\n';
						#endif
							;
						}
						else
						{
							throw std::runtime_error( "Failed appending preprocessor (m_value was nullptr)" );
						}
					}

					if( type == AppendType_t::INLINE_COMMENT )
					{
						stream << "//" FORMAT_DELIMITER << m_content;
					}

					if( type == AppendType_t::MULTILINE_COMMENT )
					{
						stream << "/* " FORMAT_DELIMITER << m_content << " */";
					}

					if( type == AppendType_t::CONSTANT )
					{
						if( m_value != nullptr )
						{
							stream << "[[nodiscard]] static constexpr uint32_t " << m_content << " = " << m_value << ';';
						}
						else
						{
							throw std::runtime_error( "Failed appending constant (m_value was nullptr)" );
						}
					}

					stream << '\n';
				}
			};
		}

		namespace C
		{
			struct AppendType_t
			{
				enum : HandlerType_t
				{
					PREPROCESSOR,
					INLINE_COMMENT,
					MULTILINE_COMMENT,
					CONSTANT,
					LENGTH
				};
			};

			template <HandlerType_t type>
			class IAppend : public CBaseAppend
			{
				public:
				static std::unique_ptr<IAppend> make( const char *name )
				{
					std::unique_ptr<IAppend> ret = std::make_unique<IAppend>();
					ret.get()->initialize( name );
					return ret;
				}

				static std::unique_ptr<IAppend> make( const char *name, const char *value )
				{
					std::unique_ptr<IAppend> ret = std::make_unique<IAppend>();
					ret.get()->initialize( name, value );
					return ret;
				}

				void append( std::ofstream &stream,
							 unsigned int &enclosure_counter ) override
				{
					assert( m_content != nullptr );

					for( auto i = 0u; i < enclosure_counter; ++i )
					{
						stream << '\t';
					}

					if( type == AppendType_t::PREPROCESSOR )
					{
						if( m_value != nullptr )
						{
							stream << "#" << m_content << ' ' << m_value
							#if EXTRA_ENDLINE_PREPROCESSORS == true
								<< '\n';
						#endif
							;
						}
						else
						{
							throw std::runtime_error( "Failed appending preprocessor (m_value was nullptr)" );
						}
					}

					if( type == AppendType_t::INLINE_COMMENT )
					{
						stream << "//" FORMAT_DELIMITER << m_content;
					}

					else if( type == AppendType_t::MULTILINE_COMMENT )
					{
						stream << "/* " FORMAT_DELIMITER << m_content << " */";;
					}

					else if( type == AppendType_t::CONSTANT )
					{
						if( m_value != nullptr )
						{
							stream << "static const uint32_t " << m_content << " = " << m_value << ';';
						}
						else
						{
							throw std::runtime_error( "Failed appending constant (m_value was nullptr)" );
						}
					}

					stream << '\n';
				}
			};
		}
	}

	namespace Nesting
	{
		namespace CPP
		{
			struct UnnamedNestType_t
			{
				enum : HandlerType_t
				{
					CONTROL_PATH,
					NAMESPACE,
					LENGTH
				};
			};

			struct NamedNestType_t
			{
				enum : HandlerType_t
				{
					STRUCT,
					NAMESPACE,
					LENGTH
				};
			};
		}

		namespace C
		{
			struct UnnamedNestType_t
			{
				enum : HandlerType_t
				{
					CONTROL_PATH,
					LENGTH
				};
			};

			struct NamedNestType_t
			{
				enum : HandlerType_t
				{
					STRUCT
				};
			};
		}
	}

	//	Handler
	template <ELanguage language, bool use_pragma = true,
		bool is_header = true>
		class CStream
	{
		public:
		CStream( const char *file_name ) :
			m_file_name( file_name )
		{
			//	Prepare stream for use
			m_stream.open( file_name, std::ofstream::in |
						   std::ofstream::out |
						   //	This'll allow for it to be cleared
						   std::ofstream::trunc );

			if( m_stream.is_open() )
			{
				if( is_header )
				{
					//	This kind of scenarios could use our
					//	own functions, but I'll by-hand them anyway
					if( language == ELanguage::CPP ||
						language == ELanguage::C )
					{
						if( use_pragma )
						{
							m_stream << "#pragma once\n"
							#if EXTRA_ENDLINE_PREPROCESSORS == true
								"\n";
						#else
								;
						#endif
						}
						else
						{
							m_stream << "#ifndef " APPLICATION_NAME "\n"
								"#define " APPLICATION_NAME "\n"
							#if EXTRA_ENDLINE_PREPROCESSORS == true
								"\n";
						#else
								;
						#endif
						}
					}
				}
			}
			else
			{
				throw std::runtime_error( "Stream is locked after initialization" );
			}
		}

		~CStream()
		{
			//	If we aren't using pragma, close
			//	the preprocessor context
			if( m_stream.is_open() )
			{
				if( is_header )
				{
					if( !use_pragma )
					{
						m_stream << "\n#endif";
					}
				}

				//	This'll stop the stream from acting
				//	upon our file and allow for safe
				//	interaction
				m_stream.close();
			}
		}

		inline void newline()
		{
			m_stream << '\n';
		}

		inline void append( std::unique_ptr<Append::CBaseAppend> ptr )
		{
			ptr.get()->append( m_stream, m_current_enclosure_counter );
		}

		template <HandlerType_t type>
		void nest( const char *name, const std::function<void( CStream & )> &thunk )
		{
			for( auto i = 0u; i < m_current_enclosure_counter; ++i )
			{
				m_stream << '\t';
			}

			if( language == ELanguage::C )
			{
				if( type == Nesting::C::NamedNestType_t::STRUCT )
				{
				#if INLINE_STRUCT_OPENING_BRACKET == true
				#if FORMAT_DELIMIT_INLINE_SYNTAX == true
					m_stream << "struct " << name << " {" FORMAT_DELIMITER
					#if EXTRA_ENDLINE_AFTER_NAMESPACE_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#else
					m_stream << "struct " << name << " {"
					#if EXTRA_ENDLINE_AFTER_STRUCT_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#endif
				#else
					m_stream << "struct " << name << '\n';

					for( auto i = 0u; i < m_current_enclosure_counter; ++i )
					{
						m_stream << '\t';
					}

					m_stream << "{\n"
					#if EXTRA_ENDLINE_AFTER_STRUCT_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#endif
					++m_current_enclosure_counter;
				}
			}

			else if( language == ELanguage::CPP )
			{
				if( type == Nesting::CPP::NamedNestType_t::STRUCT )
				{
				#if INLINE_STRUCT_OPENING_BRACKET == true
				#if FORMAT_DELIMIT_INLINE_SYNTAX == true
					m_stream << "struct " << name << " {" FORMAT_DELIMITER
					#if EXTRA_ENDLINE_AFTER_NAMESPACE_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#else
					m_stream << "struct " << name << " {"
					#if EXTRA_ENDLINE_AFTER_STRUCT_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#endif
				#else
					m_stream << "struct " << name << '\n';

					for( auto i = 0u; i < m_current_enclosure_counter; ++i )
					{
						m_stream << '\t';
					}

					m_stream << "{\n"
					#if EXTRA_ENDLINE_AFTER_STRUCT_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#endif
					++m_current_enclosure_counter;
				}


				if( type == Nesting::CPP::NamedNestType_t::NAMESPACE )
				{
				#if INLINE_NAMESPACE_OPENING_BRACKET == true
				#if FORMAT_DELIMIT_INLINE_SYNTAX == true
					m_stream << "namespace " << name << " {" FORMAT_DELIMITER
					#if EXTRA_ENDLINE_AFTER_NAMESPACE_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#else
					m_stream << "namespace " << name << " {"
					#if EXTRA_ENDLINE_AFTER_NAMESPACE_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#endif
				#else
					m_stream << "namespace " << name << '\n';

					for( auto i = 0u; i < m_current_enclosure_counter; ++i )
					{
						m_stream << '\t';
					}

					m_stream << "{\n"
					#if EXTRA_ENDLINE_AFTER_NAMESPACE_OPENING_BRACKET == true
						"\n";
				#else
						;
				#endif
				#endif
					++m_current_enclosure_counter;
				}
			}

			thunk( *this );

			if( language == ELanguage::CPP )
			{
				if( type == Nesting::CPP::NamedNestType_t::NAMESPACE )
				{
					if( m_current_enclosure_counter > 0 )
					{
						--m_current_enclosure_counter;
						for( auto i = 0u; i < m_current_enclosure_counter; ++i )
						{
							m_stream << '\t';
						}

						m_stream << "}\n"
						#if EXTRA_ENDLINE_AFTER_NAMESPACE_CLOSING_BRACKET == true
							"\n";
					#endif
						;
					}
					else
					{
						throw std::runtime_error( "Attempted to close unopened closure (Namespace)" );
					}
				}

				else if( type == Nesting::CPP::NamedNestType_t::STRUCT )
				{
					if( m_current_enclosure_counter > 0 )
					{
						--m_current_enclosure_counter;
						for( auto i = 0u; i < m_current_enclosure_counter; ++i )
						{
							m_stream << '\t';
						}

						m_stream << "};\n"
						#if EXTRA_ENDLINE_AFTER_STRUCT_CLOSING_BRACKET == true
							"\n";
					#endif
						;
					}
					else
					{
						throw std::runtime_error( "Attempted to close unopened closure (Namespace)" );
					}
				}

			}

			else if( language == ELanguage::C )
			{
				if( type == Nesting::C::NamedNestType_t::STRUCT )
				{
					if( m_current_enclosure_counter > 0 )
					{
						--m_current_enclosure_counter;
						for( auto i = 0u; i < m_current_enclosure_counter; ++i )
						{
							m_stream << '\t';
						}

						m_stream << "};\n"
						#if EXTRA_ENDLINE_AFTER_STRUCT_CLOSING_BRACKET == true
							"\n";
					#endif
						;
					}
					else
					{
						throw std::runtime_error( "Attempted to close unopened closure (Namespace)" );
					}
				}
			}
		}

		template <HandlerType_t type>
		void nest( const std::function<void( CStream & )> &thunk )
		{
			for( auto i = 0u; i < m_current_enclosure_counter; ++i )
			{
				m_stream << '\t';
			}

			if( language == ELanguage::CPP ||
				language == ELanguage::C )
			{
				if( type == Nesting::C::UnnamedNestType_t::CONTROL_PATH ||
					type == Nesting::CPP::UnnamedNestType_t::CONTROL_PATH )
				{
				#if INLINE_CONTROL_PATH_OPENING_BRACKET == true
				#if FORMAT_DELIMIT_INLINE_SYNTAX == true
					m_stream << "{" FORMAT_DELIMITER;
				#else
					m_stream << '{';
				#endif
				#else
				#if EXTRA_ENDLINE_AFTER_CONTROL_PATH_OPENING_BRACKET == true
					m_stream << "{\n\n";
				#else
					m_stream << "{\n";
				#endif
				#endif
					++m_current_enclosure_counter;
				}
			}

			thunk( *this );

			if( language == ELanguage::CPP ||
				language == ELanguage::C )
			{
				if( type == Nesting::C::UnnamedNestType_t::CONTROL_PATH ||
					type == Nesting::CPP::UnnamedNestType_t::CONTROL_PATH )
				{
					m_stream << '\n';

					--m_current_enclosure_counter;

					if( m_current_enclosure_counter > 0 )
					{
						for( auto i = 0u; i < m_current_enclosure_counter; ++i )
						{
							m_stream << '\t';
						}
					}

				#if EXTRA_ENDLINE_AFTER_CONTROL_PATH_CLOSING_BRACKET == true

					m_stream << "}\n"
					#else
					m_stream << '}'
					#endif
						;
				}
				else
				{
					throw std::runtime_error( "Attempted to close unopened closure (ControlPath)" );
				}
			}
		}

		private:
		const char *m_file_name = nullptr;
		unsigned int m_current_enclosure_counter = 0;
		std::ofstream m_stream = {};
	};
}