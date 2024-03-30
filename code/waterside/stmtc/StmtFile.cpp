#include "StmtFile.h"
#include <fstream>
#include "flatbuffers/util.h"
#include "boost/algorithm/string.hpp"
using namespace std::literals;

StmtFile::StmtFile()
	: m_config(nullptr)
{
}

StmtFile::~StmtFile()
{
}

bool StmtFile::parse(const std::string& filename)
{
	// load FlatBuffer schema (.fbs) and JSON from disk
	std::string schemafile;
	std::string jsonfile;
	bool ok = flatbuffers::LoadFile("StmtConfig.fbs", false, &schemafile);
	if (!ok)
	{
		printf("couldn't load StmtConfig!\n");
		return false;
	}
	ok = flatbuffers::LoadFile(filename.c_str(), false, &jsonfile);
	if (!ok)
	{
		printf(std::format("couldn't load {}!\n", filename).c_str());
		return false;
	}

	// parse schema first, so we can use it to parse the data after
	const char *include_directories[] = { nullptr };
	ok = m_parser.Parse(schemafile.c_str(), include_directories) &&
		m_parser.Parse(jsonfile.c_str(), include_directories);
	if (!ok)
	{
		printf(std::format("parse {} failed! {}\n", filename, m_parser.error_).c_str());
		return false;
	}

	// here, parser.builder_ contains a binary buffer that is the parsed data.

	flatbuffers::Verifier verifier(m_parser.builder_.GetBufferPointer(), m_parser.builder_.GetSize());
	if (!VerifyStmtConfigBuffer(verifier))
	{
		printf(std::format("verify {} failed!\n", filename).c_str());
		return false;
	}

	m_config = GetStmtConfig(m_parser.builder_.GetBufferPointer());

	return verifyConfig();
}

void StmtFile::gen(const std::string& path)
{
	std::string hfilename = path + m_config->class_name()->str() + ".h";
	std::string cppfilename = path + m_config->class_name()->str() + ".cpp";

	std::ofstream hfile;
	std::ofstream cppfile;
	hfile.open(hfilename, std::ofstream::out | std::ofstream::ate);
	cppfile.open(cppfilename, std::ofstream::out | std::ofstream::ate);

	hfile << "// create by stmtc\n#pragma once\n\n#include \"MysqlManager.h\"\n\n";
	cppfile << "// create by stmtc\n#include \"" << m_config->class_name()->string_view() << ".h\"\n\n";

	if (m_config->package())
	{
		hfile << "namespace " << m_config->package()->string_view() << " {\n";
		cppfile << "namespace " << m_config->package()->string_view() << " {\n";
	}

	genStmt(hfile, cppfile);
	genMysqlManager(hfile, cppfile);

	if (m_config->package())
	{
		hfile << "}\n";
		cppfile << "}\n";
	}
}

bool StmtFile::verifyIdentifier(std::string_view sv)
{
	if (sv.empty())
	{
		return false;
	}

	bool bNumber = false;
	for (auto ch : sv)
	{
		if (bNumber)
		{
			if (ch >= '0' && ch <= '9')
				continue;
		}
		else
			bNumber = true;

		if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_')
		{
			continue;
		}
		return false;
	}
	return true;
}

bool StmtFile::verifyConfig()
{
	for (auto item : *m_config->all())
	{
		if (!verifyIdentifier(item->index()->string_view()))
		{
			printf(std::format("verify identifier failed:{}\n", item->index()->string_view()).c_str());
			return false;
		}
		auto params = item->params();
		if (params)
		{
			for (auto attr : *params)
			{
				if (!verifyIdentifier(attr->name()->string_view()))
				{
					printf(std::format("Index[{}] <params> verify identifier failed:{}\n", item->index()->string_view(), attr->name()->string_view()).c_str());
					return false;
				}
			}
		}

		auto results = item->results();
		if (results)
		{
			for (auto attr : *results)
			{
				if (!verifyIdentifier(attr->name()->string_view()))
				{
					printf(std::format("Index[{}] <results> verify identifier failed:{}\n", item->index()->string_view(), attr->name()->string_view()).c_str());
					return false;
				}
			}
		}
	}
	return true;
}

