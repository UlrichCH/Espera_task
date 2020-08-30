#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <iostream>
#include <thread>
#include <queue>
#include <chrono>
#include <ctime> 
#include <condition_variable>

using namespace std;
//struct for the Package
struct package  {
	int id;
	double dimensions[3];
};
// Print function for a Package
void printPackage(package pack) {
	cout << right << "ID: " << pack.id << " | x = " << pack.dimensions[0] << " | y = " << pack.dimensions[1] << " | z = " << pack.dimensions[2] << endl;
}

void generate_package(int startID, double x, double y, double z, double acc, queue<package> & workload, condition_variable & cv, mutex & m, bool & stopworking)
{
	int packageCount = 0;
	while (!stopworking ) {

		//Generating random packages, which are all a little bit of the ideal dimensions x,y,z 
		package pckg;
		pckg.id = startID + packageCount;
		pckg.dimensions[0] = x + acc * (2 * ((double)rand() / (double)RAND_MAX) - 1);
		pckg.dimensions[1] = y + acc * (2 * ((double)rand() / (double)RAND_MAX) - 1);
		pckg.dimensions[2] = z + acc * (2 * ((double)rand() / (double)RAND_MAX) - 1);
		//push the package into the Queue
		workload.push(pckg);
		packageCount++;

		time_t act_time = time(0);
		cout << ctime(&act_time) << "PRODUCER \nsend Package with ID " << pckg.id << endl;
		printPackage(pckg);
		cout << endl;
		cv.notify_one();
		this_thread::sleep_for(chrono::seconds(1));
	}
	while (stopworking && !workload.empty()) {
		cv.notify_one();
		this_thread::sleep_for(chrono::seconds(1));
	}
}

void label_package(double x, double y, double z, double acc, queue<package> & workload, condition_variable & cv, mutex & m, bool & stopworking) {
	// Initial delay to represent the "transportation time"
	this_thread::sleep_for(chrono::seconds(6));
	while (!stopworking || !workload.empty())
	{   
		//only works when producer told that there is another package
		std::unique_lock<std::mutex> lk(m);
		cv.notify_one();
		cv.wait(lk);

		time_t act_time = time(0);
		cout.width(100); cout << std::right << ctime(&act_time);
		cout.width(100); cout << std::right << "CONSUMER " << endl;
		//test if the package meets the expected meseaurments x, y, z
		if (workload.front().dimensions[0] < x - acc) {
			cout.width(80); cout << std::right << "-DISCARDED- Package with ID " << workload.front().id << " | width too short" << endl;
		}
		else {
			if (workload.front().dimensions[0] > x + acc) {
				cout.width(80); cout << std::right << "-DISCARDED- Package with ID " << workload.front().id << " | width too long" << endl;
			}
			else {
				if (workload.front().dimensions[1] < y - acc) {
					cout.width(80); cout << std::right << "-DISCARDED- Package with ID " << workload.front().id << " | depth too short" << endl;
				}
				else {
					if (workload.front().dimensions[1] > y + acc) {
						cout.width(80); cout << std::right << "-DISCARDED- Package with ID " << workload.front().id << " | depth too long" << endl;
					}
					else {
						if (workload.front().dimensions[2] < z - acc) {
							cout.width(80); cout << std::right << "-DISCARDED- Package with ID " << workload.front().id << " | height too short" << endl;
						}
						else {
							if (workload.front().dimensions[2] > z + acc) {
								cout.width(80); cout << std::right << "-DISCARDED- Package with ID " << workload.front().id << " | height too long" << endl;
							}
							else {
								cout.width(96); cout << std::right << "-LABELED- Package with ID " << workload.front().id << endl;
							}
						}
					}
				}
			}
		}
		
		cout.width(56); printPackage(workload.front());
		cout << endl;
		//pop the package from the queue
		workload.pop();
		
	}
}

int main() {
	condition_variable cv;
	mutex m;
	//queue for the packages
	queue<package> workload;
	bool stopworking = false;
	//producer thread
	thread producer(generate_package, 0, 40.0, 30.0, 20.0, 5.0, ref(workload), ref(cv), ref(m), ref(stopworking));
	//consumer thread
	thread consumer(label_package, 40.0, 30.0, 20.0, 2.5, ref(workload), ref(cv), ref(m), ref(stopworking));
    
	producer.join();
	consumer.join();
	
	system("pause");
	return 0;
};