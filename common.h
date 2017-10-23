#ifndef COMMON_H
#define COMMON_H

#include <iostream>

template<typename T>
void log_info( const T& val )
{
	std::cout << "I: " << val << std::endl;
}

template<typename T>
void log_warning( const T& val )
{
	std::cout << "W: " << val << std::endl;
}

template<typename T>
void log_error( const T& val )
{
	std::cerr << "E: " << val << std::endl;
}

template<typename T>
void log_debug( const T& val )
{
	std::cout << "D: " << val << std::endl;
}

template<typename T>
void safe_delete( T*& ptr )
{
	if( ptr != nullptr )
	{
		delete ptr;
		ptr = nullptr;
	}
}

template<typename PAY, typename RET, typename... PAR>
class PayloadCallback
{
public:
	typedef RET ( *FunctionPtr )( PAR..., PAY );

public:
	PayloadCallback() = default;

	PayloadCallback( FunctionPtr fun, PAY payload )
	    : m_pCallback( fun ),
	      m_Payload( payload )
	{
	}

	RET operator()( PAR... par )
	{
		return m_pCallback( par..., m_Payload );
	}

	operator bool() const
	{
		return ( m_pCallback != nullptr );
	}

private:
	FunctionPtr m_pCallback;
	PAY         m_Payload;
};

#endif // COMMON_H
