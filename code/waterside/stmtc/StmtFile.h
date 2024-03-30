#pragma once

#include <format>
#include "StmtConfig_generated.h"
#include "flatbuffers/idl.h"

class StmtFile
{
public:
	StmtFile();

	virtual ~StmtFile();

	// 解析
	bool parse(const std::string& filename);

	// 生成
	void gen(const std::string& path);

protected:
	// 验证符合c++命名
	bool verifyIdentifier(std::string_view sv);
	// 验证配置文件
	bool verifyConfig();

	// 生成stmt
	void genStmt(std::ofstream& hfile, std::ofstream& cppfile);
	// 得到参数类型枚举名
	std::string_view getBindTypeName(STMT_TYPE type);
	// 得到C++类型名
	std::string_view getCppTypeName(STMT_TYPE type, bool bStringView = true);
	// 生成初始化参数
	void genInitializeParam(std::ofstream& cppfile, const StmtItem* item);
	// 生成绑定参数
	void genBoundParams(std::ofstream& hfile, std::ofstream& cppfile, const StmtItem* item, bool bFirst);
	void genBoundParamsDefine(std::ofstream& cppfile, const StmtItem* attr);
	// 生成设置参数
	void genSetParam(std::ofstream& hfile, std::ofstream& cppfile, size_t i, const StmtItem* item);
	// 生成得到结果
	void genGetResult(std::ofstream& hfile, std::ofstream& cppfile, size_t i, const StmtItem* item);
	// 得到参数列表
	std::string getParamListString(const StmtConfigItem* item);
	// 得到参数值列表
	std::string getParamValueListString(const StmtConfigItem* item);
	// 得到返回列表
	std::string getResultListString(const StmtConfigItem* item);
	
	// 生成Mysql管理器
	void genMysqlManager(std::ofstream& hfile, std::ofstream& cppfile);
	// 生成回调函数
	void genVoidCallback(std::ofstream& hfile, std::ofstream& cppfile, const StmtConfigItem* item);
	void genCallback(std::ofstream& hfile, std::ofstream& cppfile, const StmtConfigItem* item);
	void genListCallback(std::ofstream& hfile, std::ofstream& cppfile, const StmtConfigItem* item);

private:
	flatbuffers::Parser m_parser;
	const StmtConfig* m_config;
};
