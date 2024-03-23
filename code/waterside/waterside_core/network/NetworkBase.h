#pragma once

#include "SessionBase.h"
#include "SessionIdManager.h"
#include "ObjectManager.h"
#include "Logger.h"
#include "tbb/concurrent_queue.h"
#include "async_simple/coro/FutureAwaiter.h"
#include "ylt/struct_pack.hpp"

namespace waterside
{
	class NetworkBase
	{
	public:
		void cleanSession(SessionID sessionId);

		std::shared_ptr<SessionBase> findSession(SessionID sessionId);

		void addProcessPacket(std::shared_ptr<NetworkContext>& pContext);

		void onProcessPacket();

		std::function<void(SessionID)> onDisconnectCallback;

		template <auto func>
		void registHandler()
		{
			using T = decltype(func);
			using param_type = coro_rpc::function_parameters_t<T>;
			using return_type = coro_rpc::function_return_type_t<T>;

			constexpr auto key = coro_rpc::func_id<func>();
			constexpr auto name = coro_rpc::get_func_name<func>();

			if constexpr (coro_rpc::is_specialization_v<return_type, async_simple::coro::Lazy>)
			{
				auto it = mCoroDispatchHandlers.emplace(
					key,
					std::bind(&NetworkBase::coroNetDispatcher<func>, this, std::placeholders::_1));
				if (!it.second)
				{
					MLOG_CRITICAL(NET, "duplication function {} register!", name);
				}
			}
			else
			{
				auto it = mDispatchHandlers.emplace(
					key,
					std::bind(&NetworkBase::netDispatcher<func>, this, std::placeholders::_1));
				if (!it.second)
				{
					MLOG_CRITICAL(NET, "duplication function {} register!", name);
				}
			}

			mFuncId2Name.emplace(key, name);
		}

		template <auto func, typename... Args>
		async_simple::coro::Lazy<std::optional<decltype(getReturnType<func>())>>
			call(SessionID sessionId, Args... args)
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			using return_type = typename get_type_t<coro_rpc::function_return_type_t<Function>>::type;

			static_check<func, Args...>();

			auto pSession = findSession(sessionId);
			if (!pSession)
			{
				co_return std::nullopt;
			}

			std::vector<char> buffer;
			SERIALIZATION_TYPE type = prepareBuffer<func>(buffer, std::move(args)...);

			auto seqnum = getSeqNum();
			RpcPacketHeader header(MESSAGE_PACKET_TYPE_RPC_REQUEST,
				type,
				buffer.size(),
				seqnum,
				coro_rpc::func_id<func>()
				);

			pSession->send(header, std::pair<const void*, size_t>{buffer.data(), buffer.size()});
			
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
						MLOG_WARN(NET, "struct_pack::deserialize_to error:{}, function:{}",
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
		
	protected:
		SessionID getNewId()
		{
			return mSessionIdManager.getNewId();
		}

		uint32_t getSeqNum()
		{
			return ++mSeqNum;
		}

		void addSession(std::shared_ptr<SessionBase> pSession);

		string_view getFunctionName(uint32_t key);

		template <auto func, typename objid, typename... Args>
		void static_check_member()
		{
			static_assert(std::is_same_v<objid, unique_id>, "objid must be unique_id");

			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			constexpr bool bContext = hasContext<param_type>();

			using Self = coro_rpc::class_type_t<Function>;
			if constexpr (bContext)
			{
				static_assert(coro_rpc::is_invocable<Function, Self, NetworkContext, Args...>::value,
					"called rpc function and arguments are not match");
			}
			else
			{
				static_assert(coro_rpc::is_invocable<Function, Self, Args...>::value,
					"called rpc function and arguments are not match");
			}
		}

		template <auto func, typename... Args>
		void static_check()
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			using return_type = typename get_type_t<coro_rpc::function_return_type_t<Function>>::type;

			constexpr bool bContext = hasContext<param_type>();
			constexpr bool bObjectMemberFunction = isObjectmemberFunction<Function>();

			if constexpr (!std::is_void_v<param_type>)
			{
				auto args = getArgs<bContext, bObjectMemberFunction, param_type>();

				if constexpr (isFlatbuffersArg<decltype(args)>())
				{
					static_assert(std::is_void_v<return_type>, "the return_type must be void");
				}
				else
				{
					if constexpr (bObjectMemberFunction)
					{
						static_check_member<func, Args...>();
					}
					else
					{
						if constexpr (bContext)
						{
							static_assert(coro_rpc::is_invocable<Function, NetworkContext, Args...>::value,
								"called rpc function and arguments are not match");
						}
						else
						{
							static_assert(coro_rpc::is_invocable<Function, Args...>::value,
								"called rpc function and arguments are not match");
						}
					}
				}
			}
			else
			{
				if constexpr (bContext)
				{
					static_assert(coro_rpc::is_invocable<Function, NetworkContext, Args...>::value,
						"called rpc function and arguments are not match");
				}
				else
				{
					static_assert(coro_rpc::is_invocable<Function, Args...>::value,
						"called rpc function and arguments are not match");
				}
			}
		}