std::string_view StmtFile::getBindTypeName(STMT_TYPE type)
{
	switch (type)
	{
	case STMT_TYPE_BYTE:
		return "BYTE"sv;
	case STMT_TYPE_UBYTE:
		return "UBYTE"sv;
	case STMT_TYPE_SHORT:
		return "SHORT"sv;
	case STMT_TYPE_USHORT:
		return "USHORT"sv;
	case STMT_TYPE_INT:
	case STMT_TYPE_TIMESTAMP:
		return "INT"sv;
	case STMT_TYPE_UINT:
		return "UINT"sv;
	case STMT_TYPE_LONG:
		return "LONG"sv;
	case STMT_TYPE_ULONG:
		return "ULONG"sv;
	case STMT_TYPE_FLOAT:
		return "FLOAT"sv;
	case STMT_TYPE_DOUBLE:
		return "DOUBLE"sv;
	case STMT_TYPE_STRING:
		return "STRING"sv;
	case STMT_TYPE_VARSTRING:
		return "VARSTRING"sv;
	case STMT_TYPE_TINYBLOB:
		return "TINYBLOB"sv;
	case STMT_TYPE_BLOB:
		return "BLOB"sv;
	case STMT_TYPE_MEDIUMBLOB:
		return "MEDIUMBLOB"sv;
	default:
		break;
	}
	return std::string_view{};
}

std::string_view StmtFile::getCppTypeName(STMT_TYPE type, bool bStringView)
{
	switch (type)
	{
	case STMT_TYPE_BYTE:
		return "int8_t"sv;
	case STMT_TYPE_UBYTE:
		return "uint8_t"sv;
	case STMT_TYPE_SHORT:
		return "int16_t"sv;
	case STMT_TYPE_USHORT:
		return "uint16_t"sv;
	case STMT_TYPE_INT:
	case STMT_TYPE_TIMESTAMP:
		return "int32_t"sv;
	case STMT_TYPE_UINT:
		return "uint32_t"sv;
	case STMT_TYPE_LONG:
		return "int64_t"sv;
	case STMT_TYPE_ULONG:
		return "uint64_t"sv;
	case STMT_TYPE_FLOAT:
		return "float"sv;
	case STMT_TYPE_DOUBLE:
		return "double"sv;
	case STMT_TYPE_STRING:
	case STMT_TYPE_VARSTRING:
	case STMT_TYPE_TINYBLOB:
	case STMT_TYPE_BLOB:
	case STMT_TYPE_MEDIUMBLOB:
		return bStringView ? "std::string_view"sv : "std::string"sv;
	default:
		break;
	}
	return std::string_view{};
}

void StmtFile::genInitializeParam(std::ofstream& cppfile, const StmtItem* item)
{
	cppfile << "{MYSQL_STMT_TYPE::" << getBindTypeName(item->bind_type());
	switch (item->bind_type())
	{
	case STMT_TYPE_STRING:
		cppfile << ", " << item->buf_size();
		break;
	case STMT_TYPE_VARSTRING:
		cppfile << ", " << item->buf_size();
		break;
	case STMT_TYPE_TINYBLOB:
		cppfile << ", " << item->buf_size();
		break;
	case STMT_TYPE_BLOB:
		cppfile << ", " << item->buf_size();
		break;
	case STMT_TYPE_MEDIUMBLOB:
		cppfile << ", " << item->buf_size();
		break;
	case STMT_TYPE_TIMESTAMP:
		if (item->maybe_null())
		{
			cppfile << ", 0, true";
		}
		cppfile << "},";
		return;
	default:
		cppfile << "},";
		return;
	}
	if (item->maybe_null())
	{
		cppfile << ", true";
	}
	cppfile << "},";
}

void StmtFile::genBoundParams(std::ofstream& hfile, std::ofstream& cppfile, const StmtItem* item, bool bFirst)
{
	if (!bFirst)
	{
		hfile << ", ";
		cppfile << ", ";
	}

	auto cppTypeName = getCppTypeName(item->bind_type());
	auto name = item->name()->string_view();
	hfile << cppTypeName << " " << name;
	cppfile << cppTypeName << " " << name;
}

void StmtFile::genBoundParamsDefine(std::ofstream& cppfile, const StmtItem* attr)
{
	auto name = attr->name()->string_view();
	cppfile << "\tset_" << name << "(" << name << ");\n";
}

