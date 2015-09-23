#pragma once

#include "ServerConfig.hpp"

#include <commitmanager/CommitManager.hpp>

#include <crossbow/byte_buffer.hpp>
#include <crossbow/infinio/InfinibandService.hpp>
#include <crossbow/infinio/InfinibandSocket.hpp>
#include <crossbow/infinio/RpcServer.hpp>

#include <memory>

namespace tell {
namespace commitmanager {

class ServerManager;

/**
 * @brief Handles communication with one CommitManager client
 *
 * Listens for incoming RPC requests, performs the desired action and sends the response back to the client.
 */
class ServerSocket : public crossbow::infinio::RpcServerSocket<ServerManager, ServerSocket> {
    using Base = crossbow::infinio::RpcServerSocket<ServerManager, ServerSocket>;

public:
    ServerSocket(ServerManager& manager, crossbow::infinio::InfinibandProcessor& processor,
            crossbow::infinio::InfinibandSocket socket, size_t maxBatchSize)
            : Base(manager, processor, std::move(socket), crossbow::string(), maxBatchSize) {
    }

private:
    friend Base;
    friend class ServerManager;

    void onRequest(crossbow::infinio::MessageId messageId, uint32_t messageType, crossbow::buffer_reader& message);
};

class ServerManager : public crossbow::infinio::RpcServerManager<ServerManager, ServerSocket> {
    using Base = crossbow::infinio::RpcServerManager<ServerManager, ServerSocket>;

public:
    ServerManager(crossbow::infinio::InfinibandService& service, const ServerConfig& config);

private:
    friend Base;
    friend class ServerSocket;

    ServerSocket* createConnection(crossbow::infinio::InfinibandSocket socket, const crossbow::string& data);

    void onMessage(ServerSocket* con, crossbow::infinio::MessageId messageId, uint32_t messageType,
            crossbow::buffer_reader& message);

    void handleStartTransaction(ServerSocket* con, crossbow::infinio::MessageId messageId,
            crossbow::buffer_reader& message);

    void handleCommitTransaction(ServerSocket* con, crossbow::infinio::MessageId messageId,
            crossbow::buffer_reader& message);

    std::unique_ptr<crossbow::infinio::InfinibandProcessor> mProcessor;

    size_t mMaxBatchSize;

    CommitManager mCommitManager;
};

} // namespace commitmanager
} // namespace tell
