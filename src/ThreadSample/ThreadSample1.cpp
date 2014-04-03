#include "stdafx.h"

class MessageQueue : private boost::noncopyable {
public:
	MessageQueue() {}
	virtual ~MessageQueue() {}

	void post_message(const int &msg) {
		boost::mutex::scoped_lock lock(message_queue_mutex_);
		message_queue_.push_back(msg);
	}

	bool peek_message(int &msg) {
		boost::mutex::scoped_lock lock(message_queue_mutex_);
		if (message_queue_.size() == 0) return false;

		msg = message_queue_[0];
		message_queue_.pop_front();

		return true;
	}

protected:
	std::deque<int> message_queue_;
	boost::mutex message_queue_mutex_;  // noncopyable
};

class WorkerThreadTest {
public:
	WorkerThreadTest() : break_flag_(false) {}
	virtual ~WorkerThreadTest() {}

	void run() {
		break_flag_ = false;
		while(!break_flag_) {
			int msg;
			bool rv = message_queue_.peek_message(msg);
			if (rv == true) {
				std::cout << "receive message : msg=" << msg << std::endl;
			}
			else {
				boost::this_thread::sleep(boost::posix_time::milliseconds(500));
			}
		}
	}

	void stop() {
		break_flag_ = true;
	}

	void post_message(const int &msg) {
		message_queue_.post_message(msg);
	}

private:
	bool break_flag_;
	MessageQueue message_queue_;
};

int main(int argc, char* argv[])
{
	WorkerThreadTest worker;
	boost::thread thread(boost::bind(&WorkerThreadTest::run, &worker));

	for (int i = 0; i < 10; ++i) {
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
		worker.post_message(i);
		std::cout << "send message : msg=" << i << std::endl;
	}

	worker.stop();
	thread.join();

	return 0;
}
