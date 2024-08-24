// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: procmon.proto

#include "procmon.pb.h"
#include "procmon.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace procmon {

static const char* ProcmonService_method_names[] = {
  "/procmon.ProcmonService/GetSystemStatus",
  "/procmon.ProcmonService/GetNftTraffic",
};

std::unique_ptr< ProcmonService::Stub> ProcmonService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< ProcmonService::Stub> stub(new ProcmonService::Stub(channel, options));
  return stub;
}

ProcmonService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_GetSystemStatus_(ProcmonService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_GetNftTraffic_(ProcmonService_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  {}

::grpc::Status ProcmonService::Stub::GetSystemStatus(::grpc::ClientContext* context, const ::procmon::StatusRequest& request, ::procmon::StatusResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::procmon::StatusRequest, ::procmon::StatusResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetSystemStatus_, context, request, response);
}

void ProcmonService::Stub::async::GetSystemStatus(::grpc::ClientContext* context, const ::procmon::StatusRequest* request, ::procmon::StatusResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::procmon::StatusRequest, ::procmon::StatusResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetSystemStatus_, context, request, response, std::move(f));
}

void ProcmonService::Stub::async::GetSystemStatus(::grpc::ClientContext* context, const ::procmon::StatusRequest* request, ::procmon::StatusResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetSystemStatus_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::procmon::StatusResponse>* ProcmonService::Stub::PrepareAsyncGetSystemStatusRaw(::grpc::ClientContext* context, const ::procmon::StatusRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::procmon::StatusResponse, ::procmon::StatusRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetSystemStatus_, context, request);
}

::grpc::ClientAsyncResponseReader< ::procmon::StatusResponse>* ProcmonService::Stub::AsyncGetSystemStatusRaw(::grpc::ClientContext* context, const ::procmon::StatusRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetSystemStatusRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::Status ProcmonService::Stub::GetNftTraffic(::grpc::ClientContext* context, const ::procmon::NftTrafficRequest& request, ::procmon::NftTrafficResponse* response) {
  return ::grpc::internal::BlockingUnaryCall< ::procmon::NftTrafficRequest, ::procmon::NftTrafficResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_GetNftTraffic_, context, request, response);
}

void ProcmonService::Stub::async::GetNftTraffic(::grpc::ClientContext* context, const ::procmon::NftTrafficRequest* request, ::procmon::NftTrafficResponse* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::procmon::NftTrafficRequest, ::procmon::NftTrafficResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetNftTraffic_, context, request, response, std::move(f));
}

void ProcmonService::Stub::async::GetNftTraffic(::grpc::ClientContext* context, const ::procmon::NftTrafficRequest* request, ::procmon::NftTrafficResponse* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_GetNftTraffic_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::procmon::NftTrafficResponse>* ProcmonService::Stub::PrepareAsyncGetNftTrafficRaw(::grpc::ClientContext* context, const ::procmon::NftTrafficRequest& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::procmon::NftTrafficResponse, ::procmon::NftTrafficRequest, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_GetNftTraffic_, context, request);
}

::grpc::ClientAsyncResponseReader< ::procmon::NftTrafficResponse>* ProcmonService::Stub::AsyncGetNftTrafficRaw(::grpc::ClientContext* context, const ::procmon::NftTrafficRequest& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncGetNftTrafficRaw(context, request, cq);
  result->StartCall();
  return result;
}

ProcmonService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      ProcmonService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< ProcmonService::Service, ::procmon::StatusRequest, ::procmon::StatusResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](ProcmonService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::procmon::StatusRequest* req,
             ::procmon::StatusResponse* resp) {
               return service->GetSystemStatus(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      ProcmonService_method_names[1],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< ProcmonService::Service, ::procmon::NftTrafficRequest, ::procmon::NftTrafficResponse, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](ProcmonService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::procmon::NftTrafficRequest* req,
             ::procmon::NftTrafficResponse* resp) {
               return service->GetNftTraffic(ctx, req, resp);
             }, this)));
}

ProcmonService::Service::~Service() {
}

::grpc::Status ProcmonService::Service::GetSystemStatus(::grpc::ServerContext* context, const ::procmon::StatusRequest* request, ::procmon::StatusResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status ProcmonService::Service::GetNftTraffic(::grpc::ServerContext* context, const ::procmon::NftTrafficRequest* request, ::procmon::NftTrafficResponse* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace procmon

