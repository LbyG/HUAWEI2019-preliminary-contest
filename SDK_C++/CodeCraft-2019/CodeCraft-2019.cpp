#include "iostream"
#include "car.h"
#include "overall_schedule.h"

using namespace std;

int main(int argc, char *argv[])
{
    std::cout << "Begin" << std::endl;
	
	if(argc < 5){
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		exit(1);
	}
	
	std::string carPath(argv[1]);
	std::string roadPath(argv[2]);
	std::string crossPath(argv[3]);
	std::string answerPath(argv[4]);
	
	std::cout << "carPath is " << carPath << std::endl;
	std::cout << "roadPath is " << roadPath << std::endl;
	std::cout << "crossPath is " << crossPath << std::endl;
	std::cout << "answerPath is " << answerPath << std::endl;
	
	// TODO:read input filebuf
    overall_schedule OS;
    OS.load_cars_road_cross(carPath, roadPath, crossPath);
	// TODO:process
    OS.load_answer(answerPath);
    int T = OS.schedule_cars();
    std::cout << "T = " << T << std::endl;
	// TODO:write output file
	
	return 0;
}