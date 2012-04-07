
#pragma once

#include "FileSystemExperiment.h"
#include <time.h>
#include <stdlib.h>

class FileSystemLocalReadA : public FileSystemExperiment
{
public:
	FileSystemLocalReadA(StopwatchStack& ss);
	virtual ~FileSystemLocalReadA(void);
	virtual void Run(void);

protected:
	
};

