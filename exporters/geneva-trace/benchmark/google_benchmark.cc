// Copyright The OpenTelemetry Authors
// SPDX-License-Identifier: Apache-2.0

#include <windows.h>

#include <benchmark/benchmark.h>
#include <opentelemetry/exporters/geneva/geneva_tracer_exporter.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace otel = opentelemetry;
namespace etw  = otel::exporter::etw;

constexpr char kProvider[] = "{8DAD5A91-836E-4FBC-94D7-9ABACCD34BE5}";
constexpr size_t kWarmup   = 1000;
uint64_t expected_events   = 0;

void BM_SpanCreateToEtw(benchmark::State &state)
{
  etw::TelemetryProviderOptions options{{"enableTraceId", true}, {"enableSpanId", true}};
  etw::TracerProvider tracer_provider(options);
  auto tracer = tracer_provider.GetTracer(kProvider);
  const std::vector<std::pair<std::string, otel::common::AttributeValue>> attributes{
      {"string_key", otel::nostd::string_view("benchmark-value")},
      {"integer_key", int64_t{42}},
      {"bool_key", true}};
  auto emit_span = [&] {
    auto span = tracer->StartSpan("benchmark-span", attributes);
    span->End();
  };
  // Setup and warm-up are before the state loop, outside Google Benchmark's timer.
  for (size_t i = 0; i < kWarmup; ++i)
  {
    emit_span();
  }
  for (auto _ : state)
  {
    emit_span();
  }
  expected_events += kWarmup + static_cast<uint64_t>(state.iterations());
  state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_SpanCreateToEtw)
    ->Iterations(10000)
    ->Repetitions(7)
    ->UseRealTime()
    ->Unit(benchmark::kNanosecond);

int main(int argc, char **argv)
{
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
  {
    return 1;
  }
  otel::ETWProvider probe;
  auto &handle = probe.open(kProvider, otel::ETWProvider::ETW_MANIFEST);
  if (!EventProviderEnabled(handle.providerHandle, 0, 0))
  {
    std::cerr << "ETW provider is disabled. Start the session using run.ps1 -GoogleBenchmark.\n";
    return 1;
  }
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  std::cout << "Expected ETW events: " << expected_events << '\n';
  probe.close(handle);
}
