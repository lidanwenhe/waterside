#include "TcpNetwork.h"
#include "async_simple/coro/SyncAwait.h"

using asio::ip::tcp;

int testaa(int a, int b)
{
	int x = a + b;
	return x;
}

void testfb(const waterside::RpcPacketConnectReply* fb)
{
	std::cout << "testfb:" << fb->port() << std::endl;
}

int main(int argc, char* argv[]) {

    waterside::TcpNetwork net(std::thread::hardware_concurrency(), tcp::endpoint(tcp::v4(), 9980));
	
	net.registHandler<testaa>();
	net.registHandler<testfb>();

	//async_simple::coro::syncAwait(net.start());
    net.start().start([](async_simple::Try<void> Result) {
		if (Result.hasError())
			std::cout << "Error Happened in task.\n";
		else
			std::cout << "task completed successfully.\n";
		});

	while (true)
	{
		net.onProcessPacket();

		std::this_thread::sleep_for(std::chrono::duration<float>(0.1f));
	}

    net.stop();

    return 0;
}
