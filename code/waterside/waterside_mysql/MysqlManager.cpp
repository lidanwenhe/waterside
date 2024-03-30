#include "MysqlManager.h"
#include "MysqlException.h"
//#include "PacketWriter.h"

namespace waterside
{
	MysqlManager::MysqlManager(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond)
		: mUrl(url)
		, mReconnectDelaySecond(reconnectDelaySecond)
		, mPool(multithreading)
	{
	}

	bool MysqlManager::initialize()
	{
		for (std::size_t i = 0; i < mPool.getPoolSize(); i++)
		{
			try
			{
				auto pSession = std::make_shared<MysqlSession>();
				onRegisterStmt(pSession.get());
				pSession->initialize(mUrl);
				pSession->connect();

				mpMysqlSessions.emplace_back(pSession);

				MLOG_INFO(MYSQL, "mysql url={} connect ok"sv, mUrl);
			}
			catch (const MysqlException& e)
			{
				MLOG_ERROR(MYSQL, "mysql url={} create thread exception:{}"sv, mUrl, e.what());

				mpMysqlSessions.clear();
				return false;
			}
			catch (const std::exception& e)
			{
				MLOG_ERROR(MYSQL, "mysql url={} create thread c++ exception:{}"sv, mUrl, e.what());

				mpMysqlSessions.clear();
				return false;
			}
			catch (...)
			{
				MLOG_ERROR(MYSQL, "mysql url={} create thread unknown exception"sv, mUrl);

				mpMysqlSessions.clear();
				return false;
			}
		}

		mThread = std::thread([&] { mPool.run(); });
		return true;
	}

	void MysqlManager::release()
	{
		mPool.stop();
		if (mThread.joinable())
			mThread.join();
	}

	std::string_view MysqlManager::getFunctionName(uint32_t key)
	{
		if (auto it = mFuncId2Name.find(key); it != mFuncId2Name.end())
			return it->second;
		else
			return {};
	}

	void MysqlManager::post(std::shared_ptr<MysqlContext> pContext)
	{
		if (mPool.isStopped())
			return;

		auto& ctx = mPool.getIoContext();
		boost::asio::post(ctx,
			[this, pContext]()
			{
				proccess(pContext);
			}
		);
	}

	MysqlManager::DispatchHandler* MysqlManager::getDispatchHandler(uint32_t funcid)
	{
		if (auto it = mDispatchHandlers.find(funcid); it != mDispatchHandlers.end())
		{
			return &it->second;
		}
		return nullptr;
	}

	MysqlSession* MysqlManager::getMysqlSession()
	{
		std::lock_guard<std::mutex> lock(mMutex);

		if (mNextMysqlSession < mpMysqlSessions.size())
		{
			return mpMysqlSessions[mNextMysqlSession++].get();
		}
		return nullptr;
	}

	void MysqlManager::reconnect(MysqlSession* pSession)
	{
		while (!mPool.isStopped())
		{
			try
			{
				if (pSession->ping())
				{
					break;
				}

				pSession->connect();
				MLOG_DEBUG(MYSQL, "mysql url={} thread reconnect"sv, mUrl);
				break;
			}
			catch (const MysqlException& e)
			{
				MLOG_ERROR(MYSQL, "mysql url={} reconnect exception:{}"sv, mUrl, e.what());
			}
			catch (const std::exception& e)
			{
				MLOG_ERROR(MYSQL, "mysql url={} reconnect c++ exception:{}"sv, mUrl, e.what());
			}
			catch (...)
			{
				MLOG_ERROR(MYSQL, "mysql url={} reconnect unknown exception"sv, mUrl);
			}

			std::this_thread::sleep_for(std::chrono::seconds(mReconnectDelaySecond));
		}
	}

	void MysqlManager::proccess(std::shared_ptr<MysqlContext> pContext)
	{
		thread_local MysqlSession* pMysqlSession = nullptr;
		if (!pMysqlSession)
		{
			pMysqlSession = getMysqlSession();
			assert(pMysqlSession);
		}

		// 断线重连
		reconnect(pMysqlSession);

		for (int i = 0; i < 2; i++)
		{
			try
			{
				onDispatch(pMysqlSession, pContext);
			}
			catch (const MysqlException& e)
			{
				if (i == 0 && (e.geterrno() == CR_SERVER_LOST || e.geterrno() == CR_SERVER_GONE_ERROR))
				{// 断线重连
					reconnect(pMysqlSession);
					continue;
				}

				MLOG_ERROR(MYSQL, "mysql url={} exception:{}"sv, mUrl, e.what());
			}
			catch (const std::exception& e)
			{
				const boost::stacktrace::stacktrace* st = boost::get_error_info<traced>(e);
				if (st)
				{
					//打印堆栈
					std::stringstream ss;
					ss << e.what() << std::endl << *st;

					MLOG_ERROR(MYSQL, "mysql url={} c++ exception:{}"sv, mUrl, ss.str());
				}
				else
				{
					MLOG_ERROR(MYSQL, "mysql url={} c++ exception:{}"sv, mUrl, e.what());
				}

			}
			catch (...)
			{
				MLOG_ERROR(MYSQL, "mysql url={} c++ unknown exception", mUrl);
			}

			break;
		}
	}

	void MysqlManager::onDispatch(MysqlSession* pSession, std::shared_ptr<MysqlContext> pContext)
	{
		std::optional<std::vector<char>> res;
		auto funcid = pContext->header.function_id();
		try
		{
			auto handler = getDispatchHandler(funcid);
			if (handler)
			{
				res = (*handler)(pSession, pContext);
			}
			else
			{
				MLOG_ERROR(MYSQL, "function {} not registered!", getFunctionName(funcid));
			}
		}
		catch (const MysqlException& e)
		{
			MLOG_ERROR(MYSQL, "function {} mysql exception:{}", getFunctionName(funcid), e.what());
		}
		catch (const std::exception& e)
		{
			MLOG_ERROR(MYSQL, "function {} exception:{}", getFunctionName(funcid), e.what());
		}
		catch (...)
		{
			MLOG_ERROR(MYSQL, "function {} unknown exception", getFunctionName(funcid));
		}

		if (res.has_value())
		{
			pContext->body = std::move(res.value());
			mProcessPackets.push(pContext);
		}
	}
	
	void MysqlManager::onProcessPacket()
	{
		if (!mProcessPackets.empty())
		{
			std::shared_ptr<MysqlContext> pContext;
			while (mProcessPackets.try_pop(pContext))
			{
				if (auto it = mPromises.find(pContext->header.seq_num()); it != mPromises.end())
				{
					it->second->promise.setValue(std::optional<std::vector<char>>{pContext->body});
				}
			}
		}
	}
}
