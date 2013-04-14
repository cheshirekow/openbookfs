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
 *  @file   /home/josh/Codes/cpp/openbookfs/src/backend/MessageHandler.h
 *
 *  @date   Apr 14, 2013
 *  @author Josh Bialkowski (jbialk@mit.edu)
 *  @brief  
 */

#ifndef OPENBOOK_FS_MESSAGEHANDLER_H_
#define OPENBOOK_FS_MESSAGEHANDLER_H_

#include <cpp-pthreads.h>

#include "Pool.h"
#include "Marshall.h"
#include "Queue.h"

namespace   openbook {
namespace filesystem {

// forward dec
class Backend;

// /// throws an exception for any messages which are part of the handshake
// /// protocol
//template <typename ...MessageList>
//struct HandshakeHandler;
//
//template <typename First, typename... Rest>
//struct HandshakeHandler<First,Rest...>:
//    public HandshakeHandler<First>,
//    public HandshakeHandler<Rest...>
//{};
//
//template <typename Message_t>
//struct HandshakeHandler<Message_t>
//{
//    void handleMessage( Message_t* msg )
//    {
//        ex()() << "Protocol Error: MessageHandler " << (void*)this
//               << " received handshake message "
//               << messageIdToString( MessageTypeToId<Message_t>::ID )
//               << " during worker loop";
//    }
//};

/// reads a message and does whatever the message says to do
class MessageHandler
//    public HandshakeHandler<messages::LeaderElect,
//                            messages::DiffieHellmanParams,
//                            messages::KeyExchange,
//                            messages::ContentKey,
//                            messages::AuthRequest,
//                            messages::AuthChallenge,
//                            messages::AuthSolution,
//                            messages::AuthResult>
{
    public:
        typedef Pool<MessageHandler> Pool_t;
        typedef RefPtr<AutoMessage>  MsgPtr_t;
        typedef Queue< MsgPtr_t >    MsgQueue_t;

    private:
        Backend*            m_backend;          ///< top-level object
        Pool_t*             m_pool;             ///< pool this object came from
        MsgQueue_t*         m_inboundQueue;     ///< messages to process
        MsgQueue_t*         m_outboundQueue;    ///< where we put messages
        pthreads::Mutex     m_mutex;            ///< locks this data
        bool                m_shouldQuit;       ///< main loop termination

    public:
        MessageHandler();
        ~MessageHandler();

        /// initialize with backend pointer
        void init( Backend*, Pool_t* );

        /// set the parent pointer and start DH parameter generation in
        /// detached thread
        void go( pthreads::Thread& thread, MsgQueue_t* in, MsgQueue_t* out );

        /// returns this handler to the pool
        void returnToPool();

        /// main method of the job handler, waits for jobs in the queue and
        /// then does them
        void main();

        /// static method for pthreads, calls main()
        static void* dispatch_main( void* vp_h );

        /// for messages we dont expect to recieve
        template <typename Message_t>
        void exceptMessage( Message_t* msg )
        {
            ex()() << "Protocol Error: MessageHandler " << (void*)this
               << " received handshake message "
               << messageIdToString( MessageTypeToId<Message_t>::ID )
               << " during worker loop";
        }

        /// typed message  handlers
        void handleMessage( messages::Quit*         msg );
        void handleMessage( messages::Ping*         msg );
        void handleMessage( messages::Pong*         msg );
        void handleMessage( messages::LeaderElect*  msg );
        void handleMessage( messages::DiffieHellmanParams* msg );
        void handleMessage( messages::KeyExchange*      msg );
        void handleMessage( messages::ContentKey*       msg );
        void handleMessage( messages::AuthRequest*      msg );
        void handleMessage( messages::AuthChallenge*    msg );
        void handleMessage( messages::AuthSolution*     msg );
        void handleMessage( messages::AuthResult*       msg );
        void handleMessage( messages::Subscribe*    msg );
        void handleMessage( messages::Unsubscribe*  msg );
        void handleMessage( messages::NewVersion*   msg );
        void handleMessage( messages::RequestFile*  msg );
        void handleMessage( messages::FileInfo*     msg );
        void handleMessage( messages::FileChunk*    msg );
        void handleMessage( messages::DirInfo*      msg );
        void handleMessage( messages::DirChunk*     msg );
        void handleMessage( messages::Invalid*      msg );
};

template <int NA, int NC>
struct MessageDispatcher
{
    typedef RefPtr<AutoMessage> MsgPtr_t;

    static const int        NB  = (NA+NC)/2;
    static const MessageId  MID = (MessageId)NB;

    static void dispatch( MessageHandler* handler, MsgPtr_t msg )
    {
        if( msg->type < NB )
            MessageDispatcher<NA,NB>::dispatch(handler,msg);
        else if( msg->type > NB )
            MessageDispatcher<NB,NC>::dispatch(handler,msg);
        else
        {
            typedef typename MessageType<MID>::type DownType;
            DownType* downcast = message_cast<MID>(msg->msg);
            handler->handleMessage(downcast);
        }
    }
};



} // namespace filesystem
} // namespace openbook


#endif // MESSAGEHANDLER_H_