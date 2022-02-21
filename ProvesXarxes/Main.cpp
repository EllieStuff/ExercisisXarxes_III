//
//#include <iostream>
//#include <thread>
//#include <vector>
//#include <string>
//#include <time.h>
//#include <mutex>
//
//
//class ThreadClass;
//std::vector<ThreadClass> threadsVec;
//std::mutex mtx;
//
//class ThreadClass {
//public:
//	int posInArray;
//	std::thread* thread;
//
//	void PrintThread() {
//		posInArray = threadsVec.size() - 1;
//
//		int timesPrinted = 0;
//		while (timesPrinted < 10) {
//			mtx.lock();
//			std::cout << "Soy el thread " << std::this_thread::get_id() << " y estoy en la posicion " << std::to_string(posInArray) << std::endl;
//			mtx.unlock();
//
//			time_t finalTime, currTime;
//			time(&currTime);
//			finalTime = currTime + 1;
//			while (currTime < finalTime) {
//				time(&currTime);
//			}
//
//			timesPrinted++;
//		}
//
//		mtx.lock();
//		threadsVec.erase(threadsVec.begin());
//		for (int i = 0; i < threadsVec.size(); i++)
//			threadsVec[i].posInArray--;
//		mtx.unlock();
//
//	}
//
//
//};

//int main() {
//
//	while (true) {
//		std::string text;
//		std::cin >> text;
//		if (text == "N") {
//			mtx.lock();
//
//			ThreadClass tClass;
//			std::thread t(&ThreadClass::PrintThread, tClass);
//			tClass.thread = &t;
//			t.detach();
//			threadsVec.push_back(tClass);
//
//			mtx.unlock();
//		}
//
//	}
//
//
//	return 0;
//}