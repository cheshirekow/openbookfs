/*
 *  Copyright (C) 2012 Josh Bialkowski (jbialk@mit.edu)
 *
 *  This file is part of openbook.
 *
 *  openbook is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openbook is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openbook.  If not, see <http://www.gnu.org/licenses/>.
 */
/**
 *  @file   src/ExceptionStream.h
 *
 *  @date   Feb 11, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_EXCEPTIONSTREAM_H_
#define OPENBOOK_EXCEPTIONSTREAM_H_

#include <exception>
#include <stdexcept>
#include <sstream>

namespace   openbook {
namespace filesystem {


/// used to simplify the process of generating an exception message
/**
 *  Derives from stringstream so provides an ostream interface, but throws
 *  an exception with the contents of the string when the object is destroyed
 *
 *  \tparam Exception_t must be an exception type which accepts a
 *                      const char* in it's constructor
 *
 */
template <typename Exception_t>
class ExceptionStream :
    public std::stringstream
{
    public:
        ~ExceptionStream()
        {
            throw Exception_t( str().c_str() );
        }

        std::ostream& operator()()
        {
            return *this;
        }
};


/// an exception which carries a standard error code
class CodedException :
    public std::exception
{
    private:
        int         m_code;
        std::string m_message;

    public:
        CodedException( int code, const std::string& msg ):
            m_code(code),
            m_message(msg)
        {}

        virtual ~CodedException() throw() {}

        int code(){ return m_code; }

        virtual const char* what() const throw()
        {
            return m_message.c_str();
        }
};

/// used to simplify the process of generating an exception message
class CodedExceptionStream :
    public std::stringstream
{
    private:
        int     m_error;

    public:
        CodedExceptionStream( int error ):
            m_error(error)
        {}

        ~CodedExceptionStream()
        {
            throw CodedException( m_error, str().c_str() );
        }

        std::ostream& operator()()
        {
            return *this;
        }
};




typedef ExceptionStream<std::runtime_error> ex;
typedef CodedExceptionStream codedExcept;


} // namespace filesystem
} // namespace openbook




#endif // EXCEPTIONSTREAM_H_
