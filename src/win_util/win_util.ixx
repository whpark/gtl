module;

#include "framework.h"

#include <fstream>	// for gtl:log

export module gtlw;
import gtl;
export import :misc;
export import :TMDialog;
export import :ListCtrlEd;
export import :MatHelper;
export import :SerialPort;
export import :ProgressDlg;


export namespace gtl::win_util {

}

export namespace gtlw = gtl::win_util;
