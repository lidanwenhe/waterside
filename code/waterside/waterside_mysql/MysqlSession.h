#pragma once

#include "MysqlPrepareStmt.h"

namespace waterside
{
	class MysqlSession final : private boost::noncopyable
	{
	public:
		MysqlSession();

		~MysqlSession();

		void initialize(std::string_view url);

		void connect();

		bool ping();

		void clear();

		void registerStmt(std::unique_ptr<MysqlPrepareStmt>&& pstmt);

		MysqlPrepareStmt* findStmt(uint32_t index);

		template<typename T>
		  	requires std::is_base_of_v<MysqlPrepareStmt, T> && std::is_enum_v<decltype(T::ID)>
		T* findStmt()
		{
			return static_cast<T*>(findStmt(T::ID));
		}

		class lock_guard final : private boost::noncopyable
		{
		public:
			lock_guard(MysqlSession& db)
				: mDB(db)
				, mbcommit(false)
			{
				mDB.startTransaction();
			}
			~lock_guard()
			{
				mbcommit ? mDB.commit() : mDB.rollback();
			}

			void commit(bool bcommit = true) { mbcommit = bcommit; }

		private:
			MysqlSession& mDB;
			bool mbcommit;
		};

	protected:
		void startTransaction();
		void commit();
		void rollback();

		void close();

	private:
		std::string mUser;
		std::string mPassword;
		std::string mHost;
		uint16_t mPort;
		std::string mDB;

		MYSQL* mpMysql;

		std::vector<std::unique_ptr<MysqlPrepareStmt>> mStmt;
		MysqlPrepareStmt* mpCurStmt;
	};
}