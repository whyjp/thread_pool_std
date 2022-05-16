#ifndef  _WINGO_ETL_THREAD_H_
#define  _WINGO_ETL_THREAD_H_ 

#pragma once

#include <Windows.h>
#include <process.h>

namespace wingo {

	class WorkerThread
	{
		WorkerThread(const WorkerThread&) {}
		WorkerThread& operator = (const WorkerThread&) {}

	public:
		WorkerThread() : thread_handle_(NULL), thread_id_(0) {}
		virtual ~WorkerThread()
		{
			if (NULL != thread_handle_) {
				::CloseHandle(thread_handle_);
			}
		}
		inline void setThreadInfo(HANDLE thread_handle, DWORD thread_id) 
		{
			thread_handle_ = thread_handle;
			thread_id_ = thread_id;
		}

		virtual UINT run() = 0;

	protected:
		HANDLE		thread_handle_;					// handle thread
		DWORD		thread_id_;						// thread id

	public:
		//bool start();
		void stop()
		{
			if (this->IsRunning()) {
				// 강제 Terminate이므로 정상적인 자원 해제를 기대할 수 없음
				// 공유자원 등을 물고 있을 경우 deadlock 등의 위험이 있을 수 있음
				::TerminateThread(thread_handle_, -1);
			}

			if (thread_handle_) {
				::CloseHandle(thread_handle_);
				thread_handle_ = NULL;
			}
		}

		inline DWORD getThreadID() { return thread_id_; }
		inline HANDLE getThreadHandle() { return thread_handle_; }

		// Status
		inline bool IsRunning() {
			if (thread_handle_) {
				DWORD dwExitCode = 0;
				::GetExitCodeThread(thread_handle_, &dwExitCode);
				if (dwExitCode == STILL_ACTIVE) return true;
			}
			return false;
		}

		// Join - Wait For Thread Done
		inline void Join() {
			::WaitForSingleObject(thread_handle_, INFINITE);
		}

		// Yeild - Yeild Execution to Another Thread
		inline BOOL Yeild() {
			return ::SwitchToThread();
		}

		// Suspend - Suspend Thread
		inline DWORD pause() {
			return ::SuspendThread(thread_handle_);
		}

		// Resume - Resume Thread
		inline DWORD resume() {
			return ::ResumeThread(thread_handle_);
		}

		// Sleep - Suspends the execution of the current thread until the time-out interval elapses
		inline void waitThread(DWORD dwMilliSec) {
			::Sleep(dwMilliSec);
		}
	};

}

#endif // _WINGO_ETL_THREAD_H_ 
