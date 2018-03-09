/*
 *  Stopwatch.h
 *  Breakthrough Iterative Deepening Transposition Tables
 *
 *  Created by K Donegan on 2/18/13.
 *  Copyright 2013 University of Denver. All rights reserved.
 *
 */

#include <time.h>

class Stopwatch
{
public:
	TimeSpan Duration;
	Stopwatch();
	
	void Start();
	
	double Stop();
		
private:
	Time StartTime;
};