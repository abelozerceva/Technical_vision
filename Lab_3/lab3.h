#ifndef LAB3_LAB3_H
#define LAB3_LAB3_H

#include <vector>
#include <string>

/**
 * @brief - Loads files with images
 * @return - vector with filenames of images
 */
std::vector<std::string> getFilesLab3();

void lab3();
/**
 * @brief - find objects with binary image
 * Houses are warmer, than anything else.
 * Need to find all of them and mark center of each
 * @param image - image with object to detect
 */
void findHouse(std::string image);
/**
 * @brief - find objects with HSV
 * Engines are warmer than anything else.
 * Need to find and mark center.
 * @param image - image with object to detect
 */
void findEngine(std::string image);
/**
 * @brief - Find colorful cap and circle around it color of robots' team.
 * @param image - image with object to detect
 */
void findRobots(std::string image);
/// *****************************************************
/// Find the robot closest to the lamp and mark it's mass center.
/// *****************************************************
int findNearestRobot(int X1, int Y1, int X2, int Y2);
/// *****************************************************
/// Find deffective wrenches and mark them.
/// *****************************************************
void findWrench(std::string image, std::string objectTemplate);

#endif //LAB3_LAB3_H
