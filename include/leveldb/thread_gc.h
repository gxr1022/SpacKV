#ifndef STORAGE_LEVELDB_DB_THREAD_GC_H_
#define STORAGE_LEVELDB_DB_THREAD_GC_H_

#include<thread>
#include<condition_variable>
#include<queue>
#include<mutex>
#include "export.h"
#include "leveldb/env.h"
#include "leveldb/status.h"
#include "leveldb/port.h"
#include "leveldb/thread_annotations.h"

namespace leveldb{
class LEVELDB_EXPORT Thread_gc{
private:
    void BackgroundThreadMain();
    static void BackgroundThreadEntryPoint(Thread_gc* gc) {
        gc->BackgroundThreadMain();
  }

  // Stores the work item data in a Schedule() call.
  //
  // Instances are constructed on the thread calling Schedule() and used on the
  // background thread.
  //
  // This structure is thread-safe because it is immutable.
  struct BackgroundWorkItem {
    explicit BackgroundWorkItem(void (*function)(void* arg), void* arg)
        : function(function), arg(arg) {}
    void (*const function)(void*);
    void* const arg;
  };

  port::Mutex background_work_mutex_;
  port::CondVar background_work_cv_ GUARDED_BY(background_work_mutex_);
  bool started_background_thread_ GUARDED_BY(background_work_mutex_);

  std::queue<BackgroundWorkItem> background_work_queue_
      GUARDED_BY(background_work_mutex_);

public:
    Thread_gc():background_work_cv_(&background_work_mutex_),
      started_background_thread_(false){}
    ~Thread_gc(){}

    void StartThread(void (*thread_main)(void* thread_main_arg),
                   void* thread_main_arg){
        std::thread new_thread(thread_main, thread_main_arg);
        new_thread.detach();
    }
    void Schedule(void (*background_work_function)(void* background_work_arg),
                void* background_work_arg);


};
}



#endif






