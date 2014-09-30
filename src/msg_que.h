/*
 * file name: msg_que.h
 * purpose  :
 * author   : huang chunping
 * date     : 2014-09-13
 * history  :
 */
#ifndef __MSG_QUE_H__
#define __MSG_QUE_H__

#include <list>
#include <string>
using namespace std;

template <typename ElemType>
class MsgQue {
public:
	MsgQue(int size_limit) : que_max_limit(size_limit) {
		pthread_rwlock_init(&locker, NULL);
	}
	~MsgQue() {
		pthread_rwlock_destroy(&locker);
	}

	bool get_rdlocker() {
		if (0 != pthread_rwlock_rdlock(&locker)) {
			return false;
		}

		return true;
	}

	bool get_wrlocker() {
		if (0 != pthread_rwlock_wrlock(&locker)) {
			return false;
		}
		
		return true;
	}
	void giveback_locker() {
		pthread_rwlock_unlock(&locker);
	}

	ElemType* get() {
		if (true != get_rdlocker()) {
			return NULL;
		}

		if (que.size() < 1) {
			giveback_locker();
			return NULL;
		}

		ElemType* elem = &que.front();

		giveback_locker();

		return elem;
	}

	bool del(ElemType* elem) {
		if (true != get_wrlocker()) {
			return false;
		}
		if (NULL == elem) {
			que.pop_front();
		} else {
			typename list<ElemType>::iterator iter = que.begin();
			while (iter != que.end()) {
				if (&(*iter) == elem) {
					que.erase(iter);
					break;
				}
			}
		}

		giveback_locker();

		return true;
	}

	bool add(ElemType &data) {
		if (true != get_rdlocker()) {
			return false;
		}

		if (que.size() >= que_max_limit) {
			giveback_locker();
			return false;
		}

		que.push_back(data);

		giveback_locker();

		return true;
	}

private:
	int					que_max_limit;
	pthread_rwlock_t	locker;
	list<ElemType>		que;
};

#endif // __MSG_QUE_H__
