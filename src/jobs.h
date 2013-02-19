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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/server/jobs.h
 *
 *  @date   Feb 15, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_JOBS_H_
#define OPENBOOK_JOBS_H_



namespace   openbook {
namespace filesystem {
namespace       jobs {

enum JobClass
{
    QUIT_SHOUTER=0,     ///< special job telling shouter to quit
    QUIT_WORKER,        ///< special job telling job handlers to quit
    NEW_CLIENT_VERSION, ///< new version of a file on the client
    NEW_SERVER_VERSION, ///< new version of a file on the server
    NUM_JOB_CLASSES
};


} // namespace jobs

typedef jobs::JobClass JobClass;

const char* jobIdToString( JobClass id );

} // namespace filesystem
} // namespace openbook



#endif // JOBS_H_