void StmtFile::genSetParam(std::ofstream& hfile, std::ofstream& cppfile, size_t i, const StmtItem* item)
{
	auto name = item->name()->string_view();
	auto cppTypeName = getCppTypeName(item->bind_type());
	hfile << "\tvoid set_" << name << "(" << cppTypeName << " value) { _setParam(" << i << ", \"" << name << "\", value); }\n";
}

void StmtFile::genGetResult(std::ofstream& hfile, std::ofstream& cppfile, size_t i, const StmtItem* item)
{
	auto name = item->name()->string_view();
	auto cppTypeName = getCppTypeName(item->bind_type());
	hfile << "\t" << cppTypeName << " get_" << name << "() { " << cppTypeName << " value; _getResult(" << i << ", \"" << name << "\", value); return value; }\n";
}

std::string StmtFile::getParamListString(const StmtConfigItem* item)
{
	std::string str;
	if (item->params())
	{
		bool bFirst = true;
		for (auto attr : *item->params())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				str += ",";
			}
			std::string sqlName = "`" + attr->name()->str() + "`";
			str += sqlName;
		}
	}
	return str;
}

std::string StmtFile::getParamValueListString(const StmtConfigItem* item)
{
	std::string str;
	if (item->params())
	{
		bool bFirst = true;
		for (auto attr : *item->params())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				str += ",";
			}
			std::string sqlName = "?";
			if (attr->bind_type() == STMT_TYPE_TIMESTAMP)
			{
				if (attr->maybe_null())
				{
					sqlName = "NULLIF(" + sqlName + ",0)";
				}
				sqlName = "FROM_UNIXTIME(" + sqlName + ")";
			}
			str += sqlName;
		}
	}
	return str;
}

std::string StmtFile::getResultListString(const StmtConfigItem* item)
{
	std::string str;
	if (item->results())
	{
		bool bFirst = true;
		for (auto attr : *item->results())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				str += ",";
			}
			std::string sqlName = "`" + attr->name()->str() + "`";
			if (attr->bind_type() == STMT_TYPE_TIMESTAMP)
			{
				sqlName = "UNIX_TIMESTAMP(" + sqlName + ")";
				if (attr->maybe_null())
				{
					sqlName = "IFNULL(" + sqlName + ",0)";
				}
			}
			str += sqlName;
		}
	}
	return str;
}

void StmtFile::genStmt(std::ofstream& hfile, std::ofstream& cppfile)
{
	int id = 0;
	for (auto item : *m_config->all())
	{
		hfile << "struct " << item->index()->string_view() << " final: public ";
		if (m_config->package() == nullptr || m_config->package()->string_view() != "waterside"sv)
		{
			hfile << "waterside::";
		}
		hfile << "MysqlPrepareStmt\n{\n";
		// 构成函数
		hfile << "\t" << item->index()->string_view() << "() = default;\n";
		// 析构函数
		hfile << "\tvirtual ~" << item->index()->string_view() << "() = default;\n";
		// id
		hfile << "\tenum { ID = " << id++ << " };\n";

		// 初始化
		auto sql = item->sql()->str();
		boost::algorithm::replace_all(sql, "${params}", getParamListString(item));
		boost::algorithm::replace_all(sql, "${values}", getParamValueListString(item));
		boost::algorithm::replace_all(sql, "${results}", getResultListString(item));
		hfile << "\tvirtual void initialize() override;\n";
		cppfile << "void " << item->index()->string_view() << "::initialize()\n{\n";
		cppfile << "\t_initialize(ID"
			<< ",\n\t\t\"" << sql
			<< "\",\n\t\t{";
		if (item->params())
		{
			for (auto attr : *item->params())
			{
				cppfile << "\n\t\t\t";
				genInitializeParam(cppfile, attr);
			}
			cppfile << "\n\t\t";
		}
		cppfile << "},\n\t\t{";
		if (item->results())
		{
			for (auto attr : *item->results())
			{
				cppfile << "\n\t\t\t";
				genInitializeParam(cppfile, attr);
			}
			cppfile << "\n\t\t";
		}
		cppfile << "});\n}\n\n";


		// boundParams
		if (item->params())
		{
			bool bFirst = true;
			hfile << "\tvoid boundParams(";
			cppfile << "void " << item->index()->string_view() << "::boundParams(";
			for (auto attr : *item->params())
			{
				genBoundParams(hfile, cppfile, attr, bFirst);
				bFirst = false;
			}
			hfile << ");\n";
			cppfile << ")\n{\n";

			for (auto attr : *item->params())
			{
				genBoundParamsDefine(cppfile, attr);
			}
			cppfile << "}\n\n";
		}

		// set
		if (item->params())
		{
			size_t i = 0;
			for (auto attr : *item->params())
			{
				genSetParam(hfile, cppfile, i++, attr);
			}
		}
		// get
		if (item->results())
		{
			size_t i = 0;
			for (auto attr : *item->results())
			{
				genGetResult(hfile, cppfile, i++, attr);
			}
		}

		hfile << "};\n\n";
	}
}

