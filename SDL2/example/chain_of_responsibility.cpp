#include <cstdio>

class ProcessConcept {

 public:
  virtual ProcessConcept* Next(ProcessConcept* ) = 0;
  virtual void Process() = 0;
};

struct ProcessBase : ProcessConcept {
  ProcessConcept* Next(ProcessConcept* next) override {
    next_ = next;
    return next;
  }

  void Process() override {
    if (next_ != nullptr) {
      next_->Process();
    }
  }

  ProcessConcept* next_;
};

struct QueryProcess : ProcessBase {

  void Process() {
    std::printf("QueryProcess start\n");
    // next_->Process();
    ProcessBase::Process();
    std::printf("QueryProcess Finish\n");
  }
};

struct ParseProcess : ProcessBase {
  void Process() {
    std::printf("ParseProcess start\n");
    ProcessBase::Process();
    std::printf("ParseProcess Finish\n");
  }
};

struct OptimizeProcess : ProcessBase {
  void Process() {
    std::printf("OptimizeProcess start\n");
    ProcessBase::Process();
    std::printf("OptimizeProcess Finish\n");
  }
};


int main() {
  QueryProcess* qp = new QueryProcess;
  ParseProcess* pp = new ParseProcess;
  OptimizeProcess* op = new OptimizeProcess;

  qp->Next(pp)->Next(op);


  qp->Process();
  std::printf("\n");
  pp->Process();
  std::printf("\n");
  op->Process();

  return 0;
}
