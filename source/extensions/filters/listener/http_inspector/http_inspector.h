#pragma once

#include "envoy/event/file_event.h"
#include "envoy/event/timer.h"
#include "envoy/network/filter.h"
#include "envoy/stats/scope.h"
#include "envoy/stats/stats_macros.h"

#include "common/common/logger.h"

#include "absl/container/flat_hash_set.h"

namespace Envoy {
namespace Extensions {
namespace ListenerFilters {
namespace HttpInspector {

/**
 * All stats for the http inspector. @see stats_macros.h
 */
#define ALL_HTTP_INSPECTOR_STATS(COUNTER)                                                          \
  COUNTER(read_error)                                                                              \
  COUNTER(http10_found)                                                                            \
  COUNTER(http11_found)                                                                            \
  COUNTER(http2_found)                                                                             \
  COUNTER(http_not_found)

/**
 * Definition of all stats for the Http inspector. @see stats_macros.h
 */
struct HttpInspectorStats {
  ALL_HTTP_INSPECTOR_STATS(GENERATE_COUNTER_STRUCT)
};

/**
 * Global configuration for http inspector.
 */
class Config {
public:
  Config(Stats::Scope& scope);

  const HttpInspectorStats& stats() const { return stats_; }

  static constexpr uint32_t MAX_INSPECT_SIZE = 8192;

private:
  HttpInspectorStats stats_;
};

using ConfigSharedPtr = std::shared_ptr<Config>;

/**
 * Http inspector listener filter.
 */
class Filter : public Network::ListenerFilter, Logger::Loggable<Logger::Id::filter> {
public:
  Filter(const ConfigSharedPtr config);

  // Network::ListenerFilter
  Network::FilterStatus onAccept(Network::ListenerFilterCallbacks& cb) override;

private:
  static const absl::string_view HTTP2_CONNECTION_PREFACE;

  void onRead();
  void done(bool success);
  void parseHttpHeader(absl::string_view data);

  const absl::flat_hash_set<std::string>& httpProtocols() const;

  ConfigSharedPtr config_;
  Network::ListenerFilterCallbacks* cb_{nullptr};
  Event::FileEventPtr file_event_;
  absl::string_view protocol_;

  // Use static thread_local to avoid allocating buffer over and over again.
  static thread_local uint8_t buf_[Config::MAX_INSPECT_SIZE];
};

} // namespace HttpInspector
} // namespace ListenerFilters
} // namespace Extensions
} // namespace Envoy
