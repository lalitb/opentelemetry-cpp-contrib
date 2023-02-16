#include "opentelemetry/exporters/geneva/metrics/exporter.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/aggregation/default_aggregation.h"
#include "opentelemetry/sdk/metrics/aggregation/histogram_aggregation.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include <memory>
#include <thread>

#include "../include/test_lib.h"

#  include <memory>
#  include <thread>

namespace metrics_sdk      = opentelemetry::sdk::metrics;
namespace nostd           = opentelemetry::nostd;
namespace common          = opentelemetry::common;
namespace geneva_exporter = opentelemetry::exporter::geneva::metrics;
namespace metrics_api     = opentelemetry::metrics;

namespace
{

const std::string kUnixDomainPath = "/home/labhas/mdm/sock/mdm_ifx.socket";
const std::string kNamespaceName = "test_ns";

void initMetrics(const std::string &name ="metrics_stress_14_2", const std::string &account_name = "IFxSDKCI")
{

  std::string conn_string =
      "Account=" + account_name + ";Namespace=" + kNamespaceName;
#ifndef _WIN32
  conn_string = "Endpoint=unix://" + kUnixDomainPath + ";" + conn_string;
#endif

  geneva_exporter::ExporterOptions options{conn_string};
  std::unique_ptr<metrics_sdk::PushMetricExporter> exporter{
      new geneva_exporter::Exporter(options)};

  std::string version{"1.2.0"};
  std::string schema{"https://opentelemetry.io/schemas/1.2.0"};


metrics_sdk::PeriodicExportingMetricReaderOptions reader_options;
  reader_options.export_interval_millis = std::chrono::milliseconds(60 *1000);
  reader_options.export_timeout_millis = std::chrono::milliseconds(500);
  std::unique_ptr<metrics_sdk::MetricReader> reader{
      new metrics_sdk::PeriodicExportingMetricReader(std::move(exporter),
                                                    reader_options)};
  auto provider = std::shared_ptr<metrics_api::MeterProvider>(
      new metrics_sdk::MeterProvider());
  auto p = std::static_pointer_cast<metrics_sdk::MeterProvider>(provider);
  p->AddMetricReader(std::move(reader));

#if 0
  //process.cpu.time view
  std::unique_ptr<metric_sdk::InstrumentSelector> observable_instrument_selector{
      new metric_sdk::InstrumentSelector(metric_sdk::InstrumentType::kObservableCounter,
                                         "process.cpu.time")};
  std::unique_ptr<metric_sdk::MeterSelector> observable_meter_selector{
      new metric_sdk::MeterSelector("process.cpu.time", "1.2.0", "schema")};
  std::unique_ptr<metric_sdk::View> observable_sum_view{
      new metric_sdk::View{"process.metrics", "des", metric_sdk::AggregationType::kSum}};
  p->AddView(std::move(observable_instrument_selector), std::move(observable_meter_selector),
             std::move(observable_sum_view));

#endif
  metrics_api::Provider::SetMeterProvider(provider);

}
}  // namespace

int main(int argc, char **argv)
{
  std::vector<std::string> args(argv + 1, argv + argc);
  std::string example_type;
  if (args.size() == 0) {
      std::cout <<  "Options -- \n" ;
      std::cout <<  "\t --process.cpu.time --process.cpu.utilization --process.memory.usage --process.memory.virtual\n" ;
      std::cout <<  "\t --process.disk.io --process.network.io --process.threads --process.open.files --process.context.switches --stress.test\n";
      std::cout << "\n";
      exit(1); 
  }
  initMetrics();

  for (auto &arg: args) {
    if (arg == "--process.cpu.time") {
      TestLib::create_process_cpu_time_observable_counter();
    }
    else if (arg == "--process.cpu.utilization")
    {
      TestLib::create_process_cpu_utilization_observable_gauge();
    }
    else if (arg == "--process.memory.usage")
    {
      TestLib::create_process_memory_usage_observable_gauge();
    }
    else if (arg == "--process.memory.virtual")
    {
      TestLib::create_process_memory_virtual_observable_gauge();
    }
    else if (arg == "--process.disk.io")
    {
      TestLib::create_process_disk_io_observable_gauge();
    }
    else if (arg == "--process.network.io")
    {
      TestLib::create_process_network_io_observable_gauge();
    }
    else if (arg == "--process.threads")
    {
      TestLib::create_process_threads_observable_gauge();
    }
    else if(arg == "--process.open.files")
    {
      TestLib::create_process_open_files_observable_gauge();
    }
    else if (arg == "--process.context.switches")
    {
      TestLib::create_process_context_switches_observable_gauge();
    }
    else if (arg == "--process.memory.usage.mallinfo")
    {
      TestLib::create_process_memory_mallinfo_observable_gauge();
    }
    else if (arg == "--stress.test")
    {
      TestLib::start_stress_test();
    }
    else {
      std::cout << " Invalid command, exiting..";
      std::cout <<  "\tOptions -- \n" ;
      std::cout <<  "\t\t --process.cpu.time --process.cpu.utilization --process.memory.usage --process.memory.virtual\n" ;
      std::cout <<  "\t\t --process.disk.io --process.network.io --process.threads --process.open.files --process.context.switches --stress.test\n";
      exit(1);
    }
  }

  while(true){
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}