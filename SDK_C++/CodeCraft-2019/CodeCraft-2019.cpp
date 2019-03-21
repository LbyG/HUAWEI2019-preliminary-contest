#include "iostream"
#include "car.h"

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
	// TODO:process
    int T = 0;
    // If all cars is arrive then break
    while (cars_wait_run_n > 0 || cars_running_n > 0) {
        // Initial cars schedule status, car.schedule_status = 0
        initial_cars_schedule_status();
        // Schedule cars in road.
        // If car can through cross then car into schedule wait -> car.schedule_status = 1
        // If car blocked by schedule wait car then car into schedule wait -> car.schedule_status = 1
        // If car don't be block and can't through cross then car run one time slice and into end state -> car.schedule_status = 2
        // If car blocked by end state car then car move to the back of the previous car
        schedule_cars_in_road();
        // Cycling the relevant roads at each cross until all car are end state
        while (cross_wait_car_n > 0) {
            for (all crosses) {
                schedule_cars_in_cross()
            }
        }
        // Schedule cars which wait start
        schedule_cars_wait_start();
        T ++;
    }
    std::cout << "T = " << T << std::endl;
	// TODO:write output file
	
	return 0;
}