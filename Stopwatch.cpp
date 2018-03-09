/*
 *  Stopwatch.cpp
 *  Breakthrough Iterative Deepening Transposition Tables
 *
 *  Created by K Donegan on 2/18/13.
 *  Copyright 2013 University of Denver. All rights reserved.
 *
 */

#include <time.h>
#include <iostream>


void Start()
{
	clock_t myClock;
	myClock = clock();
}

double Stop(clock_t &c)
{
	clock_t currentClock;
	currentClock = clock();
	
	double duration = difftime(c, currentClock);
	std::cout << "AI> Time = " << duration << " msecs" << std::endl;
	
	return duration;
}
