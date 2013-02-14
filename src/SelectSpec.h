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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/SelectSpec.h
 *
 *  @date   Feb 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_SELECTSPEC_H_
#define OPENBOOK_SELECTSPEC_H_

#include <vector>
#include "FdSet.h"
#include "TimeVal.h"


namespace   openbook {
namespace filesystem {

/// enumerators for SelectSpec
namespace select_spec
{
    /// just some constants we can use for the three sets
    enum Which
    {
        READ    =0,
        WRITE   =1,
        EXCEPT  =2,
        NUM_WHICH
    };
}

/// stores a specification for "select" and restores it after calling
/// "select"
class SelectSpec
{
    public:
        typedef select_spec::Which Which;

        /// stores elements of the spec
        struct Elmnt
        {
            int   fd;
            Which which;

            Elmnt( int fd_in=0, Which which_in=select_spec::READ ):
                fd(fd_in),
                which(which_in)
            {}
        };

        /// used to generate a spec in a convenient way
        class Generator
        {
            private:
                friend class SelectSpec;
                SelectSpec* m_spec;

            public:
                Generator( SelectSpec* spec );

                /// adds a file descriptor to the spec
                Generator& operator()( int fd, Which which );

                /// sets the timeout for the spec
                Generator& operator()( const TimeVal& to );
        };

    private:
        std::vector<Elmnt>  m_spec;
        TimeVal             m_timeout;
        TimeVal             m_remainder;
        int                 m_maxfd;
        FdSet               m_fdset[select_spec::NUM_WHICH];

    public:
        SelectSpec();

        /// clears the spec
        void reset();

        /// add an element to the spec
        void add( int fd, Which which );

        /// set the timeout of the spec
        void setTimeout( const TimeVal& to );

        /// call select()
        int wait();

        /// returns whether the specified fd is ready for the requested
        /// operation
        bool ready( int fd, Which which );

        /// returns a generator for this spec
        Generator gen();



};



} // namespace filesystem
} // namespace openbook













#endif // SELECTSPEC_H_
