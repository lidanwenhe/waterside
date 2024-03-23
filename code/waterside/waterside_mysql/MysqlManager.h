#pragma once

#include "IoContextPool.h"
#include "rpc_protocol_generated.h"
#include "MysqlSession.h"
#include "async_simple/coro/FutureAwaiter.h"
#include "ylt/struct_pack.hpp"

namespace waterside
{
	struct MysqlContext
	{
		RpcPacketHeader header;
		vector<char> body;
	};

	class MysqlManager
	{
	public:
		MysqlManager(string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond);

		virtual ~MysqlManager() = default;

		virtual bool initialize();

		virtual void release();

		template <auto func>
		void registHandler()
		{
			using T = decltype(func);
			using param_type = coro_rpc::function_parameters_t<T>;
			using return_type = coro_rpc::function_return_type_t<T>;

			constexpr auto key = coro_rpc::func_id<func>();
			constexpr auto name = coro_rpc::get_func_name<func>();

			auto it = mDispatchHandlers.emplace(
				key,
				std::bind(&MysqlManager::mysqlDispatcher<func>, this, std::placeholders::_1, std::placeholders::_2));
			if (!it.second)
			{
				MLOG_CRITICAL(NET, "duplication function {} register!", name);
			}

			mFuncId2Name.emplace(key, name);
		}

		template <auto func, typename... Args>
		async_simple::coro::Lazy<std::optional<decltype(getReturnType<func>())>>
			query(Args... args)
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			using return_type = typename get_type_t<coro_rpc::function_return_type_t<Function>>::type;

			//static_check<func, Args...>();

			auto buffer = prepareBuffer<func>(std::move(args)...);

			auto seqnum = getSeqNum();
			RpcPacketHeader header(MESSAGE_PACKET_TYPE_MYSQL,
				(buffer.empty() ? SERIALIZATION_TYPE_NO : SERIALIZATION_TYPE_STRUCT_PACK),
				0,
				seqnum,
				coro_rpc::func_id<func>()
			);

			post(std::make_shared<MysqlContext>(header, std::move(buffer)));

			if constexpr (!std::is_void_v<return_type>)
			{
				auto pr = std::make_shared<PromiseInfo>();
				mPromises[seqnum] = pr;

				auto fut = pr->promise.getFuture();
				auto val = co_await std::move(fut);

				if (val.has_value())
				{
					return_type ret{};
					struct_pack::errc ok{};
					ok = struct_pack::deserialize_to(ret, val.value());

					if (ok != struct_pack::errc::ok)
					{
						MLOG_WARN(MYSQL, "struct_pack::deserialize_to error:{}, function:{}",
							struct_pack::detail::category().message((int)ok),
							coro_rpc::get_func_name<func>());
					}
					else
					{
						co_return ret;
					}
				}
			}
			co_return std::nullopt;
		}

		void onProcessPacket();

	protected:
		uint32_t getSeqNum()
		{
			return ++mSeqNum;
		}

		string_view getFunctionName(uint32_t key);


		using DispatchHandler = std::function<std::optional<vector<char>>(
			MysqlSession* pSession, std::shared_ptr<MysqlContext>& pContext)>;

		template <typename Function>
		constexpr static bool isMemberFunction()
		{
			if constexpr (std::is_member_function_pointer_v<Function>)
			{
				using Self = coro_rpc::class_type_t<Function>;
				static_assert(std::is_base_of_v<MysqlManager, Self>, "must derived from MysqlManager");
				return true;
			}
			else
			{
				return false;
			}
		}

		template <typename T>
		constexpr static auto getArgs()
		{
			using args_type = coro_rpc::remove_first_t<T>;
			return args_type{};
		}

		template <typename... FuncArgs, typename Buffer, typename... Args>
		void pack_to_impl(Buffer& buffer, Args &&...args) {
			struct_pack::serialize_to(
				buffer, std::forward<FuncArgs>(std::forward<Args>(args))...);
		}

		template <typename Tuple, size_t... Is, typename Buffer, typename... Args>
		void pack_to_helper(std::index_sequence<Is...>, Buffer& buffer,
			Args &&...args) {
			pack_to_impl<std::tuple_element_t<Is, Tuple>...>(
				buffer, std::forward<Args>(args)...);
		}

		template <typename FuncArgs, typename Buffer, typename... Args>
		void pack_to(Buffer& buffer, Args &&...args) {
			pack_to_helper<FuncArgs>(
				std::make_index_sequence<std::tuple_size_v<FuncArgs>>{}, buffer,
				std::forward<Args>(args)...);
		}

		template <auto func, typename... Args>
		std::vector<char> prepareBuffer(Args&&... args)
		{
			std::vector<char> buffer;

			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;

			if constexpr (sizeof...(Args) > 0)
			{
				auto _args = getArgs<param_type>();
				pack_to<decltype(_args)>(buffer, std::forward<Args>(args)...);
			}
			return buffer;
		}

