#include "TcpNetwork.h"
#include "async_simple/coro/SyncAwait.h"

using boost::asio::ip::tcp;


int testaa(int a, int b);
void testfb(const waterside::RpcPacketConnectReply* fb);

async_simple::coro::Lazy<void> test_rpc_call(waterside::TcpNetwork& net, waterside::SessionID sessionId)
{
    auto result = co_await net.call<testaa>(sessionId, 100, 200);
    if (result.has_value())
    {
        auto xx = result.value();

        std::cout << "xxxxxx:" << xx << "\n\n";
    }

    flatbuffers::FlatBufferBuilder fbb;
    auto root = waterside::CreateRpcPacketConnectReplyDirect(fbb,
        "bForceDisconnet", 222);
    fbb.Finish(root);

    co_await net.call<testfb>(sessionId, &fbb);
}

int main(int argc, char *argv[]) {
    try {
        waterside::TcpNetwork net(1, tcp::endpoint(tcp::v4(), 9981));
        net.start([]() {}).start([](async_simple::Try<void> Result) {
            if (Result.hasError())
                std::cout << "Error Happened in task.\n";
            else
                std::cout << "task completed successfully.\n";
            });

        net.connect("127.0.0.1", "9980").start([&net](async_simple::Try<std::pair<boost::system::error_code, waterside::SessionID>> Result) {
            if (Result.hasError())
            {
                std::exception_ptr error = Result.getException();
                std::cout << "Error Happened in task.\n";
            }
            else
            {
                std::cout << "task completed successfully.\n";
                auto [ec, sesionId] = Result.value();
                if (ec)
                {
                    //
                }
                else
                {
                    test_rpc_call(net, sesionId).start([](async_simple::Try<void> Result) {
                        if (Result.hasError())
                        {
                            std::exception_ptr error = Result.getException();
                            //(*error);
                            std::cout << "Error Happened in task.\n";
                        }
                        else
                            std::cout << "task completed successfully.\n";
                        });
                }
            }
            });

        while (true)
        {
            net.onProcessPacket();

            std::this_thread::sleep_for(std::chrono::duration<float>(0.1f));
        }

        net.release();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}