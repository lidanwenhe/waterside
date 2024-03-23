#include "NetworkBase.h"

namespace waterside
{
    void NetworkBase::cleanSession(SessionID sessionId)
    {
        mSessions.erase(sessionId);
        mSessionIdManager.recyclingId(sessionId);
    }

    std::shared_ptr<SessionBase> NetworkBase::findSession(SessionID sessionId)
    {
        tbb::concurrent_hash_map<SessionID, std::shared_ptr<SessionBase>>::const_accessor acc;
        if (mSessions.find(acc, sessionId))
            return acc->second;
        return nullptr;
    }

    void NetworkBase::addProcessPacket(std::shared_ptr<NetworkContext>& pContext)
    {
        mProcessPackets.emplace(pContext);
    }

    void NetworkBase::addSession(std::shared_ptr<SessionBase> pSession)
    {
        mSessions.insert(std::make_pair(pSession->getSessionId(), pSession));
    }

    void NetworkBase::closeAllSession()
    {
        vector<std::shared_ptr<SessionBase>> vec;
        for (auto& item : mSessions)
        {
            vec.emplace_back(item.second);
        }
        for (auto& p : vec)
        {
            p->asyncCloseSocket();
        }
    }
    
    string_view NetworkBase::getFunctionName(uint32_t key)
    {
        if (auto it = mFuncId2Name.find(key); it != mFuncId2Name.end())
            return it->second;
        else
            return {};
    }

    void NetworkBase::onProcessPacket()
    {
        if (!mProcessPackets.empty())
        {
            std::shared_ptr<NetworkContext> p;
            while (mProcessPackets.try_pop(p))
            {
                onDispatch(p).start([](async_simple::Try<void> Result) {
                    if (Result.hasError())
                    {
                        std::exception_ptr error = Result.getException();
                        try
                        {
                            std::rethrow_exception(error);
                        }
                        catch (const std::exception& e)
                        {
                            LOG_ERROR("Error Happened in task. {}", e.what());
                        }
                    }
                    });
            }
        }
    }

    NetworkBase::DispatchHandler* NetworkBase::getDispatchHandler(uint32_t funcid)
    {
        if (auto it = mDispatchHandlers.find(funcid); it != mDispatchHandlers.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    NetworkBase::CoroDispatchHandler* NetworkBase::getCoroDispatchHandler(uint32_t funcid)
    {
        if (auto it = mCoroDispatchHandlers.find(funcid); it != mCoroDispatchHandlers.end())
        {
            return &it->second;
        }
        return nullptr;
    }

    async_simple::coro::Lazy<void> NetworkBase::onDispatch(std::shared_ptr<NetworkContext> pContext)
    {
        switch (pContext->header.packet_type())
        {
        case MESSAGE_PACKET_TYPE_DISCONNECT:
            if (onDisconnectCallback)
            {
                onDisconnectCallback(pContext->sessionId);
            }
            break;
        case MESSAGE_PACKET_TYPE_RPC_REQUEST:
        {
            std::optional<vector<char>> res;
            auto funcid = pContext->header.function_id();
            try
            {
                auto handler = getDispatchHandler(funcid);
                if (handler)
                {
                    res = (*handler)(pContext);
                }
                else
                {
                    auto coro_handler = getCoroDispatchHandler(funcid);
                    if (coro_handler)
                    {
                        res = co_await (*coro_handler)(pContext);
                    }
                    else
                    {
                        MLOG_ERROR(NET, "function {} not registered!", getFunctionName(funcid));
                    }
                }
            }
            catch (const std::exception& e)
            {
                MLOG_ERROR(NET, "function {} exception:{}", getFunctionName(funcid), e.what());
            }
            catch (...)
            {
                MLOG_ERROR(NET, "function {} unknown exception", getFunctionName(funcid));
            }

            if (res.has_value())
            {
                auto pSession = findSession(pContext->sessionId);
                if (pSession)
                {
                    auto& buffer = res.value();

                    RpcPacketHeader header(MESSAGE_PACKET_TYPE_RPC_RESPONSE,
                        SERIALIZATION_TYPE_STRUCT_PACK,
                        buffer.size(),
                        pContext->header.seq_num(),
                        pContext->header.function_id()
                    );

                    pSession->send(header, std::pair<const void*, size_t>{buffer.data(), buffer.size()});
                }
            }
        }
        break;
        case MESSAGE_PACKET_TYPE_RPC_RESPONSE:
        {
            if (auto it = mPromises.find(pContext->header.seq_num()); it != mPromises.end())
            {
                it->second->promise.setValue(std::optional<vector<char>>{pContext->body});
            }
        }
        break;
        default:
            break;
        }
    }
}