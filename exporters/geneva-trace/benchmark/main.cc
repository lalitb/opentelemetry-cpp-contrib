// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <windows.h>

#include <opentelemetry/exporters/geneva/geneva_logger_exporter.h>
#include <opentelemetry/exporters/geneva/geneva_tracer_exporter.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace otel = opentelemetry;
namespace etw  = otel::exporter::etw;

constexpr char kProvider[]   = "{8DAD5A91-836E-4FBC-94D7-9ABACCD34BE5}";
constexpr size_t kIterations = 10000;
constexpr size_t kRepeats    = 7;
constexpr size_t kWarmup     = 1000;

template <typename F>
double Time(size_t count, F operation)
{
  const auto start = std::chrono::steady_clock::now();
  for (size_t i = 0; i < count; ++i)
  {
    operation(i);
  }
  return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start)
             .count() /
         static_cast<double>(count);
}

void Report(const char *name, std::vector<double> samples)
{
  std::cout << name << " samples_ns:";
  for (double sample : samples)
  {
    std::cout << ' ' << sample;
  }
  std::sort(samples.begin(), samples.end());
  const double median = samples[samples.size() / 2];
  std::cout << "\n"
            << name << ": median=" << median << " ns/op, min=" << samples.front()
            << ", max=" << samples.back() << ", ops/sec=" << 1e9 / median << '\n';
}

int main()
{
  etw::TelemetryProviderOptions options{{"enableTraceId", true}, {"enableSpanId", true}};
  etw::LoggerProvider logger_provider(options);
  etw::TracerProvider tracer_provider(options);
  auto logger = logger_provider.GetLogger(kProvider);
  auto tracer = tracer_provider.GetTracer(kProvider);
  otel::ETWProvider probe;
  auto &handle = probe.open(kProvider, otel::ETWProvider::ETW_MANIFEST);
  if (!EventProviderEnabled(handle.providerHandle, 0, 0))
  {
    std::cerr << "ETW provider is disabled. Start the session using run.ps1 first.\n";
    return 1;
  }

  const std::string body(64, 'x');
  const std::vector<std::pair<std::string, otel::common::AttributeValue>> attributes{
      {"string_key", otel::nostd::string_view("benchmark-value")},
      {"integer_key", int64_t{42}},
      {"bool_key", true}};
  auto make_record = [&] {
    auto record = logger->CreateLogRecord();
    record->SetBody(otel::nostd::string_view(body));
    record->SetSeverity(otel::logs::Severity::kInfo);
    for (const auto &attribute : attributes)
    {
      record->SetAttribute(attribute.first, attribute.second);
    }
    return record;
  };
  std::vector<double> log_samples, span_samples, log_full_samples, span_full_samples;
  for (size_t repeat = 0; repeat <= kRepeats; ++repeat)
  {
    const size_t count = repeat == 0 ? kWarmup : kIterations;
    std::vector<otel::nostd::unique_ptr<otel::logs::LogRecord>> records;
    std::vector<otel::nostd::shared_ptr<otel::trace::Span>> spans;
    records.reserve(count);
    spans.reserve(count);
    // Prepare distinct records/spans outside the sender-only timers.
    for (size_t i = 0; i < count; ++i)
    {
      records.emplace_back(make_record());
      spans.emplace_back(tracer->StartSpan("benchmark-span", attributes));
    }
    const double log_ns =
        Time(count, [&](size_t i) { logger->EmitLogRecord(std::move(records[i])); });
    const double span_ns      = Time(count, [&](size_t i) { spans[i]->End(); });
    const double log_full_ns  = Time(count, [&](size_t) { logger->EmitLogRecord(make_record()); });
    const double span_full_ns = Time(count, [&](size_t) {
      auto span = tracer->StartSpan("benchmark-span", attributes);
      span->End();
    });
    if (repeat != 0)
    {
      log_samples.push_back(log_ns);
      span_samples.push_back(span_ns);
      log_full_samples.push_back(log_full_ns);
      span_full_samples.push_back(span_full_ns);
    }
  }
  Report("log_emit_3attrs_64B", log_samples);
  Report("log_create_emit_3attrs_64B", log_full_samples);
  Report("span_end_3attrs", span_samples);
  Report("span_start_end_3attrs", span_full_samples);
  std::cout << "Expected ETW events: " << 4 * (kWarmup + kRepeats * kIterations) << '\n';
  probe.close(handle);
}
