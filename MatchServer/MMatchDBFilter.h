#ifndef _MMATCH_DB_FILTER
#define _MMATCH_DB_FILTER



// DB에 저장되는 문자열중에 쿼리에 문제를 일이킬수 있는 문자를 제거하는 역활을 함.
class MMatchDBFilter
{
public :
	MMatchDBFilter();
	~MMatchDBFilter();

	string Filtering( const string& str );

private :
};


#endif // _MMATCH_DB_FILTER