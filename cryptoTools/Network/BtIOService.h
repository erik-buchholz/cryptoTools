#pragma once
// This file and the associated implementation has been placed in the public domain, waiving all copyright. No restrictions are placed on its use. 
#include <cryptoTools/Common/Defines.h>
# if defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
#  error WinSock.h has already been included. Please move the boost headers above the WinNet*.h headers
# endif // defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)

#include <thread> 

#ifndef _MSC_VER
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <boost/asio.hpp>
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
#include <mutex>
#include <list> 
#include <future>
#include <string>

namespace osuCrypto
{
    class NetworkError : public std::exception
    {
    public:
        std::string mWhat;
        NetworkError(std::string what)
            :mWhat(what)
        {
        }
    };

    class BadReceiveBufferSize : public std::exception
    {
    public:
        std::string mWhat;
        u64 mSize;
        std::function<void(u8*)> mRescheduler;

        BadReceiveBufferSize(std::string what, u64 length, std::function<void(u8*)> rescheduler)
            :
            mWhat(std::move(what)),
            mSize(length),
            mRescheduler(std::move(rescheduler))
        { }

        BadReceiveBufferSize(const BadReceiveBufferSize& src) = default;
        BadReceiveBufferSize(BadReceiveBufferSize&& src) = default;
    };


    class BtAcceptor;
    struct BtIOOperation;
    class BtEndpoint;
    class Channel;
    //class BtSocket;

    std::vector<std::string> split(const std::string &s, char delim);

    class BtIOService
    {
        friend class Channel;
        friend class BtEndpoint;

    public:

        BtIOService(const BtIOService&) = delete;

        /// <summary> Constructor for the IO service that services network IO operations.</summary>
        /// <param name="threadCount">The number of threads that should be used to service IO operations. 0 = use # of CPU cores.</param>
        BtIOService(u64 threadCount = 0);
        ~BtIOService();

        /// /// <summary> This is a Windows specific object that is used to queue up pending network IO operations.</summary>
        boost::asio::io_service mIoService;

        std::unique_ptr<boost::asio::io_service::work> mWorker;


        /// <summary> This list hold the threads that send and recv messages. </summary>
        std::list<std::thread> mWorkerThrds;

        /// <summary> The list of acceptor objects that hold state about the ports that are being listened to. </summary>
        std::list<BtAcceptor> mAcceptors;

        void printErrorMessages(bool v);

        /// <summary> indicates whether stop() has been called already.</summary>
        bool mStopped;

        /// <summary> The mutex the protects sensitive objects in this class. </summary>
        std::mutex mMtx;

        /// <summary> A list containing futures for the endpoint that use this IO service. Each is fulfilled when the endpoint is finished with this class.</summary>
        std::list<std::shared_future<void>> mEndpointStopFutures;

        void receiveOne(Channel* socket);

        void sendOne(Channel* socket);

        void startSocket(Channel* chl);

        /// <summary> Used to queue up asynchronous socket operations.</summary>
        /// <param name="socket">The socket that is being operated on.</param>
        /// <param name="op">The operation that should be queued up. </param>
        void dispatch(Channel* socket, BtIOOperation& op);

        /// <summary> Gives a new endpoint which is a host endpoint the acceptor which provides sockets. 
        /// Needed since multiple endpoints with different names may listen on a single port.</summary>
        /// <param name="endpoint">The new Endpoint that needs its acceptor.</param>
        BtAcceptor* getAcceptor(BtEndpoint& endpoint);

        /// <summary> Shut down the IO service. WARNING: blocks until all Channels and Endpoints are stopped.</summary>
        void stop();


        bool mPrint;

    };

}