/* test-getgz.cxx
 * from: https://codereview.stackexchange.com/questions/175353/getting-current-timezone
 */
 
#include <ctime>
#include <iomanip>
#include <sstream>
#include <iostream>

static constexpr time_t const NULL_TIME = -1;

// returns difference in seconds from UTC at given time
// or at current time if not specified
long tz_offset(time_t when = NULL_TIME)
{
    if (when == NULL_TIME)
        when = std::time(nullptr);
    auto const tm = *std::localtime(&when);
    std::ostringstream os;
    os << std::put_time(&tm, "%z");
    std::string s = os.str();
    std::cout << "In ISO 8601 format: \"+/-HHMM\" '" << s << "'" << std::endl;
    
    int h = std::stoi(s.substr(0,3), nullptr, 10);
    int m = std::stoi(s[0]+s.substr(3), nullptr, 10);

    return h * 3600 + m * 60;
}

int main()
{
    std::cout << "tz_offset in seconds " << tz_offset() << std::endl;
}

#if 0 /* REJECTED */
long timezone = 0;

bool _tzset() {
   std::time_t now = std::time(NULL);
   std::time_t local = std::mktime(std::localtime(&now));
   std::time_t gmt = std::mktime(std::gmtime(&now));
   timezone = static_cast<long> (gmt - local);
   return true;
}

#endif /* REJECTED */

/* eof */
