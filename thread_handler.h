#ifndef  _WINGO_ETL_THREAD_HANDLER_H_
#define  _WINGO_ETL_THREAD_HANDLER_H_ 

#pragma once

#include "thread.h"
#include <vector>

namespace wingo {

	class ThreadHandler
	{
	public:
		ThreadHandler()	{}
		~ThreadHandler() {}

		bool addJob(WorkerThread* workerThreadPtr)
		{
			if (workerThreadPtr->IsRunning()) {
				if (::WaitForSingleObject(workerThreadPtr->getThreadHandle(), 0) == WAIT_TIMEOUT) {
					return false;
				}
				::CloseHandle(workerThreadPtr->getThreadHandle());
			}
			DWORD thread_id = 0;
			HANDLE
				thread_handle = ::CreateThread(NULL,			// default security attributes
											   0,				// default stack size
						   					   (LPTHREAD_START_ROUTINE)ThreadHandler::WorkThreadProc,
											   workerThreadPtr,	// thread function argument = this class
											   0,				// default creation flags
											   &thread_id		// receive thread identifier
											  );
			if (thread_handle != NULL) {
				workerThreadPtr->setThreadInfo(thread_handle, thread_id);
				workerThreads_.push_back(workerThreadPtr);
				return true;
			}
			return false;
		}

		void deleteJob(WorkerThread* workerThreadPtr)
		{
			while (!workerThreads_.empty()) {
				WorkerThread* pWorker = workerThreads_.back();
				if (workerThreadPtr == pWorker) {
					workerThreads_.pop_back();
					pWorker->stop();
					pWorker->waitThread(2000);
					delete pWorker;
					pWorker = NULL;
				}
			}
		}
		
		void deleteAllJob()
		{
			while (!workerThreads_.empty()) {
				WorkerThread* pWorker = workerThreads_.back();
				workerThreads_.pop_back();
				pWorker->stop();
				pWorker->waitThread(2000);
				delete pWorker;
				pWorker = NULL;
			}
		}

		void pauseJob(WorkerThread* workerThreadPtr)
		{
			// lock? here? or inner?
			workerThreadPtr->pause();
		}

		void resumeJob(WorkerThread* workerThreadPtr)
		{
			// lock? here? or inner?
			workerThreadPtr->resume();
		}
	
	private:
		std::vector<WorkerThread*>	workerThreads_;

	private:
		static UINT __stdcall WorkThreadProc(LPVOID thread_parameter)
		{
			WorkerThread* thread = (WorkerThread*)thread_parameter;
			return thread->run();
		}

	};
}


#endif // _WINGO_ETL_THREAD_HANDLER_H_ 