//////////////////////////////////////////////////////////////////////
//
// gtl.h:
//
// PWH
//    2020.11.12
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "gtl/_default.h"
#include "gtl/_lib_gtl.h"
#include "gtl/concepts.h"
#include "gtl/misc.h"
#include "gtl/string.h"
#include "gtl/archive.h"
#include "gtl/reflection.h"
//#include "gtl/dynamic.h"
#include "gtl/unit.h"
#include "gtl/coord.h"
#include "gtl/rand.h"
#include "gtl/mat_helper.h"


// todo : streamable_unique_ptr / streamable_shared_ptr - derived from std::unique_ptr / std::shared_ptr  respectively. for dynamic creatable object


//-----------------------------------------------------------------------------
// Naming Convention :
/*

* basic : maximize the utilities of IDE
	+ class Name
		- camal case
		- class name starts with 'x'
			ex) xCoordTrans
		- template class name starts with 'T'
		- interface class name starts with 'I'
		- Abbrevs. are all in UPPER CASE.
		- Function Starts with Upper Case.

	+ class member variables :
		i) public member variables - starts with 'm_'
		ii) private members ....... ???

	+ class member functions : ... which to take?
		i) Categorizing - Category comes first ?
			ex) functions with 'Path'
				Path + Get -> PathGet, PathSet, PathInit ...
		ii) Verb + Object.
			ex) functions with 'Path'
				GetPath, SetPath, InitPath ...

		ex)
			PutSomethingSpecial() ('P', 'S', 'S' 대문자 사용)
			GetDNS_Name : Get, DNS, Name 의 'G'et, 'DNS', 'N'ame 은 대문자 사용. DNS 와 Name 이 대문자로 연결 되므로, '_' 를 사용하여 띄워줌.

ex)
	(1) (Set/Get), (Read/Write), (Send/Receive)
		- Set/Get : 단순히 변수의 값을 설정 할 경우.
		- Read/Write : File/Stream 등의 입출력. IO 길이가 정해진 경우.
		- Send/Receive : 특정 Protocol 을 사용하여 데이터를 송/수신 해야 하는 경우.

	(2) Init, Clear
		- Init() : 초기화. Class 를 사용하기 전, 어떤 작업(파일 입출력이든, ...)을 해야 할 경우.
		- Clear() : 초기화. 단순 값들을 0으로 돌리는 작업.

	(3) FromString, ToString



*/
