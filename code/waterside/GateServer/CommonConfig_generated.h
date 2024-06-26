// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_COMMONCONFIG_WATERSIDE_H_
#define FLATBUFFERS_GENERATED_COMMONCONFIG_WATERSIDE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

namespace waterside {

struct NetworkListenConfig;
struct NetworkListenConfigBuilder;

struct MysqlConfig;
struct MysqlConfigBuilder;

/// 网络监听配置
struct NetworkListenConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef NetworkListenConfigBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_IP = 4,
    VT_PORT = 6,
    VT_MULTI_THREADING = 8
  };
  /// 本服务器ip
  const ::flatbuffers::String *ip() const {
    return GetPointer<const ::flatbuffers::String *>(VT_IP);
  }
  /// 监听端口
  uint16_t port() const {
    return GetField<uint16_t>(VT_PORT, 0);
  }
  /// 启动网络线程数，0为CPU数处理线程
  uint32_t multi_threading() const {
    return GetField<uint32_t>(VT_MULTI_THREADING, 0);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_IP) &&
           verifier.VerifyString(ip()) &&
           VerifyField<uint16_t>(verifier, VT_PORT, 2) &&
           VerifyField<uint32_t>(verifier, VT_MULTI_THREADING, 4) &&
           verifier.EndTable();
  }
};

struct NetworkListenConfigBuilder {
  typedef NetworkListenConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_ip(::flatbuffers::Offset<::flatbuffers::String> ip) {
    fbb_.AddOffset(NetworkListenConfig::VT_IP, ip);
  }
  void add_port(uint16_t port) {
    fbb_.AddElement<uint16_t>(NetworkListenConfig::VT_PORT, port, 0);
  }
  void add_multi_threading(uint32_t multi_threading) {
    fbb_.AddElement<uint32_t>(NetworkListenConfig::VT_MULTI_THREADING, multi_threading, 0);
  }
  explicit NetworkListenConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<NetworkListenConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<NetworkListenConfig>(end);
    fbb_.Required(o, NetworkListenConfig::VT_IP);
    return o;
  }
};

inline ::flatbuffers::Offset<NetworkListenConfig> CreateNetworkListenConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> ip = 0,
    uint16_t port = 0,
    uint32_t multi_threading = 0) {
  NetworkListenConfigBuilder builder_(_fbb);
  builder_.add_multi_threading(multi_threading);
  builder_.add_ip(ip);
  builder_.add_port(port);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<NetworkListenConfig> CreateNetworkListenConfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *ip = nullptr,
    uint16_t port = 0,
    uint32_t multi_threading = 0) {
  auto ip__ = ip ? _fbb.CreateString(ip) : 0;
  return waterside::CreateNetworkListenConfig(
      _fbb,
      ip__,
      port,
      multi_threading);
}

/// Mysql配置
struct MysqlConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef MysqlConfigBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_URL = 4,
    VT_MULTI_THREADING = 6,
    VT_RECONNECT_DELAY_SECOND = 8
  };
  /// mysql连接格式：mysql://user:pwd@host:port/db
  const ::flatbuffers::String *url() const {
    return GetPointer<const ::flatbuffers::String *>(VT_URL);
  }
  /// 默认CPU数处理线程，0为CPU数处理线程
  uint32_t multi_threading() const {
    return GetField<uint32_t>(VT_MULTI_THREADING, 0);
  }
  /// 断开连接后自动重连等待多少秒
  uint32_t reconnect_delay_second() const {
    return GetField<uint32_t>(VT_RECONNECT_DELAY_SECOND, 10);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyOffsetRequired(verifier, VT_URL) &&
           verifier.VerifyString(url()) &&
           VerifyField<uint32_t>(verifier, VT_MULTI_THREADING, 4) &&
           VerifyField<uint32_t>(verifier, VT_RECONNECT_DELAY_SECOND, 4) &&
           verifier.EndTable();
  }
};

struct MysqlConfigBuilder {
  typedef MysqlConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_url(::flatbuffers::Offset<::flatbuffers::String> url) {
    fbb_.AddOffset(MysqlConfig::VT_URL, url);
  }
  void add_multi_threading(uint32_t multi_threading) {
    fbb_.AddElement<uint32_t>(MysqlConfig::VT_MULTI_THREADING, multi_threading, 0);
  }
  void add_reconnect_delay_second(uint32_t reconnect_delay_second) {
    fbb_.AddElement<uint32_t>(MysqlConfig::VT_RECONNECT_DELAY_SECOND, reconnect_delay_second, 10);
  }
  explicit MysqlConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<MysqlConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<MysqlConfig>(end);
    fbb_.Required(o, MysqlConfig::VT_URL);
    return o;
  }
};

inline ::flatbuffers::Offset<MysqlConfig> CreateMysqlConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    ::flatbuffers::Offset<::flatbuffers::String> url = 0,
    uint32_t multi_threading = 0,
    uint32_t reconnect_delay_second = 10) {
  MysqlConfigBuilder builder_(_fbb);
  builder_.add_reconnect_delay_second(reconnect_delay_second);
  builder_.add_multi_threading(multi_threading);
  builder_.add_url(url);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<MysqlConfig> CreateMysqlConfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    const char *url = nullptr,
    uint32_t multi_threading = 0,
    uint32_t reconnect_delay_second = 10) {
  auto url__ = url ? _fbb.CreateString(url) : 0;
  return waterside::CreateMysqlConfig(
      _fbb,
      url__,
      multi_threading,
      reconnect_delay_second);
}

}  // namespace waterside

#endif  // FLATBUFFERS_GENERATED_COMMONCONFIG_WATERSIDE_H_