void StmtFile::genMysqlManager(std::ofstream& hfile, std::ofstream& cppfile)
{
	auto class_name = m_config->class_name()->string_view();

	hfile << "class " << class_name << " : public ";
	if (m_config->package() == nullptr || m_config->package()->string_view() != "waterside"sv)
	{
		hfile << "waterside::";
	}
	hfile << "MysqlManager\n{\npublic:\n";
	// 构成函数
	hfile << "\t" << class_name << "(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond);\n";
	cppfile << class_name << "::" << class_name << "(std::string_view url, uint32_t multithreading, uint32_t reconnectDelaySecond)\n\t: MysqlManager(url, multithreading, reconnectDelaySecond)\n{\n}\n\n";
	// 析构函数
	hfile << "\tvirtual ~" << class_name << "() = default;\n";
	hfile << "protected:\n";

	// onRegisterStmt
	hfile << "\tvirtual void onRegisterStmt(MysqlSession* pSession) override;\n";
	cppfile << "void " << class_name << "::onRegisterStmt(MysqlSession* pSession)\n{\n";
	for (auto item : *m_config->all())
	{
		cppfile << "\tpSession->registerStmt(std::make_unique<" << item->index()->string_view() << ">());\n";
	}
	cppfile << "}\n\n";

	// onRegistHandler
	hfile << "public:\n";
	hfile << "\tvirtual void onRegistHandler();\n";
	cppfile << "void " << class_name << "::onRegistHandler()\n{\n";
	for (auto item : *m_config->all())
	{
		if (item->cb() != STMT_CALLBACK_NO)
		{
			auto cb_name = item->index()->str();
			boost::algorithm::replace_first(cb_name, "Stmt", "on");
			cppfile << "\tregistHandler<&" << class_name << "::" << cb_name << ">();\n";
		}
	}
	cppfile << "}\n";

	// callback
	for (auto item : *m_config->all())
	{
		switch (item->cb())
		{
		case STMT_CALLBACK_RETURN_VOID:
			genVoidCallback(hfile, cppfile, item);
			break;
		case STMT_CALLBACK_RETURN:
			genCallback(hfile, cppfile, item);
			break;
		case STMT_CALLBACK_RETURN_LIST:
			genListCallback(hfile, cppfile, item);
			break;
		default:
			break;
		}
	}

	hfile << "};\n";
}

void StmtFile::genVoidCallback(std::ofstream& hfile, std::ofstream& cppfile, const StmtConfigItem* item)
{
	auto class_name = m_config->class_name()->string_view();
	auto cb_name = item->index()->str();
	boost::algorithm::replace_first(cb_name, "Stmt", "on");
	hfile << "\tvoid " << cb_name << "(MysqlSession* pSession";
	cppfile << "\nvoid " << class_name << "::" << cb_name << "(MysqlSession* pSession";
	if (item->params())
	{
		for (auto attr : *item->params())
		{
			genBoundParams(hfile, cppfile, attr, false);
		}
	}
	hfile << ");\n";
	cppfile << ")\n{\n";

	cppfile << "\tauto pstmt = pSession->findStmt<" << item->index()->string_view() << ">();\n";
	cppfile << "\tif (pstmt)\n\t{\n";

	if (item->params())
	{
		cppfile << "\t\tpstmt->boundParams(";
		bool bFirst = true;
		for (auto attr : *item->params())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				cppfile << ", ";
			}
			cppfile << attr->name()->string_view();
		}
		cppfile << ");\n";
	}
	cppfile << "\t\tpstmt->execute();\n";

	cppfile << "\t}\n\telse\n\t{\n\t\tassert(false);\n\t}\n";

	cppfile << "}\n";
}

