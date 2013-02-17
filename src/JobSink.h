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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/client_fs/JobSink.h
 *
 *  @date   Feb 17, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_JOBSINK_H_
#define OPENBOOK_JOBSINK_H_


namespace   openbook {
namespace filesystem {

class Job;

/// base class for something that wants to be notified when
/// a job is complete
class JobSink
{
    public:
        /// jobsinks have a v-table
        virtual ~JobSink(){}

        /// handles a completed job, is reponsible for destroying the
        /// job object
        virtual void jobFinished(Job*)=0;
};




} // namespace filesystem
} // namespace openbook



#endif // JOBSINK_H_
