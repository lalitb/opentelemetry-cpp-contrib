#include "../include/test_lib.h"
#include "../include/process_metrics_factory.h"
#  include "opentelemetry/context/context.h"
#  include "opentelemetry/metrics/provider.h"


#include <opentelemetry/sdk/metrics/async_instruments.h>
#include <iostream>
namespace nostd       = opentelemetry::nostd;
namespace metrics_api = opentelemetry::metrics;

opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::cpu_time_obserable_counter_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::cpu_utilization_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::memory_usage_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::memory_virtual_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::disk_io_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::network_io_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::threads_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::open_files_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::context_switches_obserable_gauge_;
opentelemetry::nostd::shared_ptr<opentelemetry::metrics::ObservableInstrument> TestLib::memory_mallinfo_obserable_gauge_;

using M = std::map<std::string, size_t>;

void TestLib::create_process_cpu_time_observable_counter()
{
    std::cout << "create process cpu time\n";
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0", "schema");
  cpu_time_obserable_counter_ = meter->CreateInt64ObservableCounter("process.cpu.time", "des", "unit");
  cpu_time_obserable_counter_->AddCallback(ProcessMetricsFactory::GetProcessCpuTime, nullptr);
  auto storage = static_cast<opentelemetry::sdk::metrics::ObservableInstrument *>(cpu_time_obserable_counter_.get())->GetMetricStorage();
}

void TestLib::create_process_cpu_utilization_observable_gauge()
{
      std::cout << "create process cpu utilization\n";

  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  cpu_utilization_obserable_gauge_ = meter->CreateDoubleObservableGauge("process.cpu.utilization", "des", "unit");
  cpu_utilization_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessCpuUtilization, nullptr);
}

void TestLib::create_process_memory_usage_observable_gauge()
{
  std::cout << "create process memory usage\n";
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  memory_usage_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.memory.usage", "des", "unit");
  memory_usage_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessMemoryUsage, nullptr);
}

void TestLib::create_process_memory_virtual_observable_gauge()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  memory_virtual_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.memory.virtual", "des", "unit");
  memory_virtual_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessMemoryVirtual, nullptr);
}

void TestLib::create_process_disk_io_observable_gauge()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  disk_io_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.disk.io", "des", "unit");
  disk_io_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessDiskIO, nullptr);
}

void TestLib::create_process_network_io_observable_gauge()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  network_io_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.network.io", "des", "unit");
  network_io_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessNetworkIO, nullptr);   
}

void TestLib::create_process_threads_observable_gauge()
{
    std::cout << "create thread usage\n";

  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  threads_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.threads", "des", "unit");
  threads_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessThreads, nullptr);
}

void TestLib::create_process_open_files_observable_gauge()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  open_files_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.open.files", "des", "unit");
  open_files_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessOpenFileDescriptors, nullptr);
}

void TestLib::create_process_context_switches_observable_gauge()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  context_switches_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.context.switches", "des", "unit");
  context_switches_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessContextSwitches, nullptr);   

}

void TestLib::create_process_memory_mallinfo_observable_gauge()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  memory_mallinfo_obserable_gauge_ = meter->CreateInt64ObservableGauge("process.memory.usage.mallinfo", "des", "unit");
  memory_mallinfo_obserable_gauge_->AddCallback(ProcessMetricsFactory::GetProcessMemoryMallinfo, nullptr);   
}


static void foo() {

}

void TestLib::start_stress_test()
{
  auto provider                               = metrics_api::Provider::GetMeterProvider();
  nostd::shared_ptr<metrics_api::Meter> meter = provider->GetMeter("process.metrics", "1.2.0");
  auto instrument = meter->CreateDoubleCounter("counter1", "counter1_description", "counter1_unit");
  constexpr size_t TOTAL_MEASUREMENTS = 100000;
  constexpr int NUM_CORES=8;
  std::vector<std::thread> measurementThreads;
  std::atomic<long long> total_processed {0l};
  long long max_processed = 100000;
  std::atomic<bool> stop {false};
   auto begin = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_CORES; i++){
    measurementThreads.push_back(std::thread( [&instrument, i, &total_processed, &stop, &max_processed /*, &measurements*/]() {
      while (total_processed <= max_processed) {
      //  for (int j = 0 ; j < TOTAL_MEASUREMENTS ; j++ ){
       // size_t val1 = rand() % 10;
	//size_t val2 = rand() % 10;
	// size_t val3 = rand() % 10;
    	// instrument->Record(1.0,
         //        opentelemetry::common::KeyValueIterableView<M>({{"dim1", val1},{"dim2", val2}, {"dim3", val3}}),
       //          opentelemetry::context::Context{});
       for (double val1 = 0 ; val1 <= 10; val1++)
          instrument->Add(1.0, {{"dim1", val1}}, opentelemetry::context::Context{});
         //foo();
          //if (j % 1000 == 0){
            total_processed+=1;
            //total_processed.fetch_add(1, std::memory_order_relaxed);
           // std::this_thread::sleep_for(std::chrono::nanoseconds(10));
          //}
       // }
        //std::this_thread::sleep_for(std::chrono::nanoseconds(100));
       // std::cout << "\nprocesing..\n";
      }
      std::cout << "\n existing..." ;;
    }));
  }
  //std::this_thread::sleep_for(std::chrono::seconds(60));
  //stop = true;

  for (auto &th: measurementThreads){
    th.join();
  }
      auto end = std::chrono::high_resolution_clock::now();
   auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
    std::cout << " Total: " << total_processed<<"\n";
   printf("Time measured: %.3f seconds.\n", elapsed.count() * 1e-9);
}