void StmtFile::genCallback(std::ofstream& hfile, std::ofstream& cppfile, const StmtConfigItem* item)
{
	auto class_name = m_config->class_name()->string_view();
	auto cb_name = item->index()->str();
	boost::algorithm::replace_first(cb_name, "Stmt", "on");
	std::string strRet = "void";
	if (item->results())
	{
		strRet = "std::tuple<";
		bool bFirst = true;
		for (auto attr : *item->results())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				strRet += ", ";
			}
			strRet += getCppTypeName(attr->bind_type());
		}
		strRet += ">";
	}
	hfile << "\t" << strRet << " " << cb_name << "(MysqlSession* pSession";
	cppfile << "\n" << strRet << " " << class_name << "::" << cb_name << "(MysqlSession* pSession";
	if (item->params())
	{
		for (auto attr : *item->params())
		{
			genBoundParams(hfile, cppfile, attr, false);
		}
	}
	hfile << ");\n";
	cppfile << ")\n{\n";

	cppfile << "\t" << strRet << " ret;\n";

	cppfile << "\tauto pstmt = pSession->findStmt<" << item->index()->string_view() << ">();\n";
	cppfile << "\tif (pstmt)\n\t{\n";

	if (item->params())
	{
		cppfile << "\t\tpstmt->boundParams(";
		bool bFirst = true;
		for (auto attr : *item->params())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				cppfile << ", ";
			}
			cppfile << attr->name()->string_view();
		}
		cppfile << ");\n";
	}
	cppfile << "\t\tpstmt->execute();\n";
	cppfile << "\t\tif (pstmt->fetch())\n\t\t{\n";
	if (item->results())
	{
		int i = 0;
		for (auto attr : *item->results())
		{
			cppfile << "\t\t\tstd::get<" << i++ << ">(ret) = pstmt->get_" << attr->name()->string_view() << "();\n";
		}
	}
	cppfile << "\t\t}\n";
	cppfile << "\t}\n\telse\n\t{\n\t\tassert(false);\n\t}\n";
	cppfile << "\treturn ret;\n";

	cppfile << "}\n";
}

void StmtFile::genListCallback(std::ofstream& hfile, std::ofstream& cppfile, const StmtConfigItem* item)
{
	auto class_name = m_config->class_name()->string_view();
	auto cb_name = item->index()->str();
	boost::algorithm::replace_first(cb_name, "Stmt", "on");
	std::string strRet = "void";
	if (item->results())
	{
		strRet = "std::tuple<";
		bool bFirst = true;
		for (auto attr : *item->results())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				strRet += ", ";
			}
			strRet += getCppTypeName(attr->bind_type());
		}
		strRet += ">";
	}
	hfile << "\tstd::vector<" << strRet << "> " << cb_name << "(MysqlSession* pSession";
	cppfile << "\nstd::vector<" << strRet << "> " << class_name << "::" << cb_name << "(MysqlSession* pSession";
	if (item->params())
	{
		for (auto attr : *item->params())
		{
			genBoundParams(hfile, cppfile, attr, false);
		}
	}
	hfile << ");\n";
	cppfile << ")\n{\n";

	cppfile << "\tstd::vector<" << strRet << "> ret;\n";

	cppfile << "\tauto pstmt = pSession->findStmt<" << item->index()->string_view() << ">();\n";
	cppfile << "\tif (pstmt)\n\t{\n";

	if (item->params())
	{
		cppfile << "\t\tpstmt->boundParams(";
		bool bFirst = true;
		for (auto attr : *item->params())
		{
			if (bFirst)
			{
				bFirst = false;
			}
			else
			{
				cppfile << ", ";
			}
			cppfile << attr->name()->string_view();
		}
		cppfile << ");\n";
	}
	cppfile << "\t\tpstmt->execute();\n";
	cppfile << "\t\twhile (pstmt->fetch())\n\t\t{\n";
	if (item->results())
	{
		int i = 0;
		for (auto attr : *item->results())
		{
			cppfile << "\t\t\t" << strRet << " temp;\n";
			cppfile << "\t\t\tstd::get<" << i++ << ">(temp) = pstmt->get_" << attr->name()->string_view() << "();\n";
			cppfile << "\t\t\tret.emplace_back(std::move(temp));\n";
		}
	}
	cppfile << "\t\t}\n";
	cppfile << "\t}\n\telse\n\t{\n\t\tassert(false);\n\t}\n";
	cppfile << "\treturn ret;\n";

	cppfile << "}\n";
}