		template <typename FuncArgs>
		auto get_func_args() {
			using First = std::tuple_element_t<0, FuncArgs>;
			constexpr bool has_conn_v = requires { typename First::return_type; };
			return coro_rpc::get_args<has_conn_v, FuncArgs>();
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
			using tuple_pack = decltype(get_func_args<FuncArgs>());
			pack_to_helper<tuple_pack>(
				std::make_index_sequence<std::tuple_size_v<tuple_pack>>{}, buffer,
				std::forward<Args>(args)...);
		}

		template <auto func, typename... Args>
		SERIALIZATION_TYPE prepareBuffer(std::vector<char>& buffer, Args&&... args)
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;

			constexpr bool bContext = hasContext<param_type>();
			constexpr bool bObjectMemberFunction = isObjectmemberFunction<Function>();

			if constexpr (sizeof...(Args) > 0)
			{
				auto _args = getArgs<bContext, bObjectMemberFunction, param_type>();

				if constexpr (isFlatbuffersArg<decltype(_args)>())
				{
					if constexpr (sizeof...(Args) == 1)
					{
						(buffer.assign(reinterpret_cast<char*>(args->GetBufferPointer()), reinterpret_cast<char*>(args->GetBufferPointer()) + args->GetSize()), ...);
						return SERIALIZATION_TYPE_FLAT_BUFFERS;
					}
				}
				else
				{
					pack_to<decltype(_args)>(buffer, std::forward<Args>(args)...);
					return SERIALIZATION_TYPE_STRUCT_PACK;
				}
			}
			return SERIALIZATION_TYPE_NO;
		}


		using DispatchHandler = std::function<std::optional<vector<char>>(
			std::shared_ptr<NetworkContext>& pContext)>;

		using CoroDispatchHandler = std::function<async_simple::coro::Lazy<std::optional<vector<char>>>(
			std::shared_ptr<NetworkContext>& pContext)>;

		template <typename T>
		constexpr static bool hasContext()
		{
			if constexpr (!std::is_void_v<T>)
			{
				using First = std::tuple_element_t<0, T>;
				return std::is_same_v<std::remove_cvref_t<First>, NetworkContext>;
			}
			else
			{
				return false;
			}
		}

		template <typename Function>
		constexpr static bool isObjectmemberFunction()
		{
			if constexpr (std::is_member_function_pointer_v<Function>)
			{
				using Self = coro_rpc::class_type_t<Function>;
				static_assert(std::is_base_of_v<ObjectBase, Self>, "must derived from ObjectBase");
				return true;
			}
			else
			{
				return false;
			}
		}

		template <typename T>
		constexpr static bool isFlatbuffersArg()
		{
			constexpr size_t size = std::tuple_size_v<T>;
			if  constexpr (size == 1)
			{
				using First = std::tuple_element_t<0, T>;
				return std::is_base_of_v<flatbuffers::Table, std::remove_pointer_t<std::remove_cv_t<First>>>;
			}
			else
			{
				return false;
			}
		}

		template <bool bContext, bool bObjectMemberFunction, typename T>
		constexpr static auto getArgs()
		{
			if constexpr (bContext)
			{
				using args_type = coro_rpc::remove_first_t<T>;
				if constexpr (bObjectMemberFunction)
				{
					if constexpr (!isFlatbuffersArg<decltype(args_type)>())
					{
						return std::tuple_cat(std::forward_as_tuple(unique_id), args_type);
					}
					else
					{
						return args_type{};
					}
				}
				else
				{
					return args_type{};
				}
			}
			else
			{
				if constexpr (bObjectMemberFunction)
				{
					if constexpr (!isFlatbuffersArg<T>())
					{
						return std::tuple_cat(std::forward_as_tuple(unique_id), T);
					}
					else
					{
						return T{};
					}
				}
				else
				{
					return T{};
				}
			}
		}

		template <bool bObjectMember, typename T>
		bool packetDeserialize(T& args, std::shared_ptr<NetworkContext>& pContext)
		{
			if constexpr (isFlatbuffersArg<T>())
			{
				if (pContext->header.serialize_type() != SERIALIZATION_TYPE_FLAT_BUFFERS)
				{
					MLOG_WARN(NET, "serialize_type must FLAT_BUFFERS type:{}, function:{}", EnumNameSERIALIZATION_TYPE(pContext->header.serialize_type()), getFunctionName(pContext->header.function_id()));
					return false;
				}

				using First = std::tuple_element_t<0, T>;
				if constexpr (bObjectMember && !requires { First->obj_id()->unique_id; })
				{
					MLOG_ERROR(NET, "fb must has obj_id(), function:{}", getFunctionName(pContext->header.function_id()));
					return false;
				}

				using FbType = std::remove_pointer_t<std::remove_cv_t<First>>;
				flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(pContext->body.data()), pContext->body.size());
				if (!verifier.VerifyBuffer<FbType>(nullptr))
				{
					MLOG_WARN(NET, "fb verify failed, function:{}", getFunctionName(pContext->header.function_id()));
					return false;
				}

				std::get<0>(args) = flatbuffers::GetRoot<FbType>(pContext->body.data());
			}
			else
			{
				constexpr size_t size = std::tuple_size_v<T>;
				if constexpr (size > 0)
				{
					if (pContext->header.serialize_type() != SERIALIZATION_TYPE_STRUCT_PACK)
					{
						MLOG_WARN(NET, "serialize_type must STRUCT_PACK type:{}, function:{}", EnumNameSERIALIZATION_TYPE(pContext->header.serialize_type()), getFunctionName(pContext->header.function_id()));
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
						MLOG_WARN(NET, "struct_pack::deserialize_to error:{}, function:{}", struct_pack::detail::category().message((int)ok), getFunctionName(pContext->header.function_id()));
						return false;
					}
				}
				else
				{
					MLOG_WARN(NET, "args size=0 type:{}, function:{}", EnumNameSERIALIZATION_TYPE(pContext->header.serialize_type()), getFunctionName(pContext->header.function_id()));
					return false;
				}
			}
			return true;
		}

		template <typename T>
		unique_id getObjId(T& args)
		{
			if constexpr (isFlatbuffersArg<T>())
			{
				using First = std::tuple_element_t<0, args>;
				if constexpr (requires { First->obj_id()->unique_id; })
				{
					return std::get<0>(args)->obj_id();
				}
				else
				{
					return 0;
				}
			}
			else
			{
				constexpr size_t size = std::tuple_size_v<T>;
				if constexpr (size > 0)
				{
					using First = std::tuple_element_t<0, args>;
					if constexpr (std::is_same_v<unique_id, std::remove_cvref_t<First>>)
					{
						return std::get<0>(args);
					}
					else
					{
						return 0;
					}
				}
				else
				{
					return 0;
				}
			}
			return 0;
		}

		template<auto func>
		std::optional<vector<char>> netDispatcher(std::shared_ptr<NetworkContext>& pContext)
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			using return_type = coro_rpc::function_return_type_t<Function>;

			constexpr bool bContext = hasContext<param_type>();
			constexpr bool bObjectMemberFunction = isObjectmemberFunction<Function>();

			if constexpr (!std::is_void_v<param_type>)
			{
				auto args = getArgs<bContext, bObjectMemberFunction, param_type>();
				
				if constexpr (isFlatbuffersArg<decltype(args)>())
				{
					static_assert(std::is_void_v<return_type>, "the return_type must be void");
				}

				if (!packetDeserialize<bObjectMemberFunction>(args, pContext))
				{
					return std::nullopt;
				}

				if constexpr (bObjectMemberFunction)
				{
					auto objId = getObjId(args);
					auto pObject = ObjectManager::instance().findObject(objId);
					if (!pObject)
					{
						MLOG_WARN(NET, "object not found, id:{}, function:{}", objId, getFunctionName(pContext->header.function_id()));
						return std::nullopt;
					}

					if constexpr (std::is_void_v<return_type>)
					{
						if constexpr (bContext)
						{// call void o.func(pContext, args...)
							std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*pObject,
										*pContext
									),
									std::move(args))
							);
						}
						else
						{// call void o.func(args...)
							std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*pObject
									),
									std::move(args))
							);
						}
					}
					else
					{
						if constexpr (bContext)
						{// call return_type o.func(pContext, args...)
							return struct_pack::serialize(
								std::apply(func,
									std::tuple_cat(
										std::forward_as_tuple(
											*pObject,
											*pContext
										),
										std::move(args))
								)
							);
						}
						else
						{// call return_type o.func(args...)
							return struct_pack::serialize(
								std::apply(func,
									std::tuple_cat(
										std::forward_as_tuple(
											*pObject
										),
										std::move(args))
								)
							);
						}
					}
				}
				else
				{
					if constexpr (std::is_void_v<return_type>)
					{
						if constexpr (bContext)
						{// call void func(pContext, args...)
							std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*pContext
									),
									std::move(args))
							);
						}
						else
						{// call void func(args...)
							std::apply(func,
								std::move(args)
							);
						}
					}
					else
					{
						if constexpr (bContext)
						{// call return_type func(pContext, args...)
							return struct_pack::serialize(
								std::apply(func,
									std::tuple_cat(
										std::forward_as_tuple(
											*pContext
										),
										std::move(args))
								)
							);
						}
						else
						{// call return_type func(args...)
							return struct_pack::serialize(
								std::apply(func,
									std::move(args)
								)
							);
						}
					}
				}
			}
			else
			{
				if constexpr (bObjectMemberFunction)
				{
					std::tuple<unique_id> args{};

					if (!packetDeserialize<bObjectMemberFunction>(args, pContext))
					{
						return std::nullopt;
					}

					auto objId = getObjId(args);
					auto pObject = ObjectManager::instance().findObject(objId);
					if (!pObject)
					{
						MLOG_WARN(NET, "object not found, id:{}, function:{}", objId, getFunctionName(pContext->header.function_id()));
						return std::nullopt;
					}

					if constexpr (std::is_void_v<return_type>)
					{// call void o.func()
						(pObject->*func)();
					}
					else
					{// call return_type o.func()
						return struct_pack::serialize(
							(pObject->*func)()
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
			return vector<char>{};
		}

		template<auto func>
		async_simple::coro::Lazy<std::optional<vector<char>>> coroNetDispatcher(std::shared_ptr<NetworkContext>& pContext)
		{
			using Function = decltype(func);
			using param_type = coro_rpc::function_parameters_t<Function>;
			using return_type = typename get_type_t<coro_rpc::function_return_type_t<Function>>::type;

			constexpr bool bContext = hasContext<param_type>();
			constexpr bool bObjectMemberFunction = isObjectmemberFunction<Function>();

			if constexpr (!std::is_void_v<param_type>)
			{
				auto args = getArgs<bContext, bObjectMemberFunction, param_type>();
				
				if constexpr (isFlatbuffersArg<decltype(args)>())
				{
					static_assert(std::is_void_v<return_type>, "the return_type must be void");
				}

				if (!packetDeserialize<bObjectMemberFunction>(args, pContext))
				{
					co_return std::nullopt;
				}

				if constexpr (bObjectMemberFunction)
				{
					auto objId = getObjId(args);
					auto pObject = ObjectManager::instance().findObject(objId);
					if (!pObject)
					{
						MLOG_WARN(NET, "object not found, id:{}, function:{}", objId, getFunctionName(pContext->header.function_id()));
						co_return std::nullopt;
					}

					if constexpr (std::is_void_v<return_type>)
					{
						if constexpr (bContext)
						{// call void o.func(pContext, args...)
							co_await std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*pObject,
										*pContext
									),
									std::move(args))
							);
						}
						else
						{// call void o.func(args...)
							co_await std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*pObject
									),
									std::move(args))
							);
						}
					}
					else
					{
						if constexpr (bContext)
						{// call return_type o.func(pContext, args...)
							co_return struct_pack::serialize(
								co_await std::apply(func,
									std::tuple_cat(
										std::forward_as_tuple(
											*pObject,
											*pContext
										),
										std::move(args))
								)
							);
						}
						else
						{// call return_type o.func(args...)
							co_return struct_pack::serialize(
								co_await std::apply(func,
									std::tuple_cat(
										std::forward_as_tuple(
											*pObject
										),
										std::move(args))
								)
							);
						}
					}
				}
				else
				{
					if constexpr (std::is_void_v<return_type>)
					{
						if constexpr (bContext)
						{// call void func(pContext, args...)
							co_await std::apply(func,
								std::tuple_cat(
									std::forward_as_tuple(
										*pContext
									),
									std::move(args))
							);
						}
						else
						{// call void func(args...)
							co_await std::apply(func,
								std::move(args)
							);
						}
					}
					else
					{
						if constexpr (bContext)
						{// call return_type func(pContext, args...)
							co_return struct_pack::serialize(
								co_await std::apply(func,
									std::tuple_cat(
										std::forward_as_tuple(
											*pContext
										),
										std::move(args))
								)
							);
						}
						else
						{// call return_type func(args...)
							co_return struct_pack::serialize(
								co_await std::apply(func,
									std::move(args)
								)
							);
						}
					}
				}
			}
			else
			{
				if constexpr (bObjectMemberFunction)
				{
					std::tuple<unique_id> args{};

					if (!packetDeserialize<bObjectMemberFunction>(args, pContext))
					{
						co_return std::nullopt;
					}

					auto objId = getObjId(args);
					auto pObject = ObjectManager::instance().findObject(objId);
					if (!pObject)
					{
						MLOG_WARN(NET, "object not found, id:{}, function:{}", objId, getFunctionName(pContext->header.function_id()));
						co_return std::nullopt;
					}

					if constexpr (std::is_void_v<return_type>)
					{// call void o.func()
						co_await (pObject->*func)();
					}
					else
					{// call return_type o.func()
						co_return struct_pack::serialize(
							co_await(pObject->*func)()
						);
					}
				}
				else
				{
					if constexpr (std::is_void_v<return_type>)
					{// call void func()
						co_await func();
					}
					else
					{// call return_type func()
						co_return struct_pack::serialize(
							co_await func()
						);
					}
				}
			}
			co_return vector<char>{};
		}

		DispatchHandler* getDispatchHandler(uint32_t funcid);
		CoroDispatchHandler* getCoroDispatchHandler(uint32_t funcid);

		async_simple::coro::Lazy<void> onDispatch(std::shared_ptr<NetworkContext> pContext);

	protected:
		void closeAllSession();

	private:
		SessionIdManager mSessionIdManager;
		tbb::concurrent_hash_map<SessionID, std::shared_ptr<SessionBase>> mSessions;

		uint32_t mSeqNum = 0;

		tbb::concurrent_queue<std::shared_ptr<NetworkContext>> mProcessPackets;
		
		unordered_map<uint32_t, DispatchHandler> mDispatchHandlers;
		unordered_map<uint32_t, CoroDispatchHandler> mCoroDispatchHandlers;
		unordered_map<uint32_t, string> mFuncId2Name;

		struct PromiseInfo
		{
			async_simple::Promise<std::optional<vector<char>>> promise;
		};
		unordered_map<uint32_t, std::shared_ptr<PromiseInfo>> mPromises;
	};
}