		template <typename T>
		bool packetDeserialize(T& args, std::shared_ptr<MysqlContext>& pContext)
		{
			constexpr size_t size = std::tuple_size_v<T>;
			if constexpr (size > 0)
			{
				if (pContext->header.serialize_type() != SERIALIZATION_TYPE_STRUCT_PACK)
				{
					MLOG_WARN(MYSQL, "serialize_type must STRUCT_PACK type:{}, function:{}", EnumNameSERIALIZATION_TYPE(pContext->header.serialize_type()), getFunctionName(pContext->header.function_id()));
					return false;
				}

				struct_pack::errc ok{};
				if constexpr (std::tuple_size_v<T> == 1)
				{
					ok = struct_pack::deserialize_to(std::get<0>(args), pContext->body);
				}
				else
				{
					ok = struct_pack::deserialize_to(args, pContext->body);
				}

				if (ok != struct_pack::errc::ok)
				{
					MLOG_WARN(MYSQL, "struct_pack::deserialize_to error:{}, function:{}", struct_pack::detail::category().message((int)ok), getFunctionName(pContext->header.function_id()));
					return false;
				}
			}
			else
			{
				if (pContext->header.serialize_type() != SERIALIZATION_TYPE_NO)
				{
					MLOG_WARN(MYSQL, "args size=0 type:{}, function:{}", EnumNameSERIALIZATION_TYPE(pContext->header.serialize_type()), getFunctionName(pContext->header.function_id()));
					return false;
				}
			}
			return true;
		}

		template<auto func>
		std::optional<vector<char>> mysqlDispatcher(MysqlSession* pSession, std::shared_ptr<MysqlContext>& pContext)
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			using return_type = coro_rpc::function_return_type_t<Function>;

			constexpr bool bMemberFunction = isMemberFunction<Function>();

			if constexpr (!std::is_void_v<param_type>)
			{
				auto args = getArgs<param_type>();

				if (!packetDeserialize(args, pContext))
				{
					return std::nullopt;
				}

				if constexpr (bMemberFunction)
				{
					if constexpr (std::is_void_v<return_type>)
					{
						// call void o.func(pSession, args...)
						std::apply(func,
							std::tuple_cat(
								std::forward_as_tuple(
									*this,
									pSession
								),
								std::move(args))
						);
					}
					else
					{// call return_type o.func(pContext, args...)
						return struct_pack::serialize(
							std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*this,
										pSession
									),
									std::move(args))
							)
						);
					}
				}
				else
				{
					if constexpr (std::is_void_v<return_type>)
					{// call void func(pContext, args...)
						std::apply(func,
							std::tuple_cat(
								std::forward_as_tuple(
									pSession
								),
								std::move(args))
						);
					}
					else
					{// call return_type func(pContext, args...)
						return struct_pack::serialize(
							std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										pSession
									),
									std::move(args))
							)
						);
					}
				}
			}
			else
			{
				if constexpr (bMemberFunction)
				{
					if constexpr (std::is_void_v<return_type>)
					{// call void o.func()
						(this->*func)();
					}
					else
					{// call return_type o.func()
						return struct_pack::serialize(
							(this->*func)()
						);
					}
				}
				else
				{
					if constexpr (std::is_void_v<return_type>)
					{// call void func()
						func();
					}
					else
					{// call return_type func()
						return struct_pack::serialize(
							func()
						);
					}
				}
			}
			return std::nullopt;
		}

		void post(std::shared_ptr<MysqlContext> pContext);

		DispatchHandler* getDispatchHandler(uint32_t funcid);

		virtual void onRegisterStmt(MysqlSession* pSession) = 0;

		void proccess(std::shared_ptr<MysqlContext> pContext);

		void reconnect(MysqlSession* pSession);

		std::shared_ptr<MysqlSession> getMysqlSession();

		void onDispatch(MysqlSession* pSession, std::shared_ptr<MysqlContext> pContext);

	private:
		string mUrl; // mysql://user:pwd@host:port/db
		uint32_t mReconnectDelaySecond = 10; // 重连延迟时间

		IoContextPool mPool;
		std::thread mThread;

		std::mutex mMutex;
		size_t mNextMysqlSession = 0;
		vector<std::shared_ptr<MysqlSession>> mpMysqlSessions;


		uint32_t mSeqNum = 0;

		tbb::concurrent_queue<std::shared_ptr<MysqlContext>> mProcessPackets;

		unordered_map<uint32_t, DispatchHandler> mDispatchHandlers;
		unordered_map<uint32_t, string> mFuncId2Name;

		struct PromiseInfo
		{
			async_simple::Promise<std::optional<vector<char>>> promise;
		};
		unordered_map<uint32_t, std::shared_ptr<PromiseInfo>> mPromises;
	};
}
