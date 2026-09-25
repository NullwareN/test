/*
 * textfile.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <stdio.h>

// Cached data path
std::optional<std::string> cached_data_path;

#include <sys/stat.h>
#include <glob.h>

static bool is_valid_data_dir(const std::string &dir)
{
    if (dir.empty())
        return false;
    std::string test_file = dir + "/fonts/tf2build.ttf";
    struct stat st{};
    return (stat(test_file.c_str(), &st) == 0);
}

namespace paths
{
// Example: getDataPath("/foo") -> "/opt/bar/data/foo"
std::string getDataPath(std::string subpath)
{
    if (!cached_data_path)
    {
        if (std::getenv("CH_DATA_PATH") && is_valid_data_dir(std::getenv("CH_DATA_PATH")))
        {
            cached_data_path = std::getenv("CH_DATA_PATH");
        }
        else if (is_valid_data_dir(DATA_PATH))
        {
            cached_data_path = DATA_PATH;
        }
        else
        {
            std::vector<std::string> patterns = {
                std::string(std::getenv("HOME") ? std::getenv("HOME") : "") + "/.local/share/cathook/data",
                std::string(std::getenv("HOME") ? std::getenv("HOME") : "") + "/Downloads/*/data",
                std::string(std::getenv("HOME") ? std::getenv("HOME") : "") + "/Desktop/*/data",
                "/home/*/.local/share/cathook/data",
                "/home/*/Downloads/*/data",
                "/home/*/Desktop/*/data",
                "/home/*/*/data",
                "/home/*/*/*/data"
            };

            for (const auto &pat : patterns)
            {
                if (pat.empty())
                    continue;
                glob_t gl;
                if (glob(pat.c_str(), GLOB_TILDE, nullptr, &gl) == 0)
                {
                    for (size_t i = 0; i < gl.gl_pathc; ++i)
                    {
                        std::string candidate = gl.gl_pathv[i];
                        if (is_valid_data_dir(candidate))
                        {
                            cached_data_path = candidate;
                            break;
                        }
                    }
                    globfree(&gl);
                }
                if (cached_data_path)
                    break;
            }

            if (!cached_data_path)
            {
                cached_data_path = DATA_PATH;
            }
        }
    }
    return *cached_data_path + subpath;
}
std::string getConfigPath()
{
    return getDataPath("/configs");
}
} // namespace paths

// Textfile class functions
TextFile::TextFile() : lines{}
{
}

bool TextFile::TryLoad(const std::string &name)
{
    if (name.length() == 0)
        return false;
    std::string filename = paths::getDataPath("/" + name);
    std::ifstream file(filename, std::ios::in);
    if (!file)
    {
        return false;
    }
    lines.clear();
    for (std::string line; std::getline(file, line);)
    {
        if (*line.rbegin() == '\r')
            line.erase(line.length() - 1, 1);
        lines.push_back(line);
    }
    if (lines.size() > 0 && *lines.rbegin() == "\n")
        lines.pop_back();

    return true;
}

void TextFile::Load(const std::string &name)
{
    std::string filename = paths::getDataPath("/" + name);
    std::ifstream file(filename, std::ios::in);
    if (file.bad())
    {
        logging::Info("Could not open the file: %s", filename.c_str());
        return;
    }
    lines.clear();
    for (std::string line; std::getline(file, line);)
    {
        if (*line.rbegin() == '\r')
            line.erase(line.length() - 1, 1);
        lines.push_back(line);
    }
}

size_t TextFile::LineCount() const
{
    return lines.size();
}

const std::string &TextFile::Line(size_t id) const
{
    return lines.at(id);
}
// Textfile class functions end
