#include <iostream>

#include <map>
#include <string>
#include <vector>

#include "opentelemetry/exporters/geneva/metrics/exporter.h"

#include "common/socket_server.h"
#include "common/generate_metrics.h"
#include "decoder/ifx_metrics_bin.h"
#include "decoder/kaitai/kaitaistream.h"
#include <gtest/gtest.h>

using namespace SOCKET_SERVER_NS;
using namespace kaitai;
using namespace opentelemetry::sdk::metrics;
using namespace opentelemetry::exporter::geneva::metrics;

const std::string kUnixDomainPath = "/tmp/ifx_unix_socket";
const std::string kNamespaceName = "test_ns";
const std::string kAccountName = "test_account";

// "busy sleep" while suggesting that other threads run
// for a small amount of time
template <typename timeunit> void yield_for(timeunit duration) {
  auto start = std::chrono::high_resolution_clock::now();
  auto end = start + duration;
  do {
    std::this_thread::yield();
  } while (std::chrono::high_resolution_clock::now() < end);
}

struct TestServer {
  SocketServer &server;
  std::atomic<uint32_t> count{0};

  TestServer(SocketServer &server) : server(server) {
    server.onRequest = [&](SocketServer::Connection &conn) {        
      try {
        std::cout << "------>RECEIVED:" << conn.response_buffer.size() << "\n";
        /*auto j = nlohmann::json::from_msgpack(msg);
        std::cout << "[" << count.fetch_add(1)
                  << "] SocketServer received payload: " << std::endl
                  << j.dump(2) << std::endl;

        conn.response_buffer = j.dump(2);*/
        conn.state.insert(SocketServer::Connection::Responding);
        conn.request_buffer.clear();
      } catch (std::exception &) {
        conn.state.insert(SocketServer::Connection::Receiving);
        // skip invalid payload
      }
    };
  }

  void Start() { server.Start(); }

  void Stop() { server.Stop(); }

  void WaitForEvents(uint32_t expectedCount, uint32_t timeout) {
    if (count.load() != expectedCount) {
      yield_for(std::chrono::milliseconds(timeout));
    }
    EXPECT_EQ(count.load(), expectedCount);
  }
};


TEST(GenevaMetricsExporter, BasicTests) 
{
  bool isRunning = true;

  // Start test server
  SocketAddr destination(kUnixDomainPath.data(), true);
  SocketParams params{AF_UNIX, SOCK_STREAM, 0};
  SocketServer socketServer(destination, params);
  //TestServer testServer(socketServer);
  //testServer.Start();
  std::cout << " here \n" ; //LALIT

  yield_for(std::chrono::milliseconds(500));
  std::cout << "after yield\n"; //LALIT

  // conn_string: `Endpoint=unix:{udsPath};Account={MetricAccount};Namespace={MetricNamespace}`
  std::string conn_string = "Endpoint=unix://" + kUnixDomainPath + ";Account=" + kAccountName + ";Namespace=" + kNamespaceName;

  auto metric_data = GenerateSumDataMetrics();
  std::cout << "generated metrics\n"; //LALIT
 ExporterOptions options{conn_string};
 std::cout << "after getting options\n"; //LALIT
 opentelemetry::exporter::geneva::metrics::Exporter exporter(options);
 std::cout << "Sending metrics\n"; //LALIT
 exporter.Export(metric_data);
  yield_for(std::chrono::milliseconds(2000));

 //testServer.Stop();

}
