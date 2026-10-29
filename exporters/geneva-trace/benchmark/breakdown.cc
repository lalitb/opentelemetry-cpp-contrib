// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <windows.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

// Generated ETW headers call these probes. Intervals do not overlap.
namespace end_probe
{
enum class Stage
{
  AttributeCopy,
  IdFields,
  Timestamps,
  FinalFields,
  Encoding,
  EtwWrite,
  Other,
  Count
};
constexpr size_t kStages = static_cast<size_t>(Stage::Count);
using Clock              = std::chrono::steady_clock;
std::array<int64_t, kStages> elapsed{};
std::array<size_t, kStages> visits{};
Clock::time_point last;
Stage current = Stage::Other;
bool active   = false;

void Switch(Stage next)
{
  if (active)
  {
    const auto now = Clock::now();
    const auto i   = static_cast<size_t>(current);
    elapsed[i] += std::chrono::duration_cast<std::chrono::nanoseconds>(now - last).count();
    ++visits[i];
    last    = now;
    current = next;
  }
}

void Begin()
{
  current = Stage::Other;
  active  = true;
  last    = Clock::now();
}

void End()
{
  Switch(Stage::Other);
  active = false;
}

void Reset()
{
  elapsed.fill(0);
  visits.fill(0);
}
}  // namespace end_probe

#include <opentelemetry/exporters/geneva/geneva_tracer_exporter.h>

namespace otel = opentelemetry;
namespace etw  = otel::exporter::etw;

constexpr char kProvider[]   = "{8DAD5A91-836E-4FBC-94D7-9ABACCD34BE5}";
constexpr size_t kIterations = 10000;
constexpr size_t kRepeats    = 7;
constexpr size_t kWarmup     = 1000;

void Report(const char *name, std::vector<double> samples)
{
  std::cout << name << " samples_ns:";
  for (double sample : samples)
  {
    std::cout << ' ' << sample;
  }
  std::sort(samples.begin(), samples.end());
  std::cout << "\n" << name << ": median=" << samples[samples.size() / 2] << " ns/span\n";
}

int main()
{
  etw::TelemetryProviderOptions options{{"enableTraceId", true}, {"enableSpanId", true}};
  etw::TracerProvider tracer_provider(options);
  auto tracer = tracer_provider.GetTracer(kProvider);
  otel::ETWProvider probe;
  auto &handle = probe.open(kProvider, otel::ETWProvider::ETW_MANIFEST);
  if (!EventProviderEnabled(handle.providerHandle, 0, 0))
  {
    std::cerr << "ETW provider is disabled. Start the session using run.ps1 -Breakdown.\n";
    return 1;
  }

  constexpr size_t kCalibration = 100000;
  std::vector<double> overhead;
  for (size_t repeat = 0; repeat < kRepeats; ++repeat)
  {
    end_probe::Reset();
    for (size_t i = 0; i < kCalibration; ++i)
    {
      end_probe::Begin();
      end_probe::Switch(end_probe::Stage::AttributeCopy);
      end_probe::Switch(end_probe::Stage::IdFields);
      end_probe::Switch(end_probe::Stage::Timestamps);
      end_probe::Switch(end_probe::Stage::FinalFields);
      end_probe::Switch(end_probe::Stage::Encoding);
      end_probe::Switch(end_probe::Stage::EtwWrite);
      end_probe::Switch(end_probe::Stage::Other);
      end_probe::End();
    }
    overhead.push_back(static_cast<double>(std::accumulate(end_probe::elapsed.begin(),
                                                           end_probe::elapsed.end(), int64_t{0})) /
                       kCalibration);
  }
  Report("empty_probe_sequence", overhead);

  const std::vector<std::pair<std::string, otel::common::AttributeValue>> attributes{
      {"string_key", otel::nostd::string_view("benchmark-value")},
      {"integer_key", int64_t{42}},
      {"bool_key", true}};
  std::array<std::vector<double>, end_probe::kStages> samples;
  std::vector<double> totals;
  for (size_t repeat = 0; repeat <= kRepeats; ++repeat)
  {
    const size_t count = repeat == 0 ? kWarmup : kIterations;
    std::vector<otel::nostd::shared_ptr<otel::trace::Span>> spans;
    spans.reserve(count);
    for (size_t i = 0; i < count; ++i)
    {
      spans.emplace_back(tracer->StartSpan("benchmark-span", attributes));
    }
    end_probe::Reset();
    for (auto &span : spans)
    {
      end_probe::Begin();
      span->End();
      end_probe::End();
    }
    for (size_t stage = 0; stage < end_probe::kStages; ++stage)
    {
      const size_t expected =
          stage == static_cast<size_t>(end_probe::Stage::Other) ? 2 * count : count;
      if (end_probe::visits[stage] != expected)
      {
        std::cerr << "Unexpected stage visit count: " << stage << '\n';
        return 1;
      }
      if (repeat != 0)
      {
        samples[stage].push_back(static_cast<double>(end_probe::elapsed[stage]) / count);
      }
    }
    if (repeat != 0)
    {
      totals.push_back(static_cast<double>(std::accumulate(end_probe::elapsed.begin(),
                                                           end_probe::elapsed.end(), int64_t{0})) /
                       count);
    }
  }
  constexpr std::array<const char *, end_probe::kStages> names{
      "attribute_copy",
      "ids_and_envelope_fields",
      "two_timestamps_and_field_assignment",
      "duration_kind_status_fields",
      "tld_metadata_payload_encoding",
      "etw_write",
      "cleanup_and_other"};
  for (size_t stage = 0; stage < end_probe::kStages; ++stage)
  {
    Report(names[stage], samples[stage]);
  }
  Report("instrumented_span_end_total", totals);
  std::cout << "Stage times include allocations and probe overhead; overhead is not subtracted.\n"
            << "Expected ETW events: " << kWarmup + kRepeats * kIterations << '\n';
  probe.close(handle);
}
