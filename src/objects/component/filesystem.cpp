#include "filesystem.hpp"


#include <regex>
#include <algorithm>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <tuple>

/////////////////////////////////////// FileSystem::CodeEntity ///////////////////////////////////////
FSType FileSystem::CodeEntity::type() const
{
    return FSType::CodeFile;
}

const std::string& FileSystem::CodeEntity::code() const
{
    return _code;
}




/////////////////////////////////////// FileSystem::DirEntity ///////////////////////////////////////

FSType FileSystem::DirEntity::type() const
{
    return FSType::Directory;
}

FileSystem::DirEntity* FileSystem::DirEntity::as_dir()
{
    return this;
}


FileSystem::Entity* FileSystem::DirEntity::entity(const std::string& file, bool wantdir, bool createdir)
{
    auto it = entity(file, wantdir);
    if(it == _content.end())
    {
        if(createdir)
        {
            auto newdir = std::make_unique<DirEntity>();
            std::tie(it, std::ignore) = _content.emplace(file, std::move(newdir));
        }else{
            return nullptr;
        }
    }
    return it->second.get();
}


FileSystem::Entity* FileSystem::DirEntity::new_file(std::string file)
{
    auto it = entity(file, false);

    if(it != _content.end())
    {
        if(it->second->type() == FSType::Directory)
        {
            throw std::runtime_error("can not override directory '" + file + "'with file");
        }

        file = it->first;
        _content.erase(it);
    }

    auto newdir = std::make_unique<CodeEntity>();
    std::tie(it, std::ignore) = _content.emplace(file, std::move(newdir));
    return it->second.get();
}

void FileSystem::DirEntity::erase(const std::string& file)
{
    auto it = entity(file, false);
    if(it == _content.end())
        throw std::runtime_error("'" + file + "' does not exist to delete it");
    _content.erase(it);
}

FileSystem::DirEntity::entity_map::iterator FileSystem::DirEntity::entity(const std::string& file, bool wantdir)
{
    auto verpos = file.find('^');
    if(verpos != std::string::npos)
    {
        // semver... we have to search all
        return semvered_entity(file, verpos, wantdir);
    }

    return _content.find(file);
}


FileSystem::DirEntity::entity_map::iterator FileSystem::DirEntity::semvered_entity(const std::string& file, std::string::size_type verpos, bool wantdir)
{
    using semver = std::array<unsigned int, 3>;
    std::string surfix = file.substr(verpos + 1);
    std::regex semregex("^(\\.?(\\d{1,9}))(\\.(\\d{1,9}))?(\\.(\\d{1,9}))?(.*)$");

    semver maxver = {0, 0, 0};
    auto maxentity = _content.end();

    for(auto it = _content.begin(); it != _content.end(); ++it)
    {
        auto& entry_name = it->first;
        auto* entry = it->second.get();

        // check if it is a dir, if only dirs are wanted
        if(wantdir && entry->type() != FSType::Directory)
            continue;

        // check prefix
        bool prefix_matches = entry_name.compare(0, verpos, file, 0, verpos) == 0;
        if(!prefix_matches)
            continue;

        auto begin = entry_name.cbegin() + verpos;
        auto end = entry_name.cend();
        
        std::smatch match;
        if(!std::regex_match(begin, end, match, semregex))
            continue;
        
        // check if surfix matches
        auto& surfix_match = *(match.end() - 1);
        if(surfix_match.compare(surfix) != 0)
            continue;
        
        // read semver
        semver ver = {0, 0, 0};
        int v = 0;
        for(int i = 2; i < match.size() - 1; i += 2)
        {
            auto str = match.str(i);
            if(match.length(i))
            {
                ver[v++] = boost::lexical_cast<unsigned int>(str);
            }
        }

        // maxentity && !(maxver < ver) === maxver >= ver
        assert(maxver != ver);
        if(maxentity != _content.end() && !std::lexicographical_compare(maxver.begin(), maxver.end(), ver.begin(), ver.end()))
            continue;

        maxver = ver;
        maxentity = it;
    }
    return maxentity;
}


/////////////////////////////////////// FileSystem ///////////////////////////////////////
FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

bool FileSystem::exists(const std::string& cwd, const std::string& path)
{
    return entity(cwd, path) != nullptr;
}

FSType FileSystem::type(const std::string& cwd, const std::string& path)
{
    return entity_checked(cwd, path)->type();
}

FileSystem::Entity* FileSystem::write(const std::string& cwd, const std::string& path, std::string content)
{
    Entity* e = entity(cwd, path, FSOperation::write);
    static_cast<CodeEntity*>(e)->_code = std::move(content);
    return e;
}

const std::string& FileSystem::read(const std::string& cwd, const std::string& path)
{
    return entity_checked(cwd, path)->code();
}

bool FileSystem::erase(const std::string& cwd, const std::string& path)
{
    try {
        entity(cwd, path, FSOperation::erase);
        return true;
    }catch(const std::exception&)
    {
        return false;
    }
}

FileSystem::Entity* FileSystem::entity_checked(const std::string& cwd, const std::string& file, FSOperation op)
{
    Entity* e = entity(cwd, file, op);
    if(!e)
    {
        throw std::runtime_error("file '" + file + "' not found");
    }
    return e;
}

FileSystem::Entity* FileSystem::entity(const std::string& cwd, const std::string& file, FSOperation op)
{
    std::string full_path;
    if(file.size() > 0 && file.front() == '/')
    {
        full_path = file;
    } else {
        full_path = cwd + "/" + file;
    }

    // split path (dir1/dir2/file.ext -> [dir1, dir2, file.txt])
    std::vector<std::string> candidates{};
    boost::split(candidates, full_path, boost::is_any_of("/"), boost::token_compress_on);

    // remove empty parts
    std::vector<std::string> comps{};
    for(auto it = candidates.begin(); it != candidates.end(); ++it)
    {
        auto& comp = *it;
        if(*it == "..")
        {
            if(comps.size())
                comps.pop_back();
        } else if(!it->empty())
        {
            comps.push_back(std::move(comp));
        }
    }

    Entity* cur = &_root;

    int i = 0;
    for(const auto& comp : comps)
    {
        const bool isLast = (i == comps.size() - 1);
        if(cur->type() != FSType::Directory)
        {
            // cur is not a directory
            return nullptr;
        }

        DirEntity* dir = cur->as_dir();

        switch(op)
        {
        case FSOperation::erase:
            if(isLast)
            {
                dir->erase(comp);
                break;
            }
        default:
            cur = dir->entity(comp, !isLast, false);
            break;
        case FSOperation::write:
            if(isLast)
                cur = dir->new_file(comp);
            else
                cur = dir->entity(comp, true, true);
            break;
        };

        if(!cur)
        {
            // comp does not exist
            return nullptr;
        }
        ++i;
    }

    return cur;
}

