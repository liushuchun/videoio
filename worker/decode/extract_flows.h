#ifndef __TRANS_FRAMES__
#define __TRANS_FRAMES__


#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#define new DBG_NEW
#endif

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif


#include "opencv2/core.hpp"
#include <opencv2/core/utility.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/video/tracking.hpp"

#include <iostream>
#include "../universal_worker.h"

int  CalOptFlow(serialize_params*  decode_memory, serialize_params*&  extract_output);


#endif