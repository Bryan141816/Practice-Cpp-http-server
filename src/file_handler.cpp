#include "file_handler.h"

bool readfiletoString(const std::filesystem::path &p, std::string &out)
{
    std::ifstream f(p, std::ios::binary);
    if (!f)
        return false;

    f.seekg(0, std::ios::end);
    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);

    out.clear();
    if (size > 0)
    {
        out.resize((size_t)size);
        f.read(&out[0], size);
    }
    return true;
}