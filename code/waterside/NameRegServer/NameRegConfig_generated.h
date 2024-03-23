// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_NAMEREGCONFIG_WATERSIDE_H_
#define FLATBUFFERS_GENERATED_NAMEREGCONFIG_WATERSIDE_H_

#include "flatbuffers/flatbuffers.h"

// Ensure the included flatbuffers.h is the same version as when this file was
// generated, otherwise it may not be compatible.
static_assert(FLATBUFFERS_VERSION_MAJOR == 23 &&
              FLATBUFFERS_VERSION_MINOR == 5 &&
              FLATBUFFERS_VERSION_REVISION == 26,
             "Non-compatible flatbuffers version included");

#include "CommonConfig_generated.h"

namespace waterside {

struct NameRegConfig;
struct NameRegConfigBuilder;

/// 名字注册服务器配置
struct NameRegConfig FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {
  typedef NameRegConfigBuilder Builder;
  enum FlatBuffersVTableOffset FLATBUFFERS_VTABLE_UNDERLYING_TYPE {
    VT_SERVER_ID = 4,
    VT_SERVICE_NAME = 6,
    VT_LOGIC_FPS = 8,
    VT_LOGIC_FPS_WARNING = 10,
    VT_NETWORK = 12
  };
  /// 服务器ID
  int32_t server_id() const {
    return GetField<int32_t>(VT_SERVER_ID, 0);
  }
  /// 服务名
  const ::flatbuffers::String *service_name() const {
    return GetPointer<const ::flatbuffers::String *>(VT_SERVICE_NAME);
  }
  /// 逻辑帧速
  float logic_fps() const {
    return GetField<float>(VT_LOGIC_FPS, 0.0f);
  }
  /// 逻辑警告日志帧速
  float logic_fps_warning() const {
    return GetField<float>(VT_LOGIC_FPS_WARNING, 0.0f);
  }
  /// 网络监听配置
  const waterside::NetworkListenConfig *network() const {
    return GetPointer<const waterside::NetworkListenConfig *>(VT_NETWORK);
  }
  bool Verify(::flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<int32_t>(verifier, VT_SERVER_ID, 4) &&
           VerifyOffsetRequired(verifier, VT_SERVICE_NAME) &&
           verifier.VerifyString(service_name()) &&
           VerifyField<float>(verifier, VT_LOGIC_FPS, 4) &&
           VerifyField<float>(verifier, VT_LOGIC_FPS_WARNING, 4) &&
           VerifyOffsetRequired(verifier, VT_NETWORK) &&
           verifier.VerifyTable(network()) &&
           verifier.EndTable();
  }
};

struct NameRegConfigBuilder {
  typedef NameRegConfig Table;
  ::flatbuffers::FlatBufferBuilder &fbb_;
  ::flatbuffers::uoffset_t start_;
  void add_server_id(int32_t server_id) {
    fbb_.AddElement<int32_t>(NameRegConfig::VT_SERVER_ID, server_id, 0);
  }
  void add_service_name(::flatbuffers::Offset<::flatbuffers::String> service_name) {
    fbb_.AddOffset(NameRegConfig::VT_SERVICE_NAME, service_name);
  }
  void add_logic_fps(float logic_fps) {
    fbb_.AddElement<float>(NameRegConfig::VT_LOGIC_FPS, logic_fps, 0.0f);
  }
  void add_logic_fps_warning(float logic_fps_warning) {
    fbb_.AddElement<float>(NameRegConfig::VT_LOGIC_FPS_WARNING, logic_fps_warning, 0.0f);
  }
  void add_network(::flatbuffers::Offset<waterside::NetworkListenConfig> network) {
    fbb_.AddOffset(NameRegConfig::VT_NETWORK, network);
  }
  explicit NameRegConfigBuilder(::flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  ::flatbuffers::Offset<NameRegConfig> Finish() {
    const auto end = fbb_.EndTable(start_);
    auto o = ::flatbuffers::Offset<NameRegConfig>(end);
    fbb_.Required(o, NameRegConfig::VT_SERVICE_NAME);
    fbb_.Required(o, NameRegConfig::VT_NETWORK);
    return o;
  }
};

inline ::flatbuffers::Offset<NameRegConfig> CreateNameRegConfig(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int32_t server_id = 0,
    ::flatbuffers::Offset<::flatbuffers::String> service_name = 0,
    float logic_fps = 0.0f,
    float logic_fps_warning = 0.0f,
    ::flatbuffers::Offset<waterside::NetworkListenConfig> network = 0) {
  NameRegConfigBuilder builder_(_fbb);
  builder_.add_network(network);
  builder_.add_logic_fps_warning(logic_fps_warning);
  builder_.add_logic_fps(logic_fps);
  builder_.add_service_name(service_name);
  builder_.add_server_id(server_id);
  return builder_.Finish();
}

inline ::flatbuffers::Offset<NameRegConfig> CreateNameRegConfigDirect(
    ::flatbuffers::FlatBufferBuilder &_fbb,
    int32_t server_id = 0,
    const char *service_name = nullptr,
    float logic_fps = 0.0f,
    float logic_fps_warning = 0.0f,
    ::flatbuffers::Offset<waterside::NetworkListenConfig> network = 0) {
  auto service_name__ = service_name ? _fbb.CreateString(service_name) : 0;
  return waterside::CreateNameRegConfig(
      _fbb,
      server_id,
      service_name__,
      logic_fps,
      logic_fps_warning,
      network);
}

inline const waterside::NameRegConfig *GetNameRegConfig(const void *buf) {
  return ::flatbuffers::GetRoot<waterside::NameRegConfig>(buf);
}

inline const waterside::NameRegConfig *GetSizePrefixedNameRegConfig(const void *buf) {
  return ::flatbuffers::GetSizePrefixedRoot<waterside::NameRegConfig>(buf);
}

inline bool VerifyNameRegConfigBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<waterside::NameRegConfig>(nullptr);
}

inline bool VerifySizePrefixedNameRegConfigBuffer(
    ::flatbuffers::Verifier &verifier) {
  return verifier.VerifySizePrefixedBuffer<waterside::NameRegConfig>(nullptr);
}

inline void FinishNameRegConfigBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<waterside::NameRegConfig> root) {
  fbb.Finish(root);
}

inline void FinishSizePrefixedNameRegConfigBuffer(
    ::flatbuffers::FlatBufferBuilder &fbb,
    ::flatbuffers::Offset<waterside::NameRegConfig> root) {
  fbb.FinishSizePrefixed(root);
}

}  // namespace waterside

#endif  // FLATBUFFERS_GENERATED_NAMEREGCONFIG_WATERSIDE_H_
