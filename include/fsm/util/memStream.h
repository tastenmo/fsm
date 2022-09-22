/**
 * @file memStorage.h
 * @author Martin Heubuch (martin.heubuch@escad.de)
 * @brief
 * @version 0.1
 * @date 2022-09-20
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <cstddef>
#include <cstring>
#include <iterator>
#include <iostream>

namespace ua
{

    class membuf : public std::basic_streambuf<char>
    {
    public:
        membuf(const uint8_t *p, size_t l)
        {
            setg((char *)p, (char *)p, (char *)p + l);
        }
    };

    class memstream : public std::istream
    {
    public:
        memstream(const uint8_t *p, size_t l) : std::istream(&_buffer),
                                                _buffer(p, l)
        {
            rdbuf(&_buffer);
        }

    private:
        membuf _buffer;
    };

} // namespace ua