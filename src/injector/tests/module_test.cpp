#include "src/injector/injector.hpp"
#include "src/testing/unit_test.hpp"

#include <memory>
#include <sstream>
#include <string>

using namespace std;
using namespace unit_test;

/****************
 * Dependencies *
 ****************/

class TaskScheduler final {
public:
  INJECT(TaskScheduler()) = default;
  TaskScheduler(const TaskScheduler&) = delete;
  TaskScheduler(TaskScheduler&&) = delete;
};

class RpcValidator {
public:
  RpcValidator(const char* serviceName) : serviceName_(serviceName){};

  string serviceName_;
};

/*******************
 * Service Classes *
 *******************/

class GenericService {
public:
  GenericService(
      size_t timeoutMs, shared_ptr<TaskScheduler> scheduler, unique_ptr<RpcValidator>&& validator)
      : timeoutMs_(timeoutMs), scheduler_(move(scheduler)), validator_(move(validator)) {}
  virtual ~GenericService(){};
  GenericService(const GenericService&) = delete;
  GenericService(GenericService&&) = delete;

  virtual string callService() = 0;

  size_t timeoutMs_;
  shared_ptr<TaskScheduler> scheduler_;
  unique_ptr<RpcValidator> validator_;
};

class SecretService final : public GenericService {
public:
  ANNOTATED(SecretService)
  INJECT(SecretService(
      size_t timeoutMs, shared_ptr<TaskScheduler> scheduler, unique_ptr<RpcValidator>&& validator))
      : GenericService(timeoutMs, move(scheduler), move(validator)) {}
  SecretService(const SecretService&) = delete;
  SecretService(SecretService&&) = delete;

  string callService() override { return "Called SecretService"; }
};

class CoolService final : public GenericService {
public:
  ANNOTATED(CoolService)
  INJECT(CoolService(
      size_t timeoutMs, shared_ptr<TaskScheduler> scheduler, unique_ptr<RpcValidator>&& validator))
      : GenericService(timeoutMs, move(scheduler), move(validator)) {}
  CoolService(const CoolService&) = delete;
  CoolService(CoolService&&) = delete;

  string callService() override { return "Called CoolService"; }
};

/***********
 * Modules *
 ***********/

class TaskSchedulerModule : public injector::BindingModule {
public:
  void install() override { injector::bindToObject(make_shared<TaskScheduler>()); }
};

class SecretServiceModule : public injector::BindingModule {
public:
  void install() override {
    injector::bindToBase<GenericService, SecretService>();
    injector::bindToObject<size_t, SecretService>(1000);
    TaskSchedulerModule().install();
    injector::bindToSupplier<RpcValidator>(
        []() { return make_unique<RpcValidator>("SecretService"); });
  };
};

class CoolServiceModule : public injector::BindingModule {
public:
  void install() override {
    injector::bindToBase<GenericService, CoolService>();
    injector::bindToObject<size_t, CoolService>(2000);
    TaskSchedulerModule().install();
    injector::bindToSupplier<RpcValidator>(
        []() { return make_unique<RpcValidator>("CoolService"); });
  };
};

BEFORE(setup) { injector::clearBindings(); }

TEST(injectSecretService) {
  SecretServiceModule().install();

  unique_ptr<GenericService> service = injector::inject<unique_ptr<GenericService>>();

  assertEquals(1000UL, service->timeoutMs_);
  assertEquals("SecretService", service->validator_->serviceName_);
  assertEquals("Called SecretService", service->callService());
}

TEST(injectCoolService) {
  CoolServiceModule().install();

  shared_ptr<GenericService> service = injector::inject<shared_ptr<GenericService>>();

  assertEquals(2000UL, service->timeoutMs_);
  assertEquals("CoolService", service->validator_->serviceName_);
  assertEquals("Called CoolService", service->callService());
}


int main() { return runTests(); }
